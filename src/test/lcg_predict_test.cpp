// 2026-01  lcg_predict_test.cpp

#include "../include/lcg_predict.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>

namespace {

using namespace ls_hower::lcg_predict;

template <unsigned long long step, std::unsigned_integral T>
[[nodiscard]] constexpr auto get_prediction(const LCGEngine<T>& engine) noexcept -> std::array<T, step>
{
    const auto prediction_vw {
        std::views::iota(0ULL, step)
        | std::views::transform(
            [engine](unsigned long long i) noexcept -> T {
                assert(i < std::numeric_limits<unsigned long long>::max());
                return engine.value_after_n_steps(i + 1);
            })
    };
    std::array<T, step> prediction {};
    std::ranges::copy(prediction_vw, prediction.begin());
    return prediction;
}

template <unsigned long long step, std::unsigned_integral T>
[[nodiscard]] constexpr auto get_simulation(const LCGEngine<T>& engine) noexcept -> std::array<T, step>
{
    std::array<T, step> simulation {};
    std::ranges::generate(simulation, engine);
    return simulation;
}

template <unsigned long long step, std::unsigned_integral T>
[[nodiscard]] constexpr auto prediction_actual_same(const LCGEngine<T>& engine, const std::array<T, step>& actual) noexcept -> bool
{
    return std::ranges::equal(get_prediction<step, T>(engine), actual);
}

template <unsigned long long step, std::unsigned_integral T>
[[nodiscard]] constexpr auto simulation_actual_same(const LCGEngine<T>& engine, const std::array<T, step>& actual) noexcept -> bool
{
    return std::ranges::equal(get_simulation<step, T>(engine), actual);
}

template <unsigned long long step, std::unsigned_integral T>
[[nodiscard]] constexpr auto prediction_simulation_same(const LCGEngine<T>& engine) noexcept -> bool
{
    return std::ranges::equal(get_prediction<step, T>(engine), get_simulation<step, T>(engine));
}

// https://oeis.org/A096553 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> krc_lcg_actual {
    1103527590, 377401575, 662824084, 1147902781, 2035015474,
    368800899, 1508029952, 486256185, 1062517886, 267834847
};

static_assert(prediction_actual_same(krc_rand_engine, krc_lcg_actual));
static_assert(simulation_actual_same(krc_rand_engine, krc_lcg_actual));
static_assert(prediction_simulation_same<1000>(krc_rand_engine));

// Operations on `std::linear_congruential_engine` are not `constexpr`, so actual values are hard-coded.
// https://oeis.org/A221556
constexpr inline std::array<std::uint_fast32_t, 10> minstd_rand_actual {
    48271, 182605794, 1291394886, 1914720637, 2078669041,
    407355683, 1105902161, 854716505, 564586691, 1596680831
};

static_assert(prediction_actual_same(minstd_rand_engine, minstd_rand_actual));
static_assert(simulation_actual_same(minstd_rand_engine, minstd_rand_actual));
static_assert(prediction_simulation_same<1000>(minstd_rand_engine));
static_assert(minstd_rand_engine.value_after_n_steps(10000) == 399268537);

// Operations on `std::linear_congruential_engine` are not `constexpr`, so actual values are hard-coded.
// https://oeis.org/A096550 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> minstd_rand0_actual {
    16807, 282475249, 1622650073, 984943658, 1144108930,
    470211272, 101027544, 1457850878, 1458777923, 2007237709
};

static_assert(prediction_actual_same(minstd_rand0_engine, minstd_rand0_actual));
static_assert(simulation_actual_same(minstd_rand0_engine, minstd_rand0_actual));
static_assert(prediction_simulation_same<1000>(minstd_rand0_engine));
static_assert(minstd_rand0_engine.value_after_n_steps(10000) == 1043618065);

// https://oeis.org/A384331 (without the first term 1)
constexpr inline std::array<std::uint_fast32_t, 10> msvc_rand_actual {
    2745024, 1210316419, 415139642, 1736732949, 1256316804,
    1030492215, 752224798, 1924036713, 1766988168, 1603301931
};

static_assert(prediction_actual_same(msvc_rand_engine, msvc_rand_actual));
static_assert(simulation_actual_same(msvc_rand_engine, msvc_rand_actual));
static_assert(prediction_simulation_same<1000>(msvc_rand_engine));

// https://oeis.org/A382305 (without the first term 1)
constexpr inline std::array<std::uint_fast64_t, 10> rand48_actual {
    25214903928, 206026503483683, 245470556921330, 105707381795861, 223576932655868,
    102497929776471, 87262199322646, 266094224901481, 44061996164032, 147838658590923
};

static_assert(prediction_actual_same(posix_rand48_engine, rand48_actual));
static_assert(simulation_actual_same(posix_rand48_engine, rand48_actual));
static_assert(prediction_simulation_same<1000>(posix_rand48_engine));

constexpr inline std::array<std::uint_fast64_t, 10> musl_rand_actual {
    6364136223846793006U, 13885033948157127959U, 14678909342070756876U, 14340359694176818205U, 3490389784639564826U,
    2377159206977889939U, 11136134660641191128U, 5776246781640716793U, 12360490266823512006U, 7783159857423531983U
};

static_assert(prediction_actual_same(musl_rand_engine, musl_rand_actual));
static_assert(simulation_actual_same(musl_rand_engine, musl_rand_actual));
static_assert(prediction_simulation_same<1000>(musl_rand_engine));

}

auto main() -> int
{
}
