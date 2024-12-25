//
// Created by artem on 18.12.2024.
//

#ifndef FLUID_FASTFIXED_H
#define FLUID_FASTFIXED_H

#include <bits/stdc++.h>
#include "Fixed.h"

using namespace std;

template <size_t N, size_t K>
class Fixed;

template <size_t N, size_t K>
class FastFixed {
    static constexpr size_t AdjustedN = (N <= 8) ? 8 : (N <= 16) ? 16 : (N <= 32) ? 32 : 64;
    using fast_int_type = std::conditional_t<AdjustedN == 8, int_fast8_t,
            std::conditional_t<AdjustedN == 16, int_fast16_t,
                    std::conditional_t<AdjustedN == 32, int_fast32_t, int_fast64_t>>>;

    fast_int_type v;

    constexpr static fast_int_type scaling_factor = 1ULL << K;

public:
    static constexpr std::size_t Nval = N;
    static constexpr std::size_t Kval = K;
    static constexpr bool IsFast          = true;
    constexpr FastFixed(int value) : v(value << K) {}
    constexpr FastFixed(float value) : v(static_cast<fast_int_type>(value * scaling_factor)) {}
    constexpr FastFixed(double value) : v(static_cast<fast_int_type>(value * scaling_factor)) {}
    template <size_t N2, size_t K2>
    constexpr FastFixed(const FastFixed<N2, K2>& other) {
        if constexpr (K2 > K) {
            v = static_cast<fast_int_type>(other.raw_value() >> (K2 - K));
        } else {
            v = static_cast<fast_int_type>(other.raw_value() << (K - K2));
        }
    }

    template <size_t N2, size_t K2>
    constexpr FastFixed(const Fixed<N2, K2>& other) {
        if constexpr (K2 > K) {
            v = static_cast<fast_int_type>(other.raw_value() >> (K2 - K));
        } else {
            v = static_cast<fast_int_type>(other.raw_value() << (K - K2));
        }
    }

    constexpr FastFixed() : v(0) {}

    constexpr static FastFixed from_raw(fast_int_type raw_value) {
        FastFixed result;
        result.v = raw_value;
        return result;
    }

    static size_t getK() {
        return K;
    }

    template <size_t N1, size_t K1>
    auto operator<(const FastFixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v < other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator<=(const FastFixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v <= other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator<=(const Fixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<Fixed<N, K>>(other);
        return v <= other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator>(const FastFixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v > other1.v;
    };

    template <size_t N1, size_t K1>
    auto operator>=(const FastFixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v >= other1.v;
    };

    template <size_t N1, size_t K1>
    bool operator==(const FastFixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v == other1.v;
    };

    template <size_t N1, size_t K1>
    bool operator==(const Fixed<N1, K1>& other) const {
        FastFixed<N, K> other1 = static_cast<FastFixed<N, K>>(other);
        return v == other1.v;
    };

    template <size_t N1, size_t K1>
    friend FastFixed operator+(FastFixed a, FastFixed<N1, K1> b) {
        return FastFixed::from_raw(a.v + static_cast<FastFixed<N, K>>(b).v);
    }

    template <size_t N1, size_t K1>
    friend FastFixed operator-(FastFixed a, FastFixed<N1, K1> b) {
        return FastFixed::from_raw(a.v - static_cast<FastFixed<N, K>>(b).v);
    }

    template <size_t N1, size_t K1>
    friend FastFixed operator*(FastFixed a, FastFixed<N1, K1> b) {
        using wide_type = typename std::conditional<N <= 32, int64_t, __int128>::type;
        return FastFixed::from_raw(static_cast<fast_int_type>((static_cast<wide_type>(a.v) * static_cast<FastFixed<N, K>>(b).v) >> K));
    }

    template <size_t N1, size_t K1>
    friend FastFixed operator/(FastFixed a, FastFixed<N1, K1> b) {
        using wide_type = typename std::conditional<N <= 32, int64_t, __int128>::type;
        return FastFixed::from_raw(static_cast<fast_int_type>((static_cast<wide_type>(a.v) << K) / static_cast<FastFixed<N, K>>(b).v));
    }

    template <size_t N1, size_t K1>
    FastFixed& operator+=(FastFixed<N1, K1> other) {
        v += static_cast<FastFixed<N, K>>(other).v;
        return *this;
    }

    template <size_t N1, size_t K1>
    FastFixed& operator-=(FastFixed<N1, K1> other) {
        v -= static_cast<FastFixed<N, K>>(other).v;
        return *this;
    }

    template <size_t N1, size_t K1>
    FastFixed& operator*=(FastFixed<N1, K1> other) {
        *this = *this * static_cast<FastFixed<N, K>>(other);
        return *this;
    }

    template <size_t N1, size_t K1>
    FastFixed& operator/=(FastFixed<N1, K1> other) {
        *this = *this / static_cast<FastFixed<N, K>>(other);
        return *this;
    }

    FastFixed operator-() const {
        return FastFixed::from_raw(-v);
    }

    template <size_t N1, size_t K1>
    friend FastFixed abs(FastFixed<N1, K1> x) {
        return x.v < 0 ? FastFixed::from_raw(-static_cast<FastFixed<N, K>>(x).v) : x;
    }

    friend std::ostream& operator<<(std::ostream& out, FastFixed x) {
        return out << static_cast<double>(x.v) / scaling_factor;
    }

    explicit operator float() const {
        return static_cast<float>(v) / scaling_factor;
    }

    explicit operator double() const {
        return static_cast<double>(v) / scaling_factor;
    }

    constexpr fast_int_type raw_value() const {
        return v;
    }

    template <size_t N1, size_t K1>
    friend FastFixed min(FastFixed& a, FastFixed<N1, K1>& b) {
        FastFixed<N, K> x1 = static_cast<FastFixed<N, K>>(b);
        if (a.v < x1.v) {
            return a;
        } else {
            return x1;
        }
    }

    template <size_t N1, size_t K1>
    friend FastFixed max(FastFixed& a, FastFixed<N1, K1>& b) {
        FastFixed<N, K> x1 = static_cast<FastFixed<N, K>>(b);
        if (a.v < x1.v) {
            return x1;
        } else {
            return a;
        }
    }
};



#endif //FLUID_FASTFIXED_H
