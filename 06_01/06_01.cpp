#include "Rational.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

int main()
{
    constexpr double kEps = 1e-9;
    const char* kPromptA = "A = ";
    const char* kPromptB = "B = ";

    auto almost_equal = [](double x, double y, double eps) -> bool
    {
        double diff = 0.0;
        diff = std::fabs(x - y);
        return diff < eps;
    };

    bool ok = true;

    Rational x(1, 1);
    Rational y(2, 1);

    ok = ok && almost_equal(static_cast<double>(x), 1.0, kEps);

    ok = ok && ((x += y) == Rational(3, 1));
    ok = ok && ((x -= y) == Rational(1, 1));
    ok = ok && ((x *= y) == Rational(2, 1));
    ok = ok && ((x /= y) == Rational(1, 1));

    ok = ok && ((x++) == Rational(1, 1));
    ok = ok && ((x--) == Rational(2, 1));
    ok = ok && ((++y) == Rational(3, 1));
    ok = ok && ((--y) == Rational(2, 1));

    ok = ok && ((x + y) == Rational(3, 1));
    ok = ok && ((x - y) == Rational(-1, 1));
    ok = ok && ((x * y) == Rational(2, 1));
    ok = ok && ((x / y) == Rational(1, 2));

    ok = ok && (x < y);
    ok = ok && !(x > y);
    ok = ok && (x <= y);
    ok = ok && !(x >= y);
    ok = ok && !(x == y);
    ok = ok && (x != y);

    std::cout << (ok ? "Self-check: OK\n" : "Self-check: FAILED\n");
    assert(ok);

    Rational a(0, 1);
    Rational b(0, 1);

    std::cout << "\nEnter two numbers in form n/d:\n";
    std::cout << kPromptA;
    std::cin >> a;
    std::cout << kPromptB;
    std::cin >> b;

    if (!std::cin)
    {
        std::cout << "Input error\n";
        return 0;
    }

    std::cout << "\nYou entered:\n";
    std::cout << "A = " << a << "  (" << static_cast<double>(a) << ")\n";
    std::cout << "B = " << b << "  (" << static_cast<double>(b) << ")\n";

    std::cout << "\nComparisons:\n";
    std::cout << "A == B : " << (a == b ? "true" : "false") << '\n';
    std::cout << "A <  B : " << (a <  b ? "true" : "false") << '\n';
    std::cout << "A <= B : " << (a <= b ? "true" : "false") << '\n';
    std::cout << "A >  B : " << (a >  b ? "true" : "false") << '\n';
    std::cout << "A >= B : " << (a >= b ? "true" : "false") << '\n';

    std::cout << "\nArithmetic:\n";
    std::cout << "A + B = " << (a + b) << '\n';
    std::cout << "A - B = " << (a - b) << '\n';
    std::cout << "A * B = " << (a * b) << '\n';
    std::cout << "A / B = " << (a / b) << '\n';

    return 0;
}
