#include <cassert>
#include <exception>
#include <iostream>
#include <numeric>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

template <typename T>
struct addable
{
    friend T operator+(T left, const T &right)
    {
        left += right;
        return left;
    }
};

template <typename T>
struct subtractable
{
    friend T operator-(T left, const T &right)
    {
        left -= right;
        return left;
    }
};

template <typename T>
struct multipliable
{
    friend T operator*(T left, const T &right)
    {
        left *= right;
        return left;
    }
};

template <typename T>
struct dividable
{
    friend T operator/(T left, const T &right)
    {
        left /= right;
        return left;
    }
};

template <typename T>
struct incrementable
{
    friend T &operator++(T &value)
    {
        value += T(1, 1);
        return value;
    }

    friend T operator++(T &value, int)
    {
        T old = value;
        ++value;
        return old;
    }
};

template <typename T>
struct decrementable
{
    friend T &operator--(T &value)
    {
        value -= T(1, 1);
        return value;
    }

    friend T operator--(T &value, int)
    {
        T old = value;
        --value;
        return old;
    }
};

class Exception : public std::exception
{
public:
    explicit Exception(const char *message) noexcept : message_(message) {}

    const char *what() const noexcept override
    {
        return message_;
    }

private:
    const char *message_ = "Exception";
};

class Rational : public addable<Rational>,
                 public subtractable<Rational>,
                 public multipliable<Rational>,
                 public dividable<Rational>,
                 public incrementable<Rational>,
                 public decrementable<Rational>
{
public:
    Rational(int numerator = 0, int denominator = 1)
        : numerator_(numerator), denominator_(denominator)
    {
        if (denominator_ == 0) {
            throw Exception("Rational: zero denominator");
        }
        normalize();
    }

    int numerator() const noexcept
    {
        return numerator_;
    }

    int denominator() const noexcept
    {
        return denominator_;
    }

    Rational &operator+=(const Rational &other)
    {
        numerator_ = numerator_ * other.denominator_ + other.numerator_ * denominator_;
        denominator_ *= other.denominator_;
        normalize();
        return *this;
    }

    Rational &operator-=(const Rational &other)
    {
        numerator_ = numerator_ * other.denominator_ - other.numerator_ * denominator_;
        denominator_ *= other.denominator_;
        normalize();
        return *this;
    }

    Rational &operator*=(const Rational &other)
    {
        numerator_ *= other.numerator_;
        denominator_ *= other.denominator_;
        if (denominator_ == 0) {
            throw Exception("Rational: zero denominator after multiplication");
        }
        normalize();
        return *this;
    }

    Rational &operator/=(const Rational &other)
    {
        if (other.numerator_ == 0) {
            throw Exception("Rational: division by zero");
        }
        numerator_ *= other.denominator_;
        denominator_ *= other.numerator_;
        if (denominator_ == 0) {
            throw Exception("Rational: zero denominator after division");
        }
        normalize();
        return *this;
    }

    bool operator==(const Rational &other) const noexcept
    {
        return numerator_ == other.numerator_ && denominator_ == other.denominator_;
    }

private:
    void normalize() noexcept
    {
        int divisor = 0;

        if (denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }

        divisor = std::gcd(numerator_, denominator_);
        if (divisor != 0) {
            numerator_ /= divisor;
            denominator_ /= divisor;
        }
    }

private:
    int numerator_ = 0;
    int denominator_ = 1;
};

void test_rational()
{
    Rational a(1, 2);
    Rational b(1, 3);
    Rational c;
    Rational d(5, 7);
    Rational e(5, 7);
    bool thrown = false;

    c = a + b;
    assert(c == Rational(5, 6));

    c = a - b;
    assert(c == Rational(1, 6));

    c = a * b;
    assert(c == Rational(1, 6));

    c = a / b;
    assert(c == Rational(3, 2));

    ++d;
    assert(d == Rational(12, 7));

    e--;
    assert(e == Rational(-2, 7));

    try {
        Rational invalid(1, 0);
    } catch (const Exception &exception) {
        thrown = true;
        assert(exception.what() != nullptr);
    }
    assert(thrown);
}

void demonstrate_bad_alloc()
{
    try {
        throw std::bad_alloc();
    } catch (const std::bad_alloc &exception) {
        std::cerr << "bad_alloc: " << exception.what() << '\n';
        std::cerr << "Reason: memory allocation failed.\n";
    }
}

void demonstrate_bad_variant_access()
{
    try {
        std::variant<int, double> value = 3.14;
        (void)std::get<int>(value);
    } catch (const std::bad_variant_access &exception) {
        std::cerr << "bad_variant_access: " << exception.what() << '\n';
        std::cerr << "Reason: requested variant alternative is not the stored one.\n";
    }
}

void demonstrate_bad_optional_access()
{
    try {
        std::optional<int> value = std::nullopt;
        (void)value.value();
    } catch (const std::bad_optional_access &exception) {
        std::cerr << "bad_optional_access: " << exception.what() << '\n';
        std::cerr << "Reason: attempted to read value from empty optional.\n";
    }
}

void demonstrate_length_error()
{
    try {
        std::vector<int> data;
        data.reserve(data.max_size() + 1U);
    } catch (const std::length_error &exception) {
        std::cerr << "length_error: " << exception.what() << '\n';
        std::cerr << "Reason: requested vector capacity exceeds max_size().\n";
    }
}

void demonstrate_out_of_range()
{
    try {
        std::vector<int> data = {1, 2, 3};
        (void)data.at(3);
    } catch (const std::out_of_range &exception) {
        std::cerr << "out_of_range: " << exception.what() << '\n';
        std::cerr << "Reason: vector::at index is outside valid range.\n";
    }
}

int main()
{
    test_rational();

    try {
        Rational invalid(1, 0);
        (void)invalid;
    } catch (const std::exception &exception) {
        std::cerr << "main: " << exception.what() << '\n';
    } catch (...) {
        std::cerr << "main: unknown exception\n";
    }

    try {
        demonstrate_bad_alloc();
        demonstrate_bad_variant_access();
        demonstrate_bad_optional_access();
        demonstrate_length_error();
        demonstrate_out_of_range();
    } catch (const std::exception &exception) {
        std::cerr << "main: unexpected std::exception: " << exception.what() << '\n';
    } catch (...) {
        std::cerr << "main: unexpected unknown exception\n";
    }

    std::cout << "Self-check: OK\n";
    return 0;
}