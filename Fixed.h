//
// Created by artem on 15.12.2024.
//

#ifndef FLUID_FIXED_H
#define FLUID_FIXED_H
#include <bits/stdc++.h>
#include "FastFixed.h"
mt19937 rnd(1337);
//using namespace std;
//template <std::size_t N, std::size_t K, bool Fast = false>
//struct Fixed {
//    static constexpr std::size_t kNValue = N;
//    static constexpr std::size_t kKValue = K;
//    static constexpr bool kFast          = Fast;
//
//    constexpr Fixed(int v) : v(static_cast<int64_t>(v) << 16) {}
//    constexpr Fixed(float f) : v(static_cast<int64_t>(f * (1 << 16))) {}
//    constexpr Fixed(double f) : v(static_cast<int64_t>(f * (1 << 16))) {}
//    constexpr Fixed() : v(0) {}
//
//    static constexpr Fixed from_raw(int32_t x) {
//        Fixed ret;
//        ret.v = x;
//        return ret;
//    }
//
//    int64_t v;
//
//    auto operator<=>(const Fixed&) const = default;
//    bool operator==(const Fixed&) const  = default;
//};
//
//template <std::size_t N, std::size_t K>
//using FastFixed = Fixed<N, K, /* Fast */ true>;
//
constexpr size_t T = 1'000'000;
constexpr std::array<pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

template <size_t N, size_t K>
class Fixed {
    //static_assert(N > K, "Number of bits for the integer part must be greater than fractional part.");
    //static_assert(N == 8 || N == 16 || N == 32 || N == 64, "Supported sizes are 8, 16, 32, 64 bits.");

    using storage_type = typename std::conditional<N == 8, int8_t,
            typename std::conditional<N == 16, int16_t,
                    typename std::conditional<N == 32, int32_t, int64_t>::type>::type>::type;

    storage_type v;

    constexpr static storage_type scaling_factor = 1ULL << K;

public:
    static constexpr std::size_t kNValue = N;
    static constexpr std::size_t kKValue = K;
    static constexpr bool kFast          = false;
    constexpr Fixed(int value) : v(value << K) {}
    constexpr Fixed(float value) : v(static_cast<storage_type>(value * scaling_factor)) {}
    constexpr Fixed(double value) : v(static_cast<storage_type>(value * scaling_factor)) {}
    template <size_t N2, size_t K2>
    constexpr Fixed(const Fixed<N2, K2>& other) {
        if constexpr (K2 > K) {
            v = static_cast<storage_type>(other.raw_value() >> (K2 - K));
        } else {
            v = static_cast<storage_type>(other.raw_value() << (K - K2));
        }
    }

    template <size_t N2, size_t K2>
    constexpr Fixed(const FastFixed<N2, K2>& other) {
        if constexpr (K2 > K) {
            v = static_cast<storage_type>(other.raw_value() >> (K2 - K));
        } else {
            v = static_cast<storage_type>(other.raw_value() << (K - K2));
        }
    }
    constexpr Fixed() : v(0) {}

    constexpr static Fixed from_raw(storage_type raw_value) {
        Fixed result;
        result.v = raw_value;
        return result;
    }

    static size_t getK() {
        return K;
    }

    template <size_t N1, size_t K1>
    auto operator<(const Fixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v < other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator<=(const Fixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v <= other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator<=(const FastFixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v <= other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator>(const Fixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v > other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator>=(const Fixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v >= other1.v;
    };

    template <size_t N1, size_t K1>
    bool operator==(const Fixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v == other1.v;
    };

    template <size_t N1, size_t K1>
    bool operator==(const FastFixed<N1, K1>& other) const {
        Fixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v == other1.v;
    };

    template <size_t N1, size_t K1>
    friend Fixed operator+(Fixed a, Fixed<N1, K1> b) {
        return Fixed::from_raw(a.v + static_cast<Fixed<N, K>>(b).v);
    }

    template <size_t N1, size_t K1>
    friend Fixed operator-(Fixed a, Fixed<N1, K1> b) {
        return Fixed::from_raw(a.v - static_cast<Fixed<N, K>>(b).v);
    }

    template <size_t N1, size_t K1>
    friend Fixed operator*(Fixed a, Fixed<N1, K1> b) {
        using wide_type = typename std::conditional<N <= 32, int64_t, __int128>::type;
        return Fixed::from_raw(static_cast<storage_type>((static_cast<wide_type>(a.v) * static_cast<Fixed<N, K>>(b).v) >> K));
    }

    template <size_t N1, size_t K1>
    friend Fixed operator/(Fixed a, Fixed<N1, K1> b) {
        using wide_type = typename std::conditional<N <= 32, int64_t, __int128>::type;
        return Fixed::from_raw(static_cast<storage_type>((static_cast<wide_type>(a.v) << K) / static_cast<Fixed<N, K>>(b).v));
    }

    template <size_t N1, size_t K1>
    Fixed& operator+=(Fixed<N1, K1> other) {
        v += static_cast<Fixed<N, K>>(other).v;
        return *this;
    }

    template <size_t N1, size_t K1>
    Fixed& operator-=(Fixed<N1, K1> other) {
        v -= static_cast<Fixed<N, K>>(other).v;
        return *this;
    }

    template <size_t N1, size_t K1>
    Fixed& operator*=(Fixed<N1, K1> other) {
        *this = *this * static_cast<Fixed<N, K>>(other);
        return *this;
    }

    template <size_t N1, size_t K1>
    Fixed& operator/=(Fixed<N1, K1> other) {
        *this = *this / static_cast<Fixed<N, K>>(other);
        return *this;
    }

    Fixed operator-() const {
        return Fixed::from_raw(-v);
    }

    template <size_t N1, size_t K1>
    friend Fixed abs(Fixed<N1, K1> x) {
        return x.v < 0 ? Fixed::from_raw(-static_cast<Fixed<N, K>>(x).v) : x;
    }

    friend std::ostream& operator<<(std::ostream& out, Fixed x) {
        return out << static_cast<double>(x.v) / scaling_factor;
    }

    explicit operator float() const {
        return static_cast<float>(v) / scaling_factor;
    }

    explicit operator double() const {
        return static_cast<double>(v) / scaling_factor;
    }

    constexpr storage_type raw_value() const {
        return v;
    }

    template <size_t N1, size_t K1>
    friend Fixed min(Fixed& a, Fixed<N1, K1>& b) {
        Fixed<N, K> x1 = static_cast<Fixed<N, K>>(b);
        if (a.v < x1.v) {
            return a;
        } else {
            return x1;
        }
    }

    template <size_t N1, size_t K1>
    friend Fixed max(Fixed& a, Fixed<N1, K1>& b) {
        Fixed<N, K> x1 = static_cast<Fixed<N, K>>(b);
        if (a.v < x1.v) {
            return x1;
        } else {
            return a;
        }
    }
};




#endif //FLUID_FIXED_H
