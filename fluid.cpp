//гуглил страдал гптшил говнокодил курил стоял много у окна плакал, так как на лекциях ничего подобного не было(, но вроде разобрался и могу пояснить как тут все работает)
#include <bits/stdc++.h>
#include "oldfiles/FluidSim.h"
#include "SmartFluidSim.h"
using namespace std;
#define FLOAT            float
#define DOUBLE           double
#define FAST_FIXED(N, K) FastFixed<N, K>
#define FIXED(N, K)      Fixed<N, K>

constexpr std::string_view kFloatTypeName   = "FLOAT";
constexpr std::string_view kDoubleTypeName  = "DOUBLE";

template <class... Types>
struct ListOfTypes;

template <std::size_t I, class Type, class... Types>
struct Iter {
    static_assert(I < 1 + sizeof...(Types));
    using type = typename Iter<I - 1, Types...>::type;
};

template <class Type, class... Types>
struct Iter<0, Type, Types...> {
    using type = Type;
};

template <std::size_t I, class... Types>
using GetAtI = typename Iter<I, Types...>::type;

struct Pair {
    size_t rows{};
    size_t columns{};
};

#define S(N, M)                  \
    Pair {            \
        .rows = N, .columns = M, \
    }

struct Params {
    float rho_air;
    int rho_fluid;
    float g;
};

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
        Pair curSize,
        Pair... Sizes>
void choose_static_sizes_impl(const vector<vector<char>>& field, const Params params) {
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
                Pair... Sizes>
void choose_static_sizes(const vector<vector<char>>& field, const Params params) {
    if constexpr (sizeof...(Sizes) > 0) {
        choose_static_sizes_impl<type_p, type_v,
                type_vf, Sizes...>(field, params);
    } else {
        run_fluid_sim<type_p, type_v, type_vf>(field, params);
    }
}

#define SIZES S(10, 10), S(36,84)
#define TYPES FIXED(32, 16),FLOAT,DOUBLE,FAST_FIXED(32, 16)
template <class kDefinedTypes, class kSelectedTypes, Pair... Sizes>
class TypesSelector;

template <class... DefinedTypes, class... SelectedTypes,
        Pair... Sizes>
class TypesSelector<ListOfTypes<DefinedTypes...>,
        ListOfTypes<SelectedTypes...>,
        Sizes...> {

    static constexpr bool IsThereFloat =
            disjunction_v<is_same<DefinedTypes, FLOAT>...>;

    static constexpr bool IsThereDouble =
            disjunction_v<is_same<DefinedTypes, DOUBLE>...>;

public:

    static bool parse_fixed_string(string_view input, size_t& n, size_t& k, bool isFast) {
        string_view prefix_fixed = "FIXED(";
        string_view prefix_fast_fixed = "FAST_FIXED(";

        if (input.substr(0, prefix_fixed.size()) == prefix_fixed) {
            if (isFast == true) {
                return false;
            }
            input.remove_prefix(prefix_fixed.size());
        } else if (input.substr(0, prefix_fast_fixed.size()) == prefix_fast_fixed) {
            if (isFast == false) {
                return false;
            }
            input.remove_prefix(prefix_fast_fixed.size());
        } else {
            return false;
        }

        size_t pos = 0;
        n = atoi(input.data());
        while (pos < input.size() && isdigit(input[pos])) {
            ++pos;
        }
        if (pos >= input.size() || input[pos] != ',') {
           return false;
        }
        ++pos;
        while (pos < input.size() && isspace(input[pos])) {
            ++pos;
        }
        k = atoi(input.data() + pos);
        return true;
    }

    template <class... Args>
    static void handle_types(const vector<vector<char>>& field, Params& params,
                                                          std::string_view type_name,
                                                          Args... type_names) {
        if (IsThereFloat && type_name == kFloatTypeName) {
            get_next_type<float, Args...>(field, params, type_names...);
            return;
        }
        if (IsThereDouble && type_name == kDoubleTypeName) {
            get_next_type<double, Args...>(field, params, type_names...);
            return;
        }

        size_t n;
        size_t k;
        if (parse_fixed_string(type_name, n, k, true)) {
            if (handle_fixed_type<0, true, Args...>(n, k, field, params,
                                                                          type_names...)) {
                return;
            }
        }
        if (parse_fixed_string(type_name, n, k, false)) {
            if (handle_fixed_type<0, false, Args...>(n, k, field, params,
                                                                           type_names...)) {
                return;
            }
        }

        throw invalid_argument("Could not parse type " + std::string{type_name});
    }

    template <class FloatType, class... Args>
    static void get_next_type(const vector<vector<char>>& field, Params& params, const Args&... type_names) {
        if constexpr (sizeof...(type_names) > 0) {
            using kDefinedTypes  = ListOfTypes<DefinedTypes...>;
            using kSelectedTypes = ListOfTypes<SelectedTypes..., FloatType>;
            TypesSelector<kDefinedTypes, kSelectedTypes, Sizes...>::
            handle_types(field, params, type_names...);
        } else {
            choose_static_sizes<SelectedTypes..., FloatType, Sizes...>(
                    field, params);
        }
    }

    template <size_t I, bool IsFast, class... Args>
    static bool handle_fixed_type(std::size_t n,
                                                                std::size_t k,
                                                                const vector<vector<char>>& field,
                                                                Params& params,
                                                                Args... type_names) {
        using FloatType = GetAtI<I, DefinedTypes...>;

        if constexpr (requires {
            {
            FloatType::Nval == std::size_t {}
            } -> std::same_as<bool>;
            {
            FloatType::Kval == std::size_t {}
            } -> std::same_as<bool>;
            {
            FloatType::IsFast == bool {}
            } -> std::same_as<bool>;
        })
        {
            if constexpr (FloatType::IsFast == IsFast) {
                if (FloatType::Nval == n && FloatType::Kval == k) {
                    get_next_type<FloatType, Args...>(field, params, type_names...);
                    return true;
                }
            }
        }

        if constexpr (I + 1 < sizeof...(DefinedTypes)) {
            return handle_fixed_type<I + 1, IsFast, Args...>(
                    n, k, field, params, type_names...);
        }

        return false;
    }
};

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
    string line;
    vector<vector<char>> field(N, vector<char>(M + 1));
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

    fin >> rho_air >> rho_fluid >> g;
    fin.close();
    Params params = Params{rho_air, rho_fluid, g};

    TypesSelector<ListOfTypes<TYPES>, ListOfTypes<>, SIZES>::
    handle_types(
            field, params, "FLOAT", "FIXED(32, 16)", "FAST_FIXED(32, 16)");
}