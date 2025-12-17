module rational;

import <cassert>;
import <cstdint>;
import <ios>;
import <istream>;
import <numeric>;
import <ostream>;

namespace edu
{
    Rational::Rational(int num, int den)
        : m_num(num), m_den(den)
    {
        reduce();
    }

    void Rational::reduce()
    {
        assert(m_den != 0 && "Denominator must not be zero.");

        if (m_den < 0)
        {
            m_den = -m_den;
            m_num = -m_num;
        }

        int g = 0;
        g = std::gcd(m_num, m_den);
        m_num /= g;
        m_den /= g;
    }

    Rational& Rational::operator+=(Rational const& other)
    {
        int l = 0;
        l = std::lcm(m_den, other.m_den);

        m_num = m_num * (l / m_den) + other.m_num * (l / other.m_den);
        m_den = l;

        reduce();
        return *this;
    }

    Rational& Rational::operator-=(Rational const& other)
    {
        Rational tmp(0, 1);
        tmp = Rational(-other.m_num, other.m_den);
        return (*this += tmp);
    }

    Rational& Rational::operator*=(Rational const& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& Rational::operator/=(Rational const& other)
    {
        assert(other.m_num != 0 && "Division by zero rational.");

        Rational tmp(0, 1);
        tmp = Rational(other.m_den, other.m_num);
        return (*this *= tmp);
    }

    Rational Rational::operator++(int)
    {
        Rational copy(*this);
        *this += Rational(1, 1);
        return copy;
    }

    Rational Rational::operator--(int)
    {
        Rational copy(*this);
        *this -= Rational(1, 1);
        return copy;
    }

    Rational& Rational::operator++()
    {
        *this += Rational(1, 1);
        return *this;
    }

    Rational& Rational::operator--()
    {
        *this -= Rational(1, 1);
        return *this;
    }

    std::strong_ordering operator<=>(Rational const& lhs, Rational const& rhs) noexcept
    {
        std::int64_t left_value = 0;
        std::int64_t right_value = 0;

        left_value = static_cast<std::int64_t>(lhs.m_num) * static_cast<std::int64_t>(rhs.m_den);
        right_value = static_cast<std::int64_t>(rhs.m_num) * static_cast<std::int64_t>(lhs.m_den);

        if (left_value < right_value)
        {
            return std::strong_ordering::less;
        }
        if (left_value > right_value)
        {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equivalent;
    }

    std::istream& operator>>(std::istream& stream, Rational& r)
    {
        int num = 0;
        int den = 1;
        char slash = '\0';

        if ((stream >> num >> slash >> den) && (slash == '/'))
        {
            if (den == 0)
            {
                stream.setstate(std::ios::failbit);
                return stream;
            }
            r = Rational(num, den);
        }
        else
        {
            stream.setstate(std::ios::failbit);
        }

        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, Rational const& r)
    {
        stream << r.m_num << '/' << r.m_den;
        return stream;
    }
}
