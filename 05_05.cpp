#include <cassert>
#include <compare>
#include <iostream>

template <typename T>
class addable
{
protected:
    addable() = default;

public:
    friend T operator+(T lhs, T const& rhs)
    {
        lhs += rhs;
        return lhs;
    }
};

template <typename T>
class subtractable
{
protected:
    subtractable() = default;

public:
    friend T operator-(T lhs, T const& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

template <typename T>
class multipliable
{
protected:
    multipliable() = default;

public:
    friend T operator*(T lhs, T const& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

template <typename T>
class dividable
{
protected:
    dividable() = default;

public:
    friend T operator/(T lhs, T const& rhs)
    {
        lhs /= rhs;
        return lhs;
    }
};

template <typename T>
class incrementable
{
protected:
    incrementable() = default;

public:
    friend T operator++(T& value, int)
    {
        T old(value);
        ++value;
        return old;
    }
};

template <typename T>
class decrementable
{
protected:
    decrementable() = default;

public:
    friend T operator--(T& value, int)
    {
        T old(value);
        --value;
        return old;
    }
};

template <typename T>
class Rational
    : private addable<Rational<T>>,
      private subtractable<Rational<T>>,
      private multipliable<Rational<T>>,
      private dividable<Rational<T>>,
      private incrementable<Rational<T>>,
      private decrementable<Rational<T>>
{
public:
    Rational(T numerator = T(0), T denominator = T(1))
        : m_num(numerator), m_den(denominator)
    {
        normalize();
    }

    explicit operator double() const
    {
        return static_cast<double>(m_num) / static_cast<double>(m_den);
    }

    Rational& operator+=(Rational const& other)
    {
        T const common = lcm(m_den, other.m_den);
        T const left = m_num * (common / m_den);
        T const right = other.m_num * (common / other.m_den);

        m_num = left + right;
        m_den = common;
        normalize();
        return *this;
    }

    Rational& operator-=(Rational const& other)
    {
        T const common = lcm(m_den, other.m_den);
        T const left = m_num * (common / m_den);
        T const right = other.m_num * (common / other.m_den);

        m_num = left - right;
        m_den = common;
        normalize();
        return *this;
    }

    Rational& operator*=(Rational const& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        normalize();
        return *this;
    }

    Rational& operator/=(Rational const& other)
    {
        assert(other.m_num != T(0));

        m_num *= other.m_den;
        m_den *= other.m_num;
        normalize();
        return *this;
    }

    Rational& operator++()
    {
        m_num += m_den;
        normalize();
        return *this;
    }

    Rational& operator--()
    {
        m_num -= m_den;
        normalize();
        return *this;
    }

    friend bool operator==(Rational const& lhs, Rational const& rhs)
    {
        return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
    }

    friend std::strong_ordering operator<=>(Rational const& lhs, Rational const& rhs)
    {
        T const left = lhs.m_num * rhs.m_den;
        T const right = rhs.m_num * lhs.m_den;

        if (left < right)
        {
            return std::strong_ordering::less;
        }
        if (left > right)
        {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    friend std::ostream& operator<<(std::ostream& out, Rational const& value)
    {
        out << value.m_num << '/' << value.m_den;
        return out;
    }

private:
    static T gcd(T a, T b)
    {
        T x = (a < T(0)) ? -a : a;
        T y = (b < T(0)) ? -b : b;

        while (y != T(0))
        {
            T const r = x % y;
            x = y;
            y = r;
        }

        return (x == T(0)) ? T(1) : x;
    }

    static T lcm(T a, T b)
    {
        T const x = (a < T(0)) ? -a : a;
        T const y = (b < T(0)) ? -b : b;

        if (x == T(0) || y == T(0))
        {
            return T(0);
        }

        return (x / gcd(x, y)) * y;
    }

    void normalize()
    {
        assert(m_den != T(0));

        if (m_den < T(0))
        {
            m_num = -m_num;
            m_den = -m_den;
        }

        {
            T const div = gcd(m_num, m_den);
            m_num /= div;
            m_den /= div;
        }
    }

private:
    T m_num;
    T m_den;
};

bool almost_equal(double x, double y, double epsilon = 1e-9)
{
    double const diff = x - y;
    double const abs_diff = (diff < 0.0) ? -diff : diff;
    return abs_diff < epsilon;
}

int main()
{
    {
        Rational<int> const a(2, 4);
        Rational<int> const b(-6, -8);
        Rational<int> const c(3, -9);

        assert(a == Rational<int>(1, 2));
        assert(b == Rational<int>(3, 4));
        assert(c == Rational<int>(-1, 3));
    }

    {
        Rational<int> x(1, 1);
        Rational<int> y(2, 1);

        assert(almost_equal(static_cast<double>(x), 1.0));

        assert((x += y) == Rational<int>(3, 1));
        assert((x -= y) == Rational<int>(1, 1));
        assert((x *= y) == Rational<int>(2, 1));
        assert((x /= y) == Rational<int>(1, 1));

        assert((x++) == Rational<int>(1, 1));
        assert((x--) == Rational<int>(2, 1));
        assert((++y) == Rational<int>(3, 1));
        assert((--y) == Rational<int>(2, 1));

        assert((x + y) == Rational<int>(3, 1));
        assert((x - y) == Rational<int>(-1, 1));
        assert((x * y) == Rational<int>(2, 1));
        assert((x / y) == Rational<int>(1, 2));

        assert(x < y);
        assert(!(x > y));
        assert(x <= y);
        assert(!(x >= y));
        assert(!(x == y));
        assert(x != y);
    }

    {
        Rational<long long> const z1(10000000000LL, 20000000000LL);
        Rational<long long> const z2(1LL, 2LL);

        assert(z1 == z2);
    }

    {
        Rational<int> const a(1, 2);
        Rational<int> const b(2, 3);

        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
        std::cout << "a + b = " << (a + b) << '\n';
        std::cout << "a - b = " << (a - b) << '\n';
        std::cout << "a * b = " << (a * b) << '\n';
        std::cout << "a / b = " << (a / b) << '\n';
    }

    return 0;
}