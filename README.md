# lcg-predict
C++20 Header-only library: A "fast jump" algorithm for LCG

## Usage

Library: [`src/include/lcg_predict.hpp`](./src/include/lcg_predict.hpp).

Test: [`src/test/lcg_predict_test.cpp`](./src/test/lcg_predict_test.cpp).

To use: Include `lcg_predict.hpp`.

To test: Compile `lcg_predict_test.cpp`. If it compiles, it passes the test. There is no need to run the generated executable.

## Interface

Given `T` as an unsigned integer type, the library exposes:
- Class `LCGAffineTransform<T>`: combines three `a`, `c` and `m` of type `T`, describing an affine transformation $x \mapsto (ax + c) \bmod m$.
- Class `LCGEngine<T>`: combines an `LCGAffineTransform<T>` and an internal `state` of type `T`.
- Some instances of `LCGEngine<T>`, corresponding to some widely used LCGs.

Overview:

```C++
namespace ls_hower::lcg_predict {

template <std::unsigned_integral UIntType>
class LCGAffineTransform {
public:
    using result_type = UIntType;
    explicit constexpr LCGAffineTransform(UIntType a, UIntType c, UIntType m = 0) noexcept;
    template <typename StdEngine>
    static constexpr auto from_std() noexcept -> LCGAffineTransform
        requires detail::std_lcg_of<std::remove_cvref_t<StdEngine>, UIntType>
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
        requires detail::std_lcg_of<std::remove_cvref_t<StdEngine>, UIntType>
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

## Test

The test verifies that the pesudo-random number sequences gotten from:

- simulating (calling `operator()`)
- predicting (calling `value_after_n_steps(i)`)
- other source (OEIS)

are the same, for initial 10 terms.

It also checks that sequences generated by simulating and predicting are the same, for initial 1000 terms.

These are done for all pre-defined engines.

It also checks:

- `minstd_rand0_engine.value_after_n_steps(10000) == 1043618065`
- `minstd_rand_engine.value_after_n_steps(10000) == 399268537`

corresponding to the required behavior of `std::minstd_rand0` and `std::minstd_rand` in C++ standard.

The whole test is evaluated at compile-time, checked by `static_assert`. So it passes the test if the code compiles.

## Mathematical Principle

### Description

So here is a classic C implementation of `rand()` function.

```C
unsigned long int next = 1;

/* rand:  return pseudo-random integer on 0..32767 */
int rand(void)
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

