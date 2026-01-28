# lcg-predict
C++20 Header-only library: A "fast jump" algorithm for LCG

## Interface

The header file is `src/include/lcg_predict.hpp`.

Given `T` as an unsigned integer type, the library exposes:
- Class `LCGAffineTransform<T>`: combines three `a`, `c` and `m` of type `T`, describing an affine transformation x -> (ax + c) mod m.
- Class `LCGEngine<T>`: combines an `LCGAffineTransform<T>` and an internal `state` of type `T`.
- Some instances of `LCGEngine<T>`, corresponding to some widely used LCGs.

Interface:

```C++
namespace ls_hower::lcg_predict {

template <std::unsigned_integral UIntType>
class LCGAffineTransform {
public:
    using result_type = UIntType;
    explicit constexpr LCGAffineTransform(UIntType a, UIntType c, UIntType m = 0) noexcept;
    template <typename StdEngine>
    static constexpr auto from_std() noexcept -> LCGAffineTransform
        requires detail::std_lcg_of<StdEngine, UIntType>;
    constexpr auto set_a(UIntType a) noexcept -> void;
    constexpr auto set_c(UIntType c) noexcept -> void;
    constexpr auto set_m(UIntType m) noexcept -> void;
    constexpr auto a() const noexcept -> UIntType;
    constexpr auto c() const noexcept -> UIntType;
    constexpr auto m() const noexcept -> UIntType;
    constexpr auto operator()(result_type x) const noexcept -> result_type;
    // Hidden friend
    friend constexpr auto operator+(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform;
    constexpr auto operator+=(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&;
    // Hidden friend
    friend constexpr auto operator-(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform;
    constexpr auto operator-=(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&;
    // Hidden friend
    // Returns h such that h(x) = lhs(rhs(x)).
    friend constexpr auto compose(LCGAffineTransform lhs, const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform;
    constexpr auto compose_assign(const LCGAffineTransform& rhs) noexcept -> LCGAffineTransform&;
    constexpr auto identity() const noexcept -> LCGAffineTransform;
    // Returns h such that h(x) = f(f(f(...f(x)...))) where f is `*this`, and there are `n` f's.
    // Time complexity: O(log(n)).
    constexpr auto powered(unsigned long long n) const noexcept -> LCGAffineTransform;
    constexpr auto min() const noexcept -> result_type;
    constexpr auto max() const noexcept -> result_type;
    // Hidden friend
    // `= default;`
    friend constexpr auto operator==(const LCGAffineTransform& lhs, const LCGAffineTransform& rhs) noexcept -> bool;
};

template <std::unsigned_integral UIntType>
class LCGEngine {
public:
    using result_type = UIntType;
    using affine_type = LCGAffineTransform<UIntType>;
    static constexpr UIntType default_seed { 1U };
    explicit constexpr LCGEngine(affine_type affine, result_type state = default_seed) noexcept;
    explicit constexpr LCGEngine(UIntType a, UIntType c, UIntType m = 0, result_type state = default_seed) noexcept;
    template <typename StdEngine>
    static auto from_std(StdEngine engine) noexcept(false) -> LCGEngine
        requires detail::std_lcg_of<StdEngine, UIntType>;
    constexpr auto operator()() noexcept -> result_type;
    // Time complexity: O(log(n)).
    constexpr auto value_after_n_steps(unsigned long long steps) const noexcept -> result_type;
    // Time complexity: O(log(n)).
    constexpr auto discard(unsigned long long n) noexcept -> void;
    constexpr auto a() const noexcept -> UIntType;
    constexpr auto c() const noexcept -> UIntType;
    constexpr auto m() const noexcept -> UIntType;
    constexpr auto affine() const noexcept -> affine_type;
    constexpr auto state() const noexcept -> result_type;
    constexpr auto set_a(UIntType new_a) noexcept -> void;
    constexpr auto set_c(UIntType new_c) noexcept -> void;
    constexpr auto set_m(UIntType new_m) noexcept -> void;
    constexpr auto set_affine(const affine_type& new_affine) noexcept -> void;
    constexpr auto set_state(result_type new_seed) noexcept -> void;
    // Hidden friend
    // `= default;`
    friend constexpr auto operator==(const LCGEngine& lhs, const LCGEngine& rhs) noexcept -> bool;
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
```
