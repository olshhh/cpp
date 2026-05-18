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
    const double absolute_value = std::abs(value);

    return absolute_value <= epsilon;
}

bool almost_equal(double left, double right)
{
    const double epsilon = 1e-9;
    const double difference = std::abs(left - right);

    return difference <= epsilon;
}

SolveResult solve(double a, double b, double c)
{
    const double zero = 0.0;
    const double discriminant_multiplier = 4.0;
    const double denominator_multiplier = 2.0;
    const double discriminant = b * b - discriminant_multiplier * a * c;
    const double square_root = discriminant > zero ? std::sqrt(discriminant) : zero;
    const double denominator = denominator_multiplier * a;
    const double linear_root = is_zero(b) ? zero : -c / b;
    const double single_root = is_zero(denominator) ? zero : -b / denominator;
    double first_root = is_zero(denominator) ? zero : (-b - square_root) / denominator;
    double second_root = is_zero(denominator) ? zero : (-b + square_root) / denominator;

    if (is_zero(a)) {
        if (is_zero(b)) {
            if (is_zero(c)) {
                return Roots{InfiniteRoots{}};
            }

            return std::nullopt;
        }

        return Roots{linear_root};
    }

    if (is_zero(discriminant)) {
        return Roots{single_root};
    }

    if (discriminant < zero) {
        return std::nullopt;
    }

    if (second_root < first_root) {
        std::swap(first_root, second_root);
    }

    return Roots{TwoRoots{first_root, second_root}};
}

class Visitor
{
public:
    void operator()(OneRoot root) const
    {
        std::cout << "One root: " << root << '\n';
    }

    void operator()(TwoRoots const & roots) const
    {
        std::cout << "Two roots: " << roots.first << ' ' << roots.second << '\n';
    }

    void operator()(InfiniteRoots const &) const
    {
        std::cout << "Infinitely many roots\n";
    }
};

void print_result(SolveResult const & result)
{
    if (!result.has_value()) {
        std::cout << "No real roots\n";
        return;
    }

    std::visit(Visitor{}, *result);
}

void test_solve()
{
    {
        const SolveResult result = solve(1.0, 0.0, -1.0);
        const TwoRoots roots = result.has_value()
            && std::holds_alternative<TwoRoots>(*result)
            ? std::get<TwoRoots>(*result)
            : TwoRoots{0.0, 0.0};

        assert(result.has_value());
        assert(std::holds_alternative<TwoRoots>(*result));
        assert(almost_equal(roots.first, -1.0));
        assert(almost_equal(roots.second, 1.0));
    }

    {
        const SolveResult result = solve(2.0, -7.0, 3.0);
        const TwoRoots roots = result.has_value()
            && std::holds_alternative<TwoRoots>(*result)
            ? std::get<TwoRoots>(*result)
            : TwoRoots{0.0, 0.0};

        assert(result.has_value());
        assert(std::holds_alternative<TwoRoots>(*result));
        assert(almost_equal(roots.first, 0.5));
        assert(almost_equal(roots.second, 3.0));
    }

    {
        const SolveResult result = solve(1.0, 2.0, 1.0);
        const OneRoot root = result.has_value()
            && std::holds_alternative<OneRoot>(*result)
            ? std::get<OneRoot>(*result)
            : 0.0;

        assert(result.has_value());
        assert(std::holds_alternative<OneRoot>(*result));
        assert(almost_equal(root, -1.0));
    }

    {
        const SolveResult result = solve(1.0, 0.0, 1.0);

        assert(!result.has_value());
    }

    {
        const SolveResult result = solve(0.0, 2.0, 4.0);
        const OneRoot root = result.has_value()
            && std::holds_alternative<OneRoot>(*result)
            ? std::get<OneRoot>(*result)
            : 0.0;

        assert(result.has_value());
        assert(std::holds_alternative<OneRoot>(*result));
        assert(almost_equal(root, -2.0));
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