/* srand:  set seed for rand() */
void srand(unsigned int seed)
{
    next = seed;
}
```

This implementation is taken from *The C Programming Language* (TCPL), 2nd edition, by Kernighan and Ritchie, as an example of a portable implementation of `rand()` and `srand()`. Variants of this code also appear in the C standard.

We can observe that on each call to `rand()`, the global variable `next` is updated by multiplying it by `1103515245` and adding `12345`. The function then returns bits 16 through 31 of `next`; this output operation does not affect the value of `next` itself. The function `srand()` simply assigns a new initial value to `next`, which can be regarded as resetting the seed.

Let us define a sequence ${s_n} , (n \ge 0)$ by

$$
\begin{aligned}
s_0 &= s, \\
s_{n+1} &= (1103515245 , s_n + 12345) \bmod 2^{31}.
\end{aligned}
$$

Here, $s$ is the initial seed value, which is $1$ by default and can be changed via `srand()`. This sequence exactly describes the evolution of the variable `next`, restricted to its lower 31 bits. Any higher bits are irrelevant.

As mentioned earlier, `rand()` does not return `next` directly. Instead, the value returned by the $n$-th call to `rand()` is

$$
\left\lfloor \frac{s_n}{65536} \right\rfloor \bmod 32768 \qquad (n \ge 1).
$$

This mapping is straightforward to compute, so in the following discussion we focus solely on the sequence ${s_n}$.

We now generalize this construction:

$$
\begin{aligned}
s_0 &= s, \\
s_{n+1} &= f(s_n).
\end{aligned}
$$

where

$$
f(x) = (a x + c) \bmod m, \qquad 0 \le a, c, s < m.
$$

In the previous example, we have $a = 1103515245$, $c = 12345$, and $m = 2^{31}$. Our goal is to compute $f^{(n)}(s)$, where $f^{(n)}$ denotes the $n$-fold application of $f$. Any further mapping applied to the result is not relevant here.

---

### Method: Fast Geometric Sum

By expanding the recurrence, we observe that

$$
\begin{aligned}
f^{(0)}(s) &= s \bmod m, \\
f^{(1)}(s) &= (a s + c) \bmod m, \\
f^{(2)}(s) &= (a^2 s + a c + c) \bmod m, \\
f^{(3)}(s) &= (a^3 s + a^2 c + a c + c) \bmod m, \\
& \vdots
\end{aligned}
$$

In general,

$$
\begin{aligned}
f^{(n)}(s)
&= (a^n s + a^{n-1} c + a^{n-2} c + \cdots + a c + c) \bmod m \\
&= (a^n s + c (a^{n-1} + a^{n-2} + \cdots + a + 1)) \bmod m \\
&= \left(a^n s + c \sum_{i=0}^{n-1} a^i \right) \bmod m.
\end{aligned}
$$

This formula can be proved by induction.

Using exponentiation by squaring, $a^n \bmod m$ can be computed in $\Theta(\log n)$ time. Therefore, the remaining task is to compute the sum

$$
\sigma = \sum_{i=0}^{n-1} a^i \bmod m.
$$

---

#### Method 1: Geometric Series Formula

If the multiplicative inverse $(a - 1)^{-1} \bmod m$ exists, then

$$
\sigma = (a^n - 1)(a - 1)^{-1} \bmod m.
$$

The inverse $(a - 1)^{-1} \bmod m$ can be computed using the extended Euclidean algorithm in $\Theta(\log m)$ time. Once $a$ and $m$ are fixed, it can also be precomputed and cached.

---

#### Method 2: Divide and Conquer

The inverse-based method does not apply when $a - 1$ and $m$ are not coprime. In that case, a divide-and-conquer algorithm can still compute $\sigma$ in $\Theta(\log n)$ time.

The following Python code implements this idea:

```Python
def fast_sum_and_pow(x: int, y: int, m: int) -> int:
    """
    Returns (init, last) where:
        init = (1 + x + x**2 + ... + x**(y-1)) % m
        last = (x**y) % m
    """
    if y == 0:
        return 0 % m, 1 % m
    elif y % 2 == 1:
        init, last = fast_sum_and_pow(x, y - 1, m)
        return (1 + init * x) % m, (last * x) % m
    else:
        init, last = fast_sum_and_pow(x, y // 2, m)
        return (init + init * last) % m, (last * last) % m


def fast_sum(x: int, y: int, m: int) -> int:
    """Returns (1 + x + x**2 + ... + x**(y-1)) % m."""
    return fast_sum_and_pow(x, y, m)[0]
```

Calling `fast_sum(a, n, m)` produces the desired value of $\sigma$.

---

### Method: Fast Affine Transformation Composition

There is an even simpler approach: compute $f^{(n)}$ directly and then apply it to $s$.

Consider two affine transformations:

$$
f(x) = (a x + b) \bmod m, \\
g(x) = (c x + d) \bmod m.
$$

Let $h(x) = f(g(x))$. Then

$$
h(x) = ((a c) x + (a d + b)) \bmod m.
$$

Thus, $h$ is again an affine transformation, and we write $h = f \circ g$.

An affine transformation can therefore be represented by a pair of integers $(a, c)$, and composition can be performed in $\Theta(1)$ time.

It is straightforward to see that

$$
f^{(n)} =
\begin{cases}
\mathrm{id}, & n = 0, \\
f \circ f^{(n-1)}, & n > 0 \text{ and } n \text{ is odd}, \\
f^{(n/2)} \circ f^{(n/2)}, & n > 0 \text{ and } n \text{ is even}.
\end{cases}
$$

where $\mathrm{id}$ denotes the identity transformation, $\mathrm{id}(x) = x$. There is no need to explicitly write $\bmod m$ here, since the initial seed already lies in the range $[0, m)$.

This recurrence immediately yields an $\Theta(\log n)$ algorithm for computing $f^{(n)}$, analogous to fast exponentiation. In fact, fast exponentiation is just a special case of this more general idea: the same algorithm applies to any associative operation, including function composition. The library implements this generalized algorithm.

All three methods discussed above can be applied to generalized LCGs. For example, those where $a$ is a square matrix and both $c$ and the internal state are vectors.

This library, however, uses only the fast affine transformation composition method to implement fast jumping and to predict the $n$-th pseudo-random value.
