// 2026-01  lcg_predict.hpp

#pragma once

#ifndef LCG_PREDICT_HPP_INCLUDED
#define LCG_PREDICT_HPP_INCLUDED

#define LCG_PREDICT_TEST

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <type_traits>
#include <utility>

#ifdef LCG_PREDICT_TEST
#include <array>
#include <ranges>
#endif // LCG_PREDICT_TEST

namespace ls_hower::lcg_predict::detail {

template <typename T>
concept unsigned_integer_like = std::unsigned_integral<T> || std::same_as<T, __uint128_t>;

template <typename T>
concept signed_integer_like = std::signed_integral<T> || std::same_as<T, __int128_t>;

template <typename T>
concept integer_like = std::integral<T> || std::same_as<T, __int128_t> || std::same_as<T, __uint128_t>;

template <std::integral T>
struct least_doubled_uint {
private:
    template <unsigned_integer_like Head, unsigned_integer_like... Tail>
    struct finder {
        using type = std::conditional_t<
            sizeof(Head) >= 2 * sizeof(T),
            Head,
            typename finder<Tail...>::type>;
    };

    template <unsigned_integer_like Last>
    struct finder<Last> {
        using type = Last;
    };

    template <unsigned_integer_like... Ts>
    using finder_t = finder<Ts...>::type;

public:
    using type = finder_t<
        unsigned int,
        unsigned long,
        unsigned long long,
        __uint128_t>;
    static_assert(unsigned_integer_like<type>);
    static_assert(sizeof(type) >= 2 * sizeof(T));
};

template <std::integral T>
using least_doubled_uint_t = typename least_doubled_uint<T>::type;

template <std::integral T>
struct least_doubled_int {
private:
    using UnsignedBigger = least_doubled_uint_t<T>;

    template <bool, typename U>
    struct signed_chooser {
        using type = __int128_t;
    };

    template <typename U>
    struct signed_chooser<false, U> {
        using type = std::make_signed_t<U>;
    };

public:
    using type = signed_chooser<std::is_same_v<UnsignedBigger, __uint128_t>, UnsignedBigger>::type;
    static_assert(signed_integer_like<type>);
    static_assert(sizeof(type) >= 2 * sizeof(T));
};

template <std::integral T>
using least_doubled_int_t = typename least_doubled_int<T>::type;

template <typename Op, typename T>
concept BinaryClosure = requires(Op op, T x, T y) {
    { op(x, y) } -> std::convertible_to<T>;
};

// Generalized fast pow algorithm.
// Require: `op` be associative and commutative.
// Require: `unit` be the identity for `op`.
// Returns: (elem op elem op ... op elem). There are `n` `elem`'s. If `n` == 0, returns `unit`.
template <typename T, BinaryClosure<T> Op>
[[nodiscard]] constexpr auto double_and_add(const T& elem, unsigned long long n, const Op& op, const T& unit) -> T
{
    T result { unit };
    T var_elem { elem };
    for (/* void */; n != 0; n >>= 1U) {
        if ((n & 1U) != 0) {
            result = op(std::move(result), var_elem);
        }
        var_elem = op(var_elem, var_elem);
    }
    return result;
}

// When m == 0, the modulus mathematically equals (numeric_limits<T>::max() + 1).
template <std::unsigned_integral T>
struct UnsignedModder {
private:
    using UnsignedBigger = least_doubled_uint_t<T>;
    using SignedBigger = least_doubled_int_t<T>;

    [[nodiscard]] constexpr auto internal_sum(T x) const noexcept -> UnsignedBigger
    {
        return static_cast<UnsignedBigger>(x);
    }

    template <typename... Tail>
    [[nodiscard]] constexpr auto internal_sum(T x, Tail... tail) const noexcept -> UnsignedBigger
    {
        return static_cast<UnsignedBigger>(x) + internal_sum(tail...);
    }

    [[nodiscard]] constexpr auto internal_prod_mod(T x) const noexcept -> UnsignedBigger
    {
        return (*this)(static_cast<UnsignedBigger>(x));
    }

