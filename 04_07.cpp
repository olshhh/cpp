#include <cassert>
#include <cstddef>
#include <numeric>
#include <type_traits>

template <int N = 0, int D = 1>
struct Ratio
{
    static_assert(D != 0, "Ratio: denominator must be non-zero.");
    static constexpr int num = N;
    static constexpr int den = D;
};

template <int N, int D>
struct Normalize
{
    static_assert(D != 0, "Normalize: denominator must be non-zero.");

private:
    static constexpr int kOne = 1;
    static constexpr int sign = (D < 0) ? -kOne : kOne;
    static constexpr int n = N * sign;
    static constexpr int d = D * sign;
    static constexpr int g = std::gcd(n, d);

public:
    using type = Ratio<n / g, d / g>;
};

template <typename R1, typename R2>
struct Sum
{
private:
    static constexpr int raw_num = R1::num * R2::den + R2::num * R1::den;
    static constexpr int raw_den = R1::den * R2::den;

public:
    using type = typename Normalize<raw_num, raw_den>::type;
    static constexpr int num = type::num;
    static constexpr int den = type::den;
};

template <typename R1, typename R2>
using sum = typename Sum<R1, R2>::type;

template <typename R1, typename R2>
struct Mul
{
private:
    static constexpr int raw_num = R1::num * R2::num;
    static constexpr int raw_den = R1::den * R2::den;

public:
    using type = typename Normalize<raw_num, raw_den>::type;
    static constexpr int num = type::num;
    static constexpr int den = type::den;
};

template <typename R1, typename R2>
using mul = typename Mul<R1, R2>::type;

template <typename R>
using neg = Ratio<-R::num, R::den>;

template <typename R1, typename R2>
struct Sub
{
    using type = sum<R1, neg<R2>>;
    static constexpr int num = type::num;
    static constexpr int den = type::den;
};

template <typename R1, typename R2>
using sub = typename Sub<R1, R2>::type;

template <typename R>
using inv = Ratio<R::den, R::num>;

template <typename R1, typename R2>
struct Div
{
    static_assert(R2::num != 0, "Div: division by zero ratio.");
    using type = mul<R1, inv<R2>>;
    static constexpr int num = type::num;
    static constexpr int den = type::den;
};

template <typename R1, typename R2>
using div = typename Div<R1, R2>::type;

template <typename T, typename R = Ratio<1>>
struct Duration
{
    T x;

    constexpr Duration() : x() {}
    constexpr explicit Duration(T v) : x(v) {}
};

template <typename T, typename R>
constexpr auto operator-(Duration<T, R> const& v)
{
    using result_t = Duration<decltype(-v.x), R>;
    return result_t{-v.x};
}

template <typename T1, typename R1, typename T2, typename R2>
constexpr auto operator+(Duration<T1, R1> const& lhs, Duration<T2, R2> const& rhs)
{
    using ratio_t = Ratio<1, sum<R1, R2>::den>;

    auto x =
        lhs.x * ratio_t::den / R1::den * R1::num +
        rhs.x * ratio_t::den / R2::den * R2::num;

    return Duration<decltype(x), ratio_t>{x};
}

template <typename T1, typename R1, typename T2, typename R2>
constexpr auto operator-(Duration<T1, R1> const& lhs, Duration<T2, R2> const& rhs)
{
    return lhs + (-rhs);
}

int main()
{
    static_assert(sum<Ratio<1, 2>, Ratio<1, 3>>::num == 5);
    static_assert(sum<Ratio<1, 2>, Ratio<1, 3>>::den == 6);

    static_assert(mul<Ratio<2, 4>, Ratio<3, 9>>::num == 1);
    static_assert(mul<Ratio<2, 4>, Ratio<3, 9>>::den == 6);

    static_assert(sub<Ratio<1, 2>, Ratio<1, 3>>::num == 1);
    static_assert(sub<Ratio<1, 2>, Ratio<1, 3>>::den == 6);

    static_assert(div<Ratio<1, 2>, Ratio<3, 4>>::num == 2);
    static_assert(div<Ratio<1, 2>, Ratio<3, 4>>::den == 3);

    {
        constexpr int x = 1;
        constexpr int y = 2;

        constexpr Duration<int, Ratio<1, 2>> d1{x};
        constexpr Duration<int, Ratio<1, 3>> d2{y};

        constexpr auto d3 = d1 + d2;
        static_assert(std::is_same_v<std::remove_cv_t<decltype(d3)>, Duration<int, Ratio<1, 6>>>);
        static_assert(d3.x == 7);

        constexpr auto d4 = d2 - d1; 
        static_assert(std::is_same_v<std::remove_cv_t<decltype(d4)>, Duration<int, Ratio<1, 6>>>);
        static_assert(d4.x == 1);
    }

    return 0;
}
