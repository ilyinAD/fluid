#include <bits/stdc++.h>
#include "FluidSim.h"
#include "SmartFluidSim.h"
using namespace std;

////#define SIZES "S(1, 1), S(2, 2)"
//#define S(x, y) SizePair<x, y>
//struct SizePair {
//    int first;
//    int second;
//};
//
//#ifdef SIZES
//    bool is_static = 1;
//    SizePair<int, int> q = SIZES;
//    //const string sizes_str = SIZES;
//    //vector<SizePair> sizes_str = SIZES;
//#else
//    bool is_static = 0;
//    SizePair<int, int> q;
//    //vector<SizePair> sizes_str;
//#endif
//constexpr size_t N = 36, M = 84;
// constexpr size_t N = 14, M = 5;

// char field[N][M + 1] = {
//     "#####",
//     "#.  #",
//     "#.# #",
//     "#.# #",
//     "#.# #",
//     "#.# #",
//     "#.# #",
//     "#.# #",
//     "#...#",
//     "#####",
//     "#   #",
//     "#   #",
//     "#   #",
//     "#####",
// };


template <class... Types>
struct TypesList;

struct SizePair {
    size_t rows{};
    size_t columns{};
};

#define S(N, M)                  \
    SizePair {            \
        .rows = N, .columns = M, \
    }

struct Params {
    float rho_air;
    int rho_fluid;
    float g;
};


//template <class PElementType,
//        class VelocityElementType,
//        class VelocityFlowElementType,
//        std::size_t Rows    = mxSize,
//        std::size_t Columns = mxSize>
//void start_simulation(const std::vector<std::vector<char>>& field, const Params params) {
////    using SimulationSessionType = SimulationSession<PElementType, VelocityElementType,
////            VelocityFlowElementType, Rows, Columns>;
////    SimulationSessionType{field}.start();
//    SmartFluidSim<PElementType, VelocityElementType, VelocityFlowElementType, Rows, Columns>
//            {field, params.rho_air, params.rho_fluid, params.g}.run();
//}
//
//template <class PElementType,
//        class VelocityElementType,
//        class VelocityFlowElementType,
//        SizePair StaticSize,
//        SizePair... StaticSizes>
//void select_size_and_start_simulation_impl(const std::vector<std::vector<char>>& field, const Params params) {
//    if (StaticSize.rows == field.size() && StaticSize.columns == field.front().size()) {
//        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType,
//                StaticSize.rows, StaticSize.columns>(field, params);
//    } else if constexpr (sizeof...(StaticSizes) == 0) {
//        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType>(field, params);
//    } else {
//        select_size_and_start_simulation_impl<PElementType, VelocityElementType,
//                VelocityFlowElementType, StaticSizes...>(field, params);
//    }
//}
//
//template <class PElementType,
//        class VelocityElementType,
//        class VelocityFlowElementType,
//        SizePair... StaticSizes>
//void select_size_and_start_simulation(const std::vector<std::vector<char>>& field, const Params params) {
//    if constexpr (sizeof...(StaticSizes) > 0) {
//        select_size_and_start_simulation_impl<PElementType, VelocityElementType,
//                VelocityFlowElementType, StaticSizes...>(field, params);
//    } else {
//        start_simulation<PElementType, VelocityElementType, VelocityFlowElementType>(field, params);
//    }
//}

template <class type_p,
        class type_v,
        class type_vf,
        size_t rows = mxSize,
        size_t cols = mxSize>
void run_fluid_sim(const vector<vector<char>>& field, const Params params) {
    SmartFluidSim<type_p, type_v, type_vf, rows, cols>{field, params.rho_air, params.rho_fluid, params.g}.run();
}

template <class type_p,
        class type_v,
        class type_vf,
        SizePair curSize,
        SizePair... Sizes>
void choose_static_sizes_impl(const vector<vector<char>>& field, const Params params) {
    static_assert(curSize.rows > 0);
    static_assert(curSize.columns > 0);
    if (curSize.rows == field.size() && curSize.columns == field.front().size() - 1) {
        run_fluid_sim<type_p, type_v, type_vf, curSize.rows, curSize.columns + 1>(field, params);
    } else if constexpr (sizeof...(Sizes) == 0) {
        run_fluid_sim<type_p, type_v, type_vf>(field, params);
    } else {
        choose_static_sizes_impl<type_p, type_v,
                type_vf, Sizes...>(field, params);
    }
}

