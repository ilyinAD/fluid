//
// Created by artem on 15.12.2024.
//

#ifndef FLUID_FLUIDSIM_H
#define FLUID_FLUIDSIM_H
#include "../Fixed.h"
//#include "FastFixed.h"


template <typename type_p, typename type_v, typename type_vf, int N, int M>
class FluidSim {
public:


    char field[N][M + 1] = {
            "####################################################################################",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                       .........                                  #",
            "#..............#            #           .........                                  #",
            "#..............#            #           .........                                  #",
            "#..............#            #           .........                                  #",
            "#..............#            #                                                      #",
            "#..............#            #                                                      #",
            "#..............#            #                                                      #",
            "#..............#            #                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............#                                                      #",
            "#..............#............################                     #                 #",
            "#...........................#....................................#                 #",
            "#...........................#....................................#                 #",
            "#...........................#....................................#                 #",
            "##################################################################                 #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "#                                                                                  #",
            "####################################################################################",
    };
    type_p rho[256];

    int dirs[N][M]{};
    int last_use[N][M]{};
    int UT = 0;
    type_p p[N][M]{}, old_p[N][M];
    type_v g;

    FluidSim(char (&field1)[N][M + 1], float rho_air, int rho_fluid, float g1) : g(g1) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M + 1; ++j) {
                field[i][j] = field1[i][j];
            }
        }
        rho[' '] = rho_air;
        rho['.'] = rho_fluid;
    }


