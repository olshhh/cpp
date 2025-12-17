#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>

template <int N>
struct Fibonacci
{
    static_assert(N >= 1, "Fibonacci<N>: N must be >= 1.");

private:
    static constexpr long long kLeft = static_cast<long long>(Fibonacci<N - 1>::value);
    static constexpr long long kRight = static_cast<long long>(Fibonacci<N - 2>::value);
    static constexpr long long kSum = kLeft + kRight;

    static_assert(kSum <= static_cast<long long>(std::numeric_limits<int>::max()),
                  "Fibonacci<N>: int overflow during compile-time computation.");

public:
    static inline int const value = static_cast<int>(kSum);
};


template <>
struct Fibonacci<1>
{
    static inline int const value = 1;
};

template <>
struct Fibonacci<2>
{
    static inline int const value = 1;
};


template <int N>
inline constexpr int fibonacci_v = Fibonacci<N>::value;

int main()
{

    static_assert(fibonacci_v<1> == 1);
    static_assert(fibonacci_v<2> == 1);
    static_assert(fibonacci_v<3> == 2);
    static_assert(fibonacci_v<4> == 3);
    static_assert(fibonacci_v<5> == 5);

    static_assert(fibonacci_v<10> == 55);
    static_assert(fibonacci_v<20> == 6765);
    static_assert(fibonacci_v<46> == 1836311903);


    int n10 = 0;
    n10 = fibonacci_v<10>;
    std::cout << "Fibonacci(10) = " << n10 << '\n';

    assert(n10 == 55);

    return 0;
}