template <class type_p,
        class type_v,
        class type_vf,
                SizePair... Sizes>
void choose_static_sizes(const vector<vector<char>>& field, const Params params) {
    if constexpr (sizeof...(Sizes) > 0) {
        choose_static_sizes_impl<type_p, type_v,
                type_vf, Sizes...>(field, params);
    } else {
        run_fluid_sim<type_p, type_v, type_vf>(field, params);
    }
}

#define SIZES S(10, 10), S(36,84)

//#ifdef SIZES
//
//using Simulator = Sim<Fixed<32, 16>, FastFixed<64, 16>, Fixed<32, 16>, SIZES>;
//
//#else
//
//using Simulator = types::Simulator<types::TypesList<TYPES>>;
//
//#endif

signed main() {
    ifstream fin;
    fin.open("../input.txt", ios_base::in);
    if (!fin) {
        std::cerr << "Не удалось открыть файл! Ошибка: " << strerror(errno) << std::endl;
        return 1;
    }

    //const int N = 36, M = 84;
    int N, M;
    fin >> N >> M;
    float rho_air, g;
    int rho_fluid;
//    char field1[N][M + 1] = {
//            "####################################################################################",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                       .........                                  #",
//            "#..............#            #           .........                                  #",
//            "#..............#            #           .........                                  #",
//            "#..............#            #           .........                                  #",
//            "#..............#            #                                                      #",
//            "#..............#            #                                                      #",
//            "#..............#            #                                                      #",
//            "#..............#            #                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............#                                                      #",
//            "#..............#............################                     #                 #",
//            "#...........................#....................................#                 #",
//            "#...........................#....................................#                 #",
//            "#...........................#....................................#                 #",
//            "##################################################################                 #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "#                                                                                  #",
//            "####################################################################################",
//    };
    //char field[N][M + 1];
    string line;
    vector<vector<char>> field(N, vector<char>(M + 1));
//    char c;
//    fin >> c;
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < N; ++i) {
        getline(fin, line);
        for (int j = 0; j < M + 1; ++j) {
            field[i][j] = line[j];
            if (j == M) {
                field[i][j] = '\000';
            }
        }
    }
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M + 1; ++j) {
//            if (field[i][j] != field1[i][j]) {
//                cout << i << ' ' << j << ' ' << field[i][j] << ' ' << field1[i][j] << endl;
//                return 0;
//            }
//        }
//    }
   // for ()
    fin >> rho_air >> rho_fluid >> g;
//    FluidSim q = FluidSim<Fixed<32, 16>, FastFixed<64, 16>, Fixed<32, 16>, N, M>(field, rho_air, rho_fluid, g);
//    q.run();
    fin.close();
    Params params = Params{rho_air, rho_fluid, g};
    choose_static_sizes<Fixed<32, 16>, Fixed<32, 16>, Fixed<32, 16>, SIZES>(field, params);
}

//#include "simulation.h"
//#define TYPES FLOAT,FIXED(32, 5)
//#define SIZES S(1, 1)
//#ifndef TYPES
//#error "TYPES is not defined"
//#endif
//
//#ifdef SIZES
//
//using Simulator = types::Simulator<types::TypesList<TYPES>, SIZES>;
//
//#else
//
//using Simulator = types::Simulator<types::TypesList<TYPES>>;
//
//#endif
//
//int main() {
//    const Simulator simulator = Simulator::from_params(types::SimulationParams{
//            .p_type_name      = "FLOAT",
//            .v_type_name      = "FIXED(32, 5)",
//            .v_flow_type_name = "DOUBLE",
//    });
//
//    simulator.start_on_field(std::vector<std::vector<char>>{
//            {'#', '#', '#', '#', '#'},
//            {'#', '#', '.', '.', '#'},
//            {'#', ' ', ' ', ' ', '#'},
//            {'#', '#', '#', '#', '#'},
//    });
//}