//    FluidSim(float rho_air, int rho_fluid, float g1) {
//        rho[' '] = rho_air;
//        rho['.'] = rho_fluid;
//        g = g1;
//    };
    template <typename type_cur>
    struct VectorField {
        array<type_cur, deltas.size()> v[N][M];
        type_cur &add(int x, int y, int dx, int dy, type_cur dv) {
            return get(x, y, dx, dy) += dv;
        }

        type_cur &get(int x, int y, int dx, int dy) {
            size_t i = ranges::find(deltas, pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v[x][y][i];
        }
    };

    struct ParticleParams {
        char type;
        type_p cur_p;
        array<type_v, deltas.size()> v;

        void swap_with(int x, int y, VectorField<type_v>& velocity1, type_p (&p1)[N][M], char (&field1)[N][M + 1]) {
            swap(field1[x][y], type);
            swap(p1[x][y], cur_p);
            swap(velocity1.v[x][y], v);
        }
    };

    VectorField<type_v> velocity{};
    VectorField<type_vf> velocity_flow{};

    type_p random01() {
        return type_p::from_raw((rnd() & ((1 << type_p::getK()) - 1)));
    }

    void propagate_stop(int x, int y, bool force = false) {
        if (!force) {
            bool stop = true;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > static_cast<type_v>(0)) {
                    stop = false;
                    break;
                }
            }
            if (!stop) {
                return;
            }
        }
        last_use[x][y] = UT;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > static_cast<type_v>(0)) {
                continue;
            }
            propagate_stop(nx, ny);
        }
    }

    type_p move_prob(int x, int y) {
        type_p sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                continue;
            }
            auto v = velocity.get(x, y, dx, dy);
            if (v < static_cast<type_v>(0)) {
                continue;
            }
            sum += static_cast<type_p>(v);
        }
        return sum;
    }

    bool propagate_move(int x, int y, bool is_first) {
        last_use[x][y] = UT - is_first;
        bool ret = false;
        int nx = -1, ny = -1;
        do {
            std::array<type_p, deltas.size()> tres;
            type_p sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                    tres[i] = sum;
                    continue;
                }
                auto v = velocity.get(x, y, dx, dy);
                if (v < static_cast<type_v>(0)) {
                    tres[i] = sum;
                    continue;
                }
                sum += static_cast<type_p>(v);
                tres[i] = sum;
            }

            if (sum == static_cast<type_p>(0)) {
                break;
            }

            Fixed p = random01() * sum;
            size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

            auto [dx, dy] = deltas[d];
            nx = x + dx;
            ny = y + dy;
            assert(velocity.get(x, y, dx, dy) > static_cast<type_v>(0) && field[nx][ny] != '#' && last_use[nx][ny] < UT);

            ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
        } while (!ret);
        last_use[x][y] = UT;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < static_cast<type_v>(0)) {
                propagate_stop(nx, ny);
            }
        }
        if (ret) {
            if (!is_first) {
                ParticleParams pp{};
                pp.swap_with(x, y, velocity, p, field);
                pp.swap_with(nx, ny, velocity, p, field);
                pp.swap_with(x, y, velocity, p, field);
            }
        }
        return ret;
    }

    tuple<type_p, bool, pair<int, int>> propagate_flow(int x, int y, type_p lim) {
        last_use[x][y] = UT - 1;
        type_p ret = 0;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                auto cap = velocity.get(x, y, dx, dy);
                auto flow = velocity_flow.get(x, y, dx, dy);
                if (flow == cap) {
                    continue;
                }
                // assert(v >= velocity_flow.get(x, y, dx, dy));
                type_v res = cap - flow;
                auto vp = min(lim, static_cast<type_p>(res));
                if (last_use[nx][ny] == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);
                    last_use[x][y] = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                    return {vp, 1, {nx, ny}};
                }
                auto [t, prop, end] = propagate_flow(nx, ny, vp);
                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, t);
                    last_use[x][y] = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                    return {t, prop && end != pair(x, y), end};
                }
            }
        }
        last_use[x][y] = UT;
        return {ret, 0, {0, 0}};
    }

    void run() {

        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy] : deltas) {
                    dirs[x][y] += (field[x + dx][y + dy] != '#');
                }
            }
        }

        for (size_t i = 0; i < T; ++i) {

            type_p total_delta_p = 0;
            // Apply external forces
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#')
                        continue;
                    if (field[x + 1][y] != '#')
                        velocity.add(x, y, 1, 0, g);
                }
            }

            // Apply forces from p
            memcpy(old_p, p, sizeof(p));
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        int nx = x + dx, ny = y + dy;
                        if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                            auto delta_p = old_p[x][y] - old_p[nx][ny];
                            auto force = delta_p;
                            auto &contr = velocity.get(nx, ny, -dx, -dy);
                            if (static_cast<type_p>(contr) * rho[(int) field[nx][ny]] >= force) {
                                contr -= static_cast<type_v>(force / rho[(int) field[nx][ny]]);
                                continue;
                            }
                            force -= static_cast<type_p>(contr) * rho[(int) field[nx][ny]];
                            contr = 0;
                            velocity.add(x, y, dx, dy, force / rho[(int) field[x][y]]);
                            //int q =
                            p[x][y] -= force / static_cast<type_p>(dirs[x][y]);
                            total_delta_p -= force / static_cast<type_p>(dirs[x][y]);
                        }
                    }
                }
            }

            // Make flow from velocities
            velocity_flow = {};
            bool prop = false;
            do {
                UT += 2;
                prop = 0;
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] != '#' && last_use[x][y] != UT) {
                            auto [t, local_prop, _] = propagate_flow(x, y, 1);
                            if (t > static_cast<type_p>(0)) {
                                prop = 1;
                            }
                        }
                    }
                }
            } while (prop);

            // Recalculate p with kinetic energy
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        auto old_v = velocity.get(x, y, dx, dy);
                        auto new_v = velocity_flow.get(x, y, dx, dy);
                        if (old_v > static_cast<type_v>(0)) {
                            assert(new_v <= old_v);
                            velocity.get(x, y, dx, dy) = new_v;
                            auto force = (old_v - new_v) * rho[(int) field[x][y]];
                            if (field[x][y] == '.')
                                force *= static_cast<type_p>(0.8);
                            if (field[x + dx][y + dy] == '#') {
                                p[x][y] += force / static_cast<type_p>(dirs[x][y]);
                                total_delta_p += force / static_cast<type_p>(dirs[x][y]);
                            } else {
                                p[x + dx][y + dy] += force / static_cast<type_p>(dirs[x + dx][y + dy]);
                                total_delta_p += force / static_cast<type_p>(dirs[x + dx][y + dy]);
                            }
                        }
                    }
                }
            }

            UT += 2;
            prop = false;
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] != '#' && last_use[x][y] != UT) {
                        if (random01() < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        } else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }

            if (prop) {
                cout << "Tick " << i << ":\n";
                for (size_t x = 0; x < N; ++x) {
                    cout << field[x] << "\n";
                }
            }
        }
    }
};



#endif //FLUID_FLUIDSIM_H