    template <typename... Tail>
    [[nodiscard]] constexpr auto internal_prod_mod(T x, Tail... tail) const noexcept -> UnsignedBigger
    {
        return (*this)(static_cast<UnsignedBigger>(x) * internal_prod_mod(tail...));
    }

public:
    T m;

    template <typename Bigger>
    [[nodiscard]] constexpr auto real_m() const noexcept -> Bigger
        requires integer_like<Bigger> && (sizeof(Bigger) > sizeof(T))
    {
        return m != 0 ? m : static_cast<Bigger>(std::numeric_limits<T>::max()) + 1;
    }

    template <typename U>
    [[nodiscard]] constexpr auto operator()(U x) const noexcept -> T
        requires unsigned_integer_like<U> && (sizeof(U) <= sizeof(UnsignedBigger))
    {
        const auto real_m { this->real_m<UnsignedBigger>() };
        return static_cast<T>(x % real_m);
    }

    template <typename U>
    [[nodiscard]] constexpr auto operator()(U x) const noexcept -> T
        requires signed_integer_like<U> && (sizeof(U) <= sizeof(SignedBigger))
    {
        const auto real_m { this->real_m<SignedBigger>() };
        SignedBigger result { static_cast<SignedBigger>(x) % real_m };
        return static_cast<T>(result < 0 ? result + real_m : result);
    }

    [[nodiscard]] constexpr auto mod(T x) const noexcept -> T
    {
        return (*this)(x);
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto plus_mod(Args... args) const noexcept -> T
    {
        return (*this)(internal_sum(args...));
    }

    [[nodiscard]] constexpr auto minus_mod(T x, T y) const noexcept -> T
    {
        return (*this)(static_cast<UnsignedBigger>(x) - y);
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto times_mod(Args... args) const noexcept -> T
    {
        return internal_prod_mod(args...);
    }

    [[nodiscard]] constexpr auto times_plus_mod(T x, T y, T z) const noexcept -> T
    {
        return (*this)((static_cast<UnsignedBigger>(x) * y) + z);
    }

    [[nodiscard]] constexpr auto times_plus_plus_mod(T x, T y, T z, T w) const noexcept -> T
    {
        return (*this)((static_cast<UnsignedBigger>(x) * y) + z + w);
    }

    [[nodiscard]] constexpr auto pow_mod(T base, unsigned long long expo) const noexcept -> T
    {
        const auto mul_mod {
            [*this](T x, T y) noexcept -> T {
                return this->times_mod(x, y);
            }
        };
        return double_and_add(base, expo, mul_mod, (*this)(1));
    }

    [[nodiscard]] constexpr friend auto operator==(UnsignedModder lhs, UnsignedModder rhs) noexcept -> bool = default;
};

template <typename>
struct std_lcg_traits;

template <typename UIntType, UIntType ax, UIntType cx, UIntType mx>
struct std_lcg_traits<std::linear_congruential_engine<UIntType, ax, cx, mx>> {
    using result_type = UIntType;
    static constexpr UIntType a { ax };
    static constexpr UIntType c { cx };
    static constexpr UIntType m { mx };
};

template <typename T, typename UIntType>
concept std_lcg_of = std::same_as<UIntType, typename std_lcg_traits<T>::result_type>;

} // namespace ls_hower::lcg_predict::detail

namespace ls_hower::lcg_predict {

template <typename UIntType, UIntType a, UIntType c, UIntType m>
[[nodiscard]] auto extract_state(const std::linear_congruential_engine<UIntType, a, c, m>& engine) -> UIntType
{
    std::ostringstream oss {};
    oss << engine;
    std::istringstream iss { oss.str() };
    UIntType state {};
    iss >> state;
    return state;
}

template <std::unsigned_integral UIntType>
class LCGAffineTransform {
    detail::UnsignedModder<UIntType> modder_;
    UIntType a_;
    UIntType c_;

public:
    using result_type = UIntType;

