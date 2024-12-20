//гуглил гптшил говнокодил, так как на лекциях ничего подобного не было(, но вроде разобрался и могу пояснить как тут все работает)
#include <bits/stdc++.h>
#include "oldfiles/FluidSim.h"
#include "SmartFluidSim.h"
using namespace std;
#define FLOAT            float
#define DOUBLE           double
#define FAST_FIXED(N, K) FastFixed<N, K>
#define FIXED(N, K)      Fixed<N, K>

#define STRINGIFY_EXACT_NO_EVAL(expr) #expr

inline constexpr std::string_view kFloatTypeName   = STRINGIFY_EXACT_NO_EVAL(FLOAT);
inline constexpr std::string_view kDoubleTypeName  = STRINGIFY_EXACT_NO_EVAL(DOUBLE);
inline constexpr std::string_view kFastFixedPrefix = "FAST_FIXED(";
inline constexpr std::string_view kFastFixedSuffix = ")";
inline constexpr std::string_view kFixedPrefix     = "FIXED(";
inline constexpr std::string_view kFixedSuffix     = ")";

#undef STRINGIFY_EXACT_NO_EVAL

template <class... Types>
struct ListOfTypes;

template <std::size_t I, class Type, class... Types>
struct TypesListGetAtHelper1 {
    static_assert(I < 1 + sizeof...(Types));
    using type = typename TypesListGetAtHelper1<I - 1, Types...>::type;
};

template <class Type, class... Types>
struct TypesListGetAtHelper1<0, Type, Types...> {
    using type = Type;
};

template <std::size_t I, class TypesListType>
struct TypesListGetAtHelper2;

template <std::size_t I, class... Types>
struct TypesListGetAtHelper2<I, ListOfTypes<Types...>> {
    using type = typename TypesListGetAtHelper1<I, Types...>::type;
};

template <std::size_t I, class TypesListType>
using TypesListGetAt = typename TypesListGetAtHelper2<I, TypesListType>::type;

template <std::size_t I, class... Types>
using TypesPackGetAt = typename TypesListGetAtHelper1<I, Types...>::type;

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
    static bool parse_params(string_view type_name,
                                       string_view prefix,
                                       string_view suffix,
                                       size_t& n,
                                       size_t& k) {
        if (!type_name.starts_with(prefix)) {
            return false;
        }
        type_name.remove_prefix(prefix.size());

        if (!type_name.ends_with(suffix)) {
            return false;
        }
        type_name.remove_suffix(suffix.size());

        const size_t sep_char_pos = type_name.find(',');
        if (sep_char_pos >= type_name.size()) {
            return false;
        }

        auto strip_sv = [](string_view s) noexcept {
            while (!s.empty() && isspace(s.front())) {
                s.remove_prefix(1);
            }
            while (!s.empty() && isspace(s.back())) {
                s.remove_suffix(1);
            }
            return s;
        };
        const string_view n_str = strip_sv(type_name.substr(0, sep_char_pos));
        const string_view k_str = strip_sv(type_name.substr(sep_char_pos + 1));

        if (from_chars(n_str.data(), n_str.data() + n_str.size(), n).ec != errc{}) {
            return false;
        }

        if (from_chars(k_str.data(), k_str.data() + k_str.size(), k).ec != errc{}) {
            return false;
        }

        return n > 0 && k > 0;
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
        if (parse_params(type_name, kFastFixedPrefix, kFastFixedSuffix, n, k)) {
            if (handle_fixed_type<true, Args...>(n, k, field, params,
                                                                          type_names...)) {
                return;
            }
        }
        if (parse_params(type_name, kFixedPrefix, kFixedSuffix, n, k)) {
            if (handle_fixed_type<false, Args...>(n, k, field, params,
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


    template <bool Fast, class... Args>
    static bool handle_fixed_type(std::size_t n,
                                                           std::size_t k,
                                                           const vector<vector<char>>& field,
                                                            Params& params,
                                                           Args... type_names) {
        return handle_fixed_type_impl<0, Fast, Args...>(n, k, field, params,
                                                                                 type_names...);
    }

    template <std::size_t I, bool Fast, class... Args>
    static bool handle_fixed_type_impl(std::size_t n,
                                                                std::size_t k,
                                                                const vector<vector<char>>& field,
                                                                Params& params,
                                                                Args... type_names) {
        using FloatType = TypesPackGetAt<I, DefinedTypes...>;

//        cout << typeid(FloatType).name() << endl;
//        bool b = (requires {
//            {
//            FloatType::kFast == bool {}
//            } -> std::same_as<bool>;
//        });
        if constexpr (requires {
            {
            FloatType::kNValue == std::size_t {}
            } -> std::same_as<bool>;
            {
            FloatType::kKValue == std::size_t {}
            } -> std::same_as<bool>;
            {
            FloatType::kFast == bool {}
            } -> std::same_as<bool>;
        })
        {
            if constexpr (FloatType::kFast == Fast) {
                if (FloatType::kNValue == n && FloatType::kKValue == k) {
                    get_next_type<FloatType, Args...>(field, params, type_names...);
                    return true;
                }
            }
        }

        if constexpr (I + 1 < sizeof...(DefinedTypes)) {
            return handle_fixed_type_impl<I + 1, Fast, Args...>(
                    n, k, field, params, type_names...);
        }

        return false;
    }
};

template <class TypesListType, Pair... Sizes>
class MainSim;

struct SimulationParams {
    string_view p_type{};
    string_view v_type{};
    string_view vf_type{};
};

template <class... FloatTypes, Pair... Sizes>
class MainSim<ListOfTypes<FloatTypes...>, Sizes...> {
public:
    MainSim (SimulationParams params1) : given_types(params1){

    }
    SimulationParams given_types;
    void start_on_field(const std::vector<std::vector<char>>& field, Params& params) const {
        TypesSelector<ListOfTypes<FloatTypes...>, ListOfTypes<>, Sizes...>::
        handle_types(
                field, params, given_types.p_type, given_types.v_type, given_types.vf_type);
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
    //choose_static_sizes<float, FastFixed<32, 16>, Fixed<32, 16>, SIZES>(field, params);
    MainSim sim = MainSim<ListOfTypes<TYPES>, SIZES>(SimulationParams{
            .p_type      = "FLOAT",
            .v_type      = "FIXED(32, 16)",
            .vf_type = "FAST_FIXED(32, 16)",
    });
    sim.start_on_field(field, params);
}