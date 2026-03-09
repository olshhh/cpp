#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using OneRoot = double;
using TwoRoots = std::pair<double, double>;
using InfiniteRoots = std::monostate;
using Roots = std::variant<OneRoot, TwoRoots, InfiniteRoots>;
using SolveResult = std::optional<Roots>;

bool is_zero(double value)
{
    const double epsilon = 1e-9;
    return std::abs(value) <= epsilon;
}

bool almost_equal(double left, double right)
{
    const double epsilon = 1e-9;
    return std::abs(left - right) <= epsilon;
}

SolveResult solve(double a, double b, double c)
{
    if (is_zero(a)) {
        if (is_zero(b)) {
            if (is_zero(c)) {
                return Roots{InfiniteRoots{}};
            }
            return std::nullopt;
        }

        const double x = -c / b;
        return Roots{x};
    }

    const double discriminant = b * b - 4.0 * a * c;

    if (is_zero(discriminant)) {
        const double x = -b / (2.0 * a);
        return Roots{x};
    }

    if (discriminant < 0.0) {
        return std::nullopt;
    }

    const double sqrt_discriminant = std::sqrt(discriminant);
    const double denominator = 2.0 * a;
    double x1 = (-b - sqrt_discriminant) / denominator;
    double x2 = (-b + sqrt_discriminant) / denominator;

    if (x2 < x1) {
        std::swap(x1, x2);
    }

    return Roots{TwoRoots{x1, x2}};
}

void print_result(const SolveResult &result)
{
    if (!result.has_value()) {
        std::cout << "No real roots\n";
        return;
    }

    if (std::holds_alternative<InfiniteRoots>(*result)) {
        std::cout << "Infinitely many roots\n";
        return;
    }

    if (const OneRoot *root = std::get_if<OneRoot>(&(*result))) {
        std::cout << "One root: " << *root << '\n';
        return;
    }

    if (const TwoRoots *roots = std::get_if<TwoRoots>(&(*result))) {
        std::cout << "Two roots: " << roots->first << ' ' << roots->second << '\n';
        return;
    }

    assert(false);
}

void test_solve()
{
    {
        const SolveResult result = solve(1.0, 0.0, -1.0);
        assert(result.has_value());
        assert(std::holds_alternative<TwoRoots>(*result));
        const TwoRoots roots = std::get<TwoRoots>(*result);
        assert(almost_equal(roots.first, -1.0));
        assert(almost_equal(roots.second, 1.0));
    }

    {
        const SolveResult result = solve(2.0, -7.0, 3.0);
        assert(result.has_value());
        assert(std::holds_alternative<TwoRoots>(*result));
        const TwoRoots roots = std::get<TwoRoots>(*result);
        assert(almost_equal(roots.first, 0.5));
        assert(almost_equal(roots.second, 3.0));
    }

    {
        const SolveResult result = solve(1.0, 2.0, 1.0);
        assert(result.has_value());
        assert(std::holds_alternative<OneRoot>(*result));
        assert(almost_equal(std::get<OneRoot>(*result), -1.0));
    }

    {
        const SolveResult result = solve(1.0, 0.0, 1.0);
        assert(!result.has_value());
    }

    {
        const SolveResult result = solve(0.0, 2.0, 4.0);
        assert(result.has_value());
        assert(std::holds_alternative<OneRoot>(*result));
        assert(almost_equal(std::get<OneRoot>(*result), -2.0));
    }

    {
        const SolveResult result = solve(0.0, 0.0, 0.0);
        assert(result.has_value());
        assert(std::holds_alternative<InfiniteRoots>(*result));
    }

    {
        const SolveResult result = solve(0.0, 0.0, 5.0);
        assert(!result.has_value());
    }
}

int main()
{
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    SolveResult result = std::nullopt;

    test_solve();
    std::cout << "Self-check: OK\n";

    std::cout << "Enter coefficients a, b, c: ";
    if (!(std::cin >> a >> b >> c)) {
        return 0;
    }

    result = solve(a, b, c);
    print_result(result);

    return 0;
}