    explicit constexpr LCGAffineTransform(UIntType a, UIntType c, UIntType m = 0) noexcept // NOLINT (bugprone-easily-swappable-parameters)
        : modder_ { m }
        , a_ { modder_(a) }
        , c_ { modder_(c) }
    {
    }

    template <typename StdEngine>
    [[nodiscard]] static constexpr auto from_std() noexcept -> LCGAffineTransform
        requires detail::std_lcg_of<StdEngine, UIntType>
    {
        using traits = detail::std_lcg_traits<StdEngine>;
        return LCGAffineTransform { traits::a, traits::c, traits::m };
    }

    constexpr auto set_a(UIntType a) noexcept -> void { a_ = modder_(a); }
    constexpr auto set_c(UIntType c) noexcept -> void { c_ = modder_(c); }
    constexpr auto set_m(UIntType m) noexcept -> void { modder_.m = m; }
    [[nodiscard]] constexpr auto a() const noexcept -> UIntType { return a_; }
    [[nodiscard]] constexpr auto c() const noexcept -> UIntType { return c_; }
    [[nodiscard]] constexpr auto m() const noexcept -> UIntType { return modder_.m; }

    [[nodiscard]] constexpr auto operator()(result_type x) const noexcept -> result_type
    {
        return modder_.times_plus_mod(a_, x, c_);
    }

