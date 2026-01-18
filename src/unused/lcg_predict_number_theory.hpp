// 2026-01  lcg_predict_number_theory.hpp

#pragma once

#ifndef LCG_PREDICT_NUMBER_THEORY_HPP_INCLUDED
#define LCG_PREDICT_NUMBER_THEORY_HPP_INCLUDED

#include "../include/lcg_predict.hpp"

#include <cassert>
#include <concepts>
#include <limits>
#include <optional>

namespace ls_hower::lcg_predict::detail {

template <std::unsigned_integral T>
struct ExtendedGCDResult {
private:
    using SignedBigger = least_doubled_int_t<T>;

public:
    T d;
    SignedBigger x;
    SignedBigger y;
};

// gcd(a, b) = d = ax + by
template <std::unsigned_integral T>
[[nodiscard]] constexpr auto gcd_ext(T a, T b) noexcept -> ExtendedGCDResult<T>
{
    if (b == 0) {
        return { a, 1, 0 };
    }

    const auto [d2, x2, y2] { gcd_ext(b, a % b) };
    return { d2, y2, x2 - ((a / b) * y2) };
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr auto inv_mod(T a, UnsignedModder<T> modder) noexcept -> std::optional<T>
{
    using UnsignedBigger = least_doubled_uint_t<T>;
    using SignedBigger = least_doubled_int_t<T>;

    if (a == 0) {
        return modder.m == 1 ? std::optional<T> { 0 } : std::nullopt;
    }
    if (a == 1) {
        return modder(1);
    }

    const auto [d, x, y] {
        [a, modder]() noexcept -> ExtendedGCDResult<T> {
            if (modder.m > 0) {
                return gcd_ext(a, modder.m);
            }

            // Real m is 1 bigger than std::numeric_limits<T>::max().
            constexpr auto real_m { modder.template real_m<UnsignedBigger>() };
            assert(real_m == static_cast<UnsignedBigger>(std::numeric_limits<T>::max()) + 1);
            const auto real_m_mod_a { static_cast<T>(real_m % a) };
            const auto real_m_div_a { static_cast<T>(real_m / a) }; // Case where a == 1 or 0 is handled above. So T is able represent the result.

            // gcdext(a, m % a):
            const auto [d3, x3, y3] { gcd_ext<T>(a, real_m_mod_a) };
            // gcdext(m, a):
            const auto [d2, x2, y2] { ExtendedGCDResult<T> { d3, y3, x3 - (static_cast<SignedBigger>(real_m_div_a) * y3) } };
            // gcdext(a, m):
            return { d2, y2, x2 };
        }()
    };

    if (d != 1) {
        return std::nullopt;
    }
    const auto result_wide { modder(x) };
    assert(result_wide >= 0);
    assert(result_wide <= std::numeric_limits<T>::max());
    const auto result { static_cast<T>(result_wide) };
    assert(modder.times_mod(a, result) == 1);
    return result;
}

} // namespace ls_hower::lcg_predict::detail

#endif // LCG_PREDICT_NUMBER_THEORY_HPP_INCLUDED
