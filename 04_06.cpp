#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numbers>

// Absolute value for long double 
constexpr long double abs_ld(long double x) noexcept
{
    long double r = 0.0L;
    r = (x < 0.0L) ? -x : x;
    return r;
}

// Absolute value for double
constexpr double abs_double(double x) noexcept
{
    double r = 0.0;
    r = (x < 0.0) ? -x : x;
    return r;
}

// arctan(x)       Taylor series: sum_{k>=0} (-1)^k * x^(2k+1)/(2k+1)
consteval long double arctan_series(long double x, long double eps) noexcept
{
    long double sum = 0.0L;
    long double x2 = 0.0L;
    long double x_pow = 0.0L;
    long double sign = 0.0L;
    long double denom = 0.0L;
    long double term = 0.0L;

    x2 = x * x;
    x_pow = x;
    sign = 1.0L;
    denom = 1.0L;

    term = sign * (x_pow / denom);
    sum = term;

    while (abs_ld(term) > eps)
    {
        sign = -sign;
        x_pow = x_pow * x2;
        denom = denom + 2.0L;

        term = sign * (x_pow / denom);
        sum = sum + term;
    }

    return sum;
}

// pi = 16 * arctan(1/5) - 4 * arctan(1/239)
consteval double compute_pi(double epsilon) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
                  "This implementation assumes IEEE-754 doubles.");

    constexpr long double kA = 16.0L;
    constexpr long double kB = 4.0L;
    constexpr long double kD1 = 5.0L;
    constexpr long double kD2 = 239.0L;

    long double eps = 0.0L;
    long double t1 = 0.0L;
    long double t2 = 0.0L;
    long double pi = 0.0L;

    if (!(epsilon > 0.0))
    {
        return std::numbers::pi_v<double>;
    }

    eps = static_cast<long double>(epsilon);

    t1 = arctan_series(1.0L / kD1, eps);
    t2 = arctan_series(1.0L / kD2, eps);

    pi = (kA * t1) - (kB * t2);
    return static_cast<double>(pi);
}

//e = sum_{k=0..inf} 1 / k!
consteval double compute_e(double epsilon) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
                  "This implementation assumes IEEE-754 doubles.");

    long double eps = 0.0L;
    long double sum = 0.0L;
    long double term = 0.0L;
    int k = 0;

    if (!(epsilon > 0.0))
    {
        return std::numbers::e_v<double>;
    }

    eps = static_cast<long double>(epsilon);

    sum = 0.0L;
    term = 1.0L;
    k = 0;

    while (term >= eps)
    {
        sum = sum + term;
        ++k;
        term = term / static_cast<long double>(k);
    }

    return static_cast<double>(sum);
}

int main()
{

    constexpr double kEps0 = 1e-3;
    constexpr double kEps1 = 1e-6;
    constexpr double kEps2 = 1e-9;
    constexpr double kEps3 = 1e-12;

    constexpr std::array<double, 4U> epsilons = {kEps0, kEps1, kEps2, kEps3};

    
    constexpr double pi0 = compute_pi(epsilons[0]);
    constexpr double pi1 = compute_pi(epsilons[1]);
    constexpr double pi2 = compute_pi(epsilons[2]);
    constexpr double pi3 = compute_pi(epsilons[3]);

    constexpr double e0 = compute_e(epsilons[0]);
    constexpr double e1 = compute_e(epsilons[1]);
    constexpr double e2 = compute_e(epsilons[2]);
    constexpr double e3 = compute_e(epsilons[3]);

    static_assert(abs_double(pi0 - std::numbers::pi_v<double>) < epsilons[0]);
    static_assert(abs_double(pi1 - std::numbers::pi_v<double>) < epsilons[1]);
    static_assert(abs_double(pi2 - std::numbers::pi_v<double>) < epsilons[2]);
    static_assert(abs_double(pi3 - std::numbers::pi_v<double>) < epsilons[3]);

    static_assert(abs_double(e0 - std::numbers::e_v<double>) < epsilons[0]);
    static_assert(abs_double(e1 - std::numbers::e_v<double>) < epsilons[1]);
    static_assert(abs_double(e2 - std::numbers::e_v<double>) < epsilons[2]);
    static_assert(abs_double(e3 - std::numbers::e_v<double>) < epsilons[3]);

    
    assert(abs_double(pi2 - std::numbers::pi_v<double>) < epsilons[2]);
    assert(abs_double(e2 - std::numbers::e_v<double>) < epsilons[2]);

    return 0;
}
