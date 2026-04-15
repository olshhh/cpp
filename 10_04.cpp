#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

#include <cassert>
#include <cstddef>
#include <iostream>

using Matrix = boost::numeric::ublas::bounded_matrix<unsigned long long int, 2, 2>;

Matrix make_identity_matrix()
{
    Matrix matrix = {};

    matrix(0, 0) = 1ULL;
    matrix(0, 1) = 0ULL;
    matrix(1, 0) = 0ULL;
    matrix(1, 1) = 1ULL;

    return matrix;
}

Matrix make_fibonacci_matrix()
{
    Matrix matrix = {};

    matrix(0, 0) = 1ULL;
    matrix(0, 1) = 1ULL;
    matrix(1, 0) = 1ULL;
    matrix(1, 1) = 0ULL;

    return matrix;
}

Matrix power(Matrix base, std::size_t exponent)
{
    Matrix result = make_identity_matrix();
    std::size_t current_exponent = exponent;

    while (current_exponent > 0U)
    {
        if ((current_exponent & 1U) != 0U)
        {
            result = boost::numeric::ublas::prod(result, base);
        }

        base = boost::numeric::ublas::prod(base, base);
        current_exponent /= 2U;
    }

    return result;
}

unsigned long long int fibonacci_matrix(const std::size_t n)
{
    const std::size_t max_n = 93U;
    Matrix transition = {};
    Matrix result = {};

    assert(n <= max_n);

    transition = make_fibonacci_matrix();
    result = power(transition, n);

    return result(0, 1);
}

unsigned long long int fibonacci_linear(const std::size_t n)
{
    unsigned long long int previous = 0ULL;
    unsigned long long int current = 1ULL;
    unsigned long long int next = 0ULL;
    std::size_t index = 0U;

    if (n == 0U)
    {
        return 0ULL;
    }

    for (index = 1U; index < n; ++index)
    {
        next = previous + current;
        previous = current;
        current = next;
    }

    return current;
}

void run_tests()
{
    std::size_t n = 0U;

    assert(fibonacci_matrix(0U) == 0ULL);
    assert(fibonacci_matrix(1U) == 1ULL);
    assert(fibonacci_matrix(2U) == 1ULL);
    assert(fibonacci_matrix(3U) == 2ULL);
    assert(fibonacci_matrix(10U) == 55ULL);
    assert(fibonacci_matrix(20U) == 6765ULL);
    assert(fibonacci_matrix(50U) == 12586269025ULL);
    assert(fibonacci_matrix(93U) == 12200160415121876738ULL);

    for (n = 0U; n <= 93U; ++n)
    {
        assert(fibonacci_matrix(n) == fibonacci_linear(n));
    }
}

int main()
{
    const std::size_t demo_limit = 10U;
    std::size_t n = 0U;

    run_tests();

    for (n = 0U; n <= demo_limit; ++n)
    {
        std::cout << "F(" << n << ") = " << fibonacci_matrix(n) << '\n';
    }

    std::cout << "F(50) = " << fibonacci_matrix(50U) << '\n';
    std::cout << "F(93) = " << fibonacci_matrix(93U) << '\n';
}