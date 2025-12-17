#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <type_traits>

// Returns true if two doubles are close enough.
bool almost_equal(double a, double b, double epsilon)
{
    double diff = 0.0;
    diff = std::abs(a - b);
    return diff <= epsilon;
}

// Max of a non-empty pack of doubles (recursive instantiation).
template <typename... Ts>
double max_value(double first, Ts... rest)
{
    static_assert((std::is_same_v<Ts, double> && ...), "All arguments must be double.");

    if constexpr (sizeof...(rest) == 0)
    {
        return first;
    }
    else
    {
        double tail_max = 0.0;
        tail_max = max_value(rest...);
        return (first < tail_max) ? tail_max : first;
    }
}

// Min of a non-empty pack of doubles (recursive instantiation).
template <typename... Ts>
double min_value(double first, Ts... rest)
{
    static_assert((std::is_same_v<Ts, double> && ...), "All arguments must be double.");

    if constexpr (sizeof...(rest) == 0)
    {
        return first;
    }
    else
    {
        double tail_min = 0.0;
        tail_min = min_value(rest...);
        return (tail_min < first) ? tail_min : first;
    }
}

// Sum of a non-empty pack of doubles.
template <typename... Ts>
double sum(double first, Ts... rest)
{
    static_assert((std::is_same_v<Ts, double> && ...), "All arguments must be double.");
    return (first + ... + rest);
}

// Arithmetic mean of a non-empty pack of doubles 
template <typename... Ts>
double average(double first, Ts... rest)
{
    static_assert((std::is_same_v<Ts, double> && ...), "All arguments must be double.");

    std::size_t count = 0U;
    double total = 0.0;

    count = sizeof...(rest) + 1U;
    total = sum(first, rest...);

    return total / static_cast<double>(count);
}

int main()
{
    constexpr double kEpsilon = 1e-12;

    // Tests: max/min ///recursive.
    {
        constexpr double kA = 1.0;
        constexpr double kB = 2.0;
        constexpr double kC = -3.0;
        constexpr double kD = 2.0;

        assert(almost_equal(max_value(kA, kB, kC, kD), 2.0, kEpsilon));
        assert(almost_equal(min_value(kA, kB, kC, kD), -3.0, kEpsilon));
    }

    // Tests: sum/average fold + sizeof
    {
        constexpr double kX = 1.5;
        constexpr double kY = 2.0;
        constexpr double kZ = 3.5;

        assert(almost_equal(sum(kX, kY, kZ), 7.0, kEpsilon));
        assert(almost_equal(average(kX, kY, kZ), (7.0 / 3.0), kEpsilon));
    }

    // Tests: single element pack.
    {
        constexpr double kOnly = -4.25;
        assert(almost_equal(max_value(kOnly), kOnly, kEpsilon));
        assert(almost_equal(min_value(kOnly), kOnly, kEpsilon));
        assert(almost_equal(sum(kOnly), kOnly, kEpsilon));
        assert(almost_equal(average(kOnly), kOnly, kEpsilon));
    }

    // example
    {
        constexpr double kP = 3.5;
        constexpr double kQ = -2.0;
        constexpr double kR = 10.0;
        constexpr double kS = 4.0;

        std::cout << "Demo values: " << kP << ' ' << kQ << ' ' << kR << ' ' << kS << '\n';
        std::cout << "Max: " << max_value(kP, kQ, kR, kS) << '\n';
        std::cout << "Min: " << min_value(kP, kQ, kR, kS) << '\n';
        std::cout << "Sum: " << sum(kP, kQ, kR, kS) << '\n';
        std::cout << "Average: " << average(kP, kQ, kR, kS) << '\n';
    }

    return 0;
}