    constexpr auto operator+=(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&
    {
        assert(modder_ == rhs.modder_);
        a_ = modder_.plus_mod(a_, rhs.a_);
        c_ = modder_.plus_mod(c_, rhs.c_);
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform
    {
        lhs += rhs;
        return lhs;
    }

    constexpr auto operator-=(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&
    {
        assert(modder_ == rhs.modder_);
        a_ = modder_.minus_mod(a_, rhs.a_);
        c_ = modder_.minus_mod(c_, rhs.c_);
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator-(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform
    {
        lhs -= rhs;
        return lhs;
    }

    constexpr auto compose_assign(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&
    {
        assert(modder_ == rhs.modder_);
        const UIntType new_a { modder_.times_mod(a_, rhs.a_) };
        const UIntType new_c { modder_.times_plus_mod(a_, rhs.c_, c_) };
        a_ = new_a;
        c_ = new_c;
        return *this;
    }

    // Returns f such that f(x) = lhs(rhs(x)).
    [[nodiscard]] friend constexpr auto compose(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform
    {
        lhs.compose_assign(rhs);
        return lhs;
    }

    [[nodiscard]] constexpr auto identity() const noexcept -> LCGAffineTransform
    {
        return LCGAffineTransform { 1, 0, modder_.m };
    }

    [[nodiscard]] constexpr auto powered(unsigned long long n) const noexcept -> LCGAffineTransform
    {
        // Use ADL to find the hidden friend `compose`.
        const auto composer {
            [](const LCGAffineTransform& lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform {
                return compose(lhs, rhs);
            }
        };
        return detail::double_and_add(*this, n, composer, this->identity());
    }

    [[nodiscard]] constexpr auto min() const noexcept -> result_type { return c_ == 0U ? 1U : 0U; }
    [[nodiscard]] constexpr auto max() const noexcept -> result_type { return modder_.m - 1U; }
    [[nodiscard]] friend constexpr auto operator==(const LCGAffineTransform& lhs, const LCGAffineTransform& rhs) noexcept -> bool = default;
};

template <std::unsigned_integral UIntType>
class LCGEngine {
public:
    using result_type = UIntType;

private:
    using Affine = LCGAffineTransform<UIntType>;

    Affine affine_;
    result_type state_;

    [[nodiscard]] constexpr auto modder() const noexcept -> detail::UnsignedModder<UIntType>
    {
        return detail::UnsignedModder<UIntType> { affine_.m() };
    }

public:
    static constexpr UIntType default_seed { 1U };

    explicit constexpr LCGEngine(Affine affine, result_type state = default_seed) noexcept
        : affine_ { affine }
        , state_ { this->modder()(state) }
    {
    }

    explicit constexpr LCGEngine(UIntType a, UIntType c, UIntType m = 0, result_type state = default_seed) noexcept
        : LCGEngine { Affine { a, c, m }, state }
    {
    }

    // Operations on `std::linear_congruential_engine` are not `constexpr`.
    // So the factory function is not `constexpr`.
    template <typename StdEngine>
    [[nodiscard]] static auto from_std(StdEngine engine) noexcept -> LCGEngine
        requires detail::std_lcg_of<StdEngine, UIntType>
    {
        return LCGEngine {
            Affine::template from_std<StdEngine>(),
            extract_state(engine),
        };
    }

    [[nodiscard]] constexpr auto operator()() noexcept -> result_type
    {
        state_ = affine_(state_);
        return state_;
    }

    [[nodiscard]] constexpr auto value_after_n_steps(unsigned long long steps) const noexcept -> result_type
    {
        return affine_.powered(steps)(state_);
    }

    constexpr auto discard(unsigned long long n) noexcept -> void
    {
        state_ = this->value_after_n_steps(n);
    }

    [[nodiscard]] constexpr auto a() const noexcept -> UIntType { return affine_.a(); }
    [[nodiscard]] constexpr auto c() const noexcept -> UIntType { return affine_.c(); }
    [[nodiscard]] constexpr auto m() const noexcept -> UIntType { return affine_.m(); }
    [[nodiscard]] constexpr auto affine() const noexcept -> Affine { return affine_; }
    [[nodiscard]] constexpr auto state() const noexcept -> result_type { return state_; }
    constexpr auto set_a(UIntType new_a) noexcept -> void { affine_.set_a(new_a); }
    constexpr auto set_c(UIntType new_c) noexcept -> void { affine_.set_c(new_c); }
    constexpr auto set_m(UIntType new_m) noexcept -> void { affine_.set_m(new_m); }
    constexpr auto set_affine(const Affine& new_affine) noexcept -> void { affine_ = new_affine; }
    constexpr auto set_state(result_type new_seed) noexcept -> void { state_ = this->modder()(new_seed); }

    [[nodiscard]] friend constexpr auto operator==(const LCGEngine& lhs, const LCGEngine& rhs) noexcept -> bool = default;
};

// LCG suggested in K&R C and C standards.
constexpr inline LCGEngine<std::uint_fast32_t> krc_rand_engine { 1103515245, 12345, 2147483648 };

// C++ `std::minstd_rand`.
constexpr inline LCGEngine<std::uint_fast32_t> minstd_rand_engine { 48271, 0, 2147483647 };

// C++ `std::minstd_rand0`.
constexpr inline LCGEngine<std::uint_fast32_t> minstd_rand0_engine { 16807, 0, 2147483647 };

// MSVC `std::rand`.
constexpr inline LCGEngine<std::uint_fast32_t> msvc_rand_engine { 214013, 2531011, 2147483648 };

// POSIX `*rand48`.
constexpr inline LCGEngine<std::uint_fast64_t> posix_rand48_engine { 25214903917, 11, 281474976710656 };

// Musl `rand`.
constexpr inline LCGEngine<std::uint_fast64_t> musl_rand_engine { 6364136223846793005, 1, 0 };

} // namespace ls_hower::lcg_predict

#ifdef LCG_PREDICT_TEST

namespace ls_hower::lcg_predict::detail::test {

template <unsigned long long step, std::unsigned_integral T>
    requires(step != std::numeric_limits<unsigned long long>::max())
[[nodiscard]] constexpr auto get_prediction(const LCGEngine<T>& engine) -> std::array<T, step>
{
    const auto prediction_vw {
        std::views::iota(0ULL, step)
        | std::views::transform(
            [engine](unsigned long long i) noexcept -> T {
                return engine.value_after_n_steps(i + 1);
            })
    };
    std::array<T, step> prediction {};
    std::ranges::copy(prediction_vw, prediction.begin());
    return prediction;
}

template <unsigned long long step, std::unsigned_integral T>
    requires(step != std::numeric_limits<unsigned long long>::max())
[[nodiscard]] constexpr auto get_simulation(const LCGEngine<T>& engine) -> std::array<T, step>
{
    std::array<T, step> simulation {};
    std::ranges::generate(simulation, engine);
    return simulation;
}

template <unsigned long long step, std::unsigned_integral T>
    requires(step != std::numeric_limits<unsigned long long>::max())
[[nodiscard]] constexpr auto prediction_actual_same(const LCGEngine<T>& engine, const std::array<T, step>& actual) -> bool
{
    return std::ranges::equal(get_prediction<step, T>(engine), actual);
}

template <unsigned long long step, std::unsigned_integral T>
    requires(step != std::numeric_limits<unsigned long long>::max())
[[nodiscard]] constexpr auto simulation_actual_same(const LCGEngine<T>& engine, const std::array<T, step>& actual) -> bool
{
    return std::ranges::equal(get_simulation<step, T>(engine), actual);
}

// https://oeis.org/A096553 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> krc_lcg_actual {
    1103527590, 377401575, 662824084, 1147902781, 2035015474,
    368800899, 1508029952, 486256185, 1062517886, 267834847
};

static_assert(prediction_actual_same(krc_rand_engine, krc_lcg_actual));
static_assert(simulation_actual_same(krc_rand_engine, krc_lcg_actual));

// Operations on `std::linear_congruential_engine` are not `constexpr`, so actual values are hard-coded.
// https://oeis.org/A221556
constexpr inline std::array<std::uint_fast32_t, 10> minstd_rand_actual {
    48271, 182605794, 1291394886, 1914720637, 2078669041,
    407355683, 1105902161, 854716505, 564586691, 1596680831
};

static_assert(prediction_actual_same(minstd_rand_engine, minstd_rand_actual));
static_assert(simulation_actual_same(minstd_rand_engine, minstd_rand_actual));
static_assert(minstd_rand_engine.value_after_n_steps(10000) == 399268537);

// Operations on `std::linear_congruential_engine` are not `constexpr`, so actual values are hard-coded.
// https://oeis.org/A096550 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> minstd_rand0_actual {
    16807, 282475249, 1622650073, 984943658, 1144108930,
    470211272, 101027544, 1457850878, 1458777923, 2007237709
};

static_assert(prediction_actual_same(minstd_rand0_engine, minstd_rand0_actual));
static_assert(simulation_actual_same(minstd_rand0_engine, minstd_rand0_actual));
static_assert(minstd_rand0_engine.value_after_n_steps(10000) == 1043618065);

// https://oeis.org/A384331 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> msvc_rand_actual {
    2745024, 1210316419, 415139642, 1736732949, 1256316804,
    1030492215, 752224798, 1924036713, 1766988168, 1603301931
};

static_assert(prediction_actual_same(msvc_rand_engine, msvc_rand_actual));
static_assert(simulation_actual_same(msvc_rand_engine, msvc_rand_actual));

// https://oeis.org/A382305 (without the first term 1)
constexpr inline std::array<std::uint_fast64_t, 10> rand48_actual {
    25214903928, 206026503483683, 245470556921330, 105707381795861, 223576932655868,
    102497929776471, 87262199322646, 266094224901481, 44061996164032, 147838658590923
};

static_assert(prediction_actual_same(posix_rand48_engine, rand48_actual));
static_assert(simulation_actual_same(posix_rand48_engine, rand48_actual));

constexpr inline std::array<std::uint_fast64_t, 10> musl_rand_actual {
    6364136223846793006U, 13885033948157127959U, 14678909342070756876U, 14340359694176818205U, 3490389784639564826U,
    2377159206977889939U, 11136134660641191128U, 5776246781640716793U, 12360490266823512006U, 7783159857423531983U
};

static_assert(prediction_actual_same(musl_rand_engine, musl_rand_actual));
static_assert(simulation_actual_same(musl_rand_engine, musl_rand_actual));
} // namespace ls_hower::lcg_predict::detail::test

#endif // LCG_PREDICT_TEST

#endif // LCG_PREDICT_HPP_INCLUDED
