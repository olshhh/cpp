#include <cassert>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <limits>
#include <numeric>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

class Integer
{
public:

    using digit_t = long long int;

    Integer() : m_is_negative(false), m_digits(s_size, 0), m_size(1) {}

    Integer(digit_t digit) : Integer()
    {
        parse(std::to_string(digit));
    }

    Integer(std::string const & string) : Integer()
    {
        parse(string);
    }

    void swap(Integer & other)
    {
        std::swap(m_is_negative, other.m_is_negative);
        std::swap(m_digits, other.m_digits);
        std::swap(m_size, other.m_size);
    }

    auto & operator+=(Integer other)
    {
        if (m_is_negative == other.m_is_negative)
        {
            this->add(other);
        }
        else if (!m_is_negative && other.m_is_negative)
        {
            if (this->less(other))
            {
                *this = std::move(other.subtract(*this));
                m_is_negative = true;
            }
            else
            {
                this->subtract(other);
            }
        }
        else if (m_is_negative && !other.m_is_negative)
        {
            if (this->less(other))
            {
                *this = std::move(other.subtract(*this));
            }
            else
            {
                this->subtract(other);
                m_is_negative = true;
            }
        }

        normalize();

        return *this;
    }

    auto & operator-=(Integer other)
    {
        other.m_is_negative = !other.m_is_negative;

        return *this += other;
    }

    auto & operator*=(Integer other)
    {
        Integer x;

        x.m_is_negative = m_is_negative ^ other.m_is_negative;

        for (std::size_t i = 0; i < m_size; ++i)
        {
            digit_t remainder = 0;

            for (std::size_t j = 0; (j < other.m_size) || remainder; ++j)
            {
                x.m_digits[i + j] += m_digits[i] * other.m_digits[j] + remainder;

                remainder = x.m_digits[i + j] / s_base;

                x.m_digits[i + j] -= remainder * s_base;
            }
        }

        x.m_size = m_size + other.m_size;

        swap(x);

        reduce();
        normalize();

        return *this;
    }

    auto & operator/=(Integer other)
    {
        assert(!other.is_zero());

        Integer x;

        x.m_size = m_size;

        x.m_is_negative = m_is_negative ^ other.m_is_negative;

        other.m_is_negative = false;

        Integer current;

        for (int i = static_cast<int>(m_size) - 1; i >= 0; --i)
        {
            current *= s_base;

            current.m_digits.front() = m_digits[static_cast<std::size_t>(i)];

            digit_t left = 0;
            digit_t right = s_base - 1;
            digit_t digit = 0;

            while (left <= right)
            {
                auto middle = std::midpoint(left, right);

                if (other * middle <= current)
                {
                    left = middle + 1;
                    digit = middle;
                }
                else
                {
                    right = middle - 1;
                }
            }

            x.m_digits[static_cast<std::size_t>(i)] = digit;

            current -= other * digit;
        }

        swap(x);

        reduce();
        normalize();

        return *this;
    }

    auto & operator%=(Integer other)
    {
        assert(!other.is_zero());

        Integer quotient = *this;

        quotient /= other;

        *this -= quotient * other;

        normalize();

        return *this;
    }

    auto const operator++(int)
    {
        auto x = *this;
        *this += 1;
        return x;
    }

    auto const operator--(int)
    {
        auto x = *this;
        *this -= 1;
        return x;
    }

    auto & operator++()
    {
        *this += 1;
        return *this;
    }

    auto & operator--()
    {
        *this -= 1;
        return *this;
    }

    auto sign() const -> int
    {
        if (is_zero())
        {
            return 0;
        }

        return m_is_negative ? -1 : 1;
    }

    auto abs() const -> Integer
    {
        Integer x = *this;

        x.m_is_negative = false;

        return x;
    }

    friend auto operator+ (Integer lhs, Integer const & rhs)            { return lhs += rhs; }
    friend auto operator- (Integer lhs, Integer const & rhs)            { return lhs -= rhs; }
    friend auto operator* (Integer lhs, Integer const & rhs) -> Integer { return lhs *= rhs; }
    friend auto operator/ (Integer lhs, Integer const & rhs)            { return lhs /= rhs; }
    friend auto operator% (Integer lhs, Integer const & rhs)            { return lhs %= rhs; }

    friend auto operator< (Integer const & lhs, Integer const & rhs)
    {
        if (lhs.m_is_negative != rhs.m_is_negative)
        {
            return lhs.m_is_negative;
        }

        if (lhs.m_is_negative && rhs.m_is_negative)
        {
            return rhs.less(lhs);
        }
        else
        {
            return lhs.less(rhs);
        }
    }

    friend auto operator> (Integer const & lhs, Integer const & rhs)
    {
        return rhs < lhs;
    }

    friend auto operator<=(Integer const & lhs, Integer const & rhs) -> bool
    {
        return !(rhs < lhs);
    }

    friend auto operator>=(Integer const & lhs, Integer const & rhs)
    {
        return !(lhs < rhs);
    }

    friend auto operator==(Integer const & lhs, Integer const & rhs) -> bool
    {
        if (lhs.m_is_negative != rhs.m_is_negative || lhs.m_size != rhs.m_size)
        {
            return false;
        }

        for (std::size_t i = 0; i < lhs.m_size; ++i)
        {
            if (lhs.m_digits[i] != rhs.m_digits[i])
            {
                return false;
            }
        }

        return true;
    }

    friend auto operator!=(Integer const & lhs, Integer const & rhs) -> bool
    {
        return !(lhs == rhs);
    }

    friend auto & operator>>(std::istream & stream, Integer & integer)
    {
        std::string string;

        stream >> string;

        integer = Integer(string);

        return stream;
    }

    friend auto & operator<<(std::ostream & stream, Integer const & integer)
    {
        if (integer.m_is_negative)
        {
            stream << '-';
        }

        stream << integer.m_digits[integer.m_size - 1];

        for (int i = static_cast<int>(integer.m_size) - 2; i >= 0; --i)
        {
            stream << std::setw(Integer::s_step) << std::setfill('0')
                   << integer.m_digits[static_cast<std::size_t>(i)];
        }

        stream << std::setfill(' ');

        return stream;
    }

    friend auto sqrt(Integer const & x)
    {
        Integer y;

        y.m_size = (x.m_size + 1) / 2;

        for (int i = static_cast<int>(y.m_size) - 1; i >= 0; --i)
        {
            digit_t left = 0;
            digit_t right = Integer::s_base - 1;
            digit_t digit = 0;

            while (left <= right)
            {
                auto middle = y.m_digits[static_cast<std::size_t>(i)] = std::midpoint(left, right);

                if (y * y <= x)
                {
                    left = middle + 1;
                    digit = middle;
                }
                else
                {
                    right = middle - 1;
                }
            }

            y.m_digits[static_cast<std::size_t>(i)] = digit;
        }

        y.reduce();
        y.normalize();

        return y;
    }

    friend auto multiply(Integer const & x, Integer const & y) -> Integer
    {
        if (auto size = std::max(x.m_size, y.m_size); size > 1)
        {
            auto step = size / 2;

            Integer x1;
            Integer x2;

            x1.m_size = step;
            x2.m_size = size - step;

            for (std::size_t i = 0; i < step; ++i) { x1.m_digits[i] = x.m_digits[i]; }
            for (std::size_t i = step; i < size; ++i) { x2.m_digits[i - step] = x.m_digits[i]; }

            Integer y1;
            Integer y2;

            y1.m_size = step;
            y2.m_size = size - step;

            for (std::size_t i = 0; i < step; ++i) { y1.m_digits[i] = y.m_digits[i]; }
            for (std::size_t i = step; i < size; ++i) { y2.m_digits[i - step] = y.m_digits[i]; }

            auto a = multiply(x2, y2);
            auto b = multiply(x1, y1);
            auto c = multiply(x2 + x1, y2 + y1);

            Integer base = Integer::s_base;

            for (std::size_t i = 1; i < step; ++i)
            {
                base *= Integer::s_base;
            }

            auto z = a * base * base + (c - b - a) * base + b;

            z.m_is_negative = x.m_is_negative ^ y.m_is_negative;
            z.normalize();

            return z;
        }
        else
        {
            return x * y;
        }
    }

    friend auto pow(Integer base, unsigned int exponent) -> Integer
    {
        Integer result = 1;

        while (exponent > 0u)
        {
            if ((exponent % 2u) == 1u)
            {
                result *= base;
            }

            base *= base;
            exponent /= 2u;
        }

        return result;
    }

private:

    void parse(std::string const & string)
    {
        assert(!string.empty());

        m_is_negative = string.front() == '-';

        m_size = 0;

        for (auto i = std::ssize(string) - 1; i >= 0; i -= s_step)
        {
            auto begin = std::max<std::ptrdiff_t>(i - s_step + 1, 0);

            if (begin == 0 && !std::isdigit(static_cast<unsigned char>(string.front())))
            {
                ++begin;
            }

            auto digit = string.substr(static_cast<std::size_t>(begin),
                                       static_cast<std::size_t>(i - begin + 1));

            if (!digit.empty())
            {
                m_digits[m_size++] = std::stoll(digit);
            }
        }

        reduce();
        normalize();
    }

    void reduce()
    {
        while (m_size > 1 && !m_digits[m_size - 1])
        {
            --m_size;
        }
    }

    void normalize()
    {
        if (is_zero())
        {
            m_is_negative = false;
        }
    }

    auto add(Integer const & other) -> Integer &
    {
        m_size = std::max(m_size, other.m_size);

        for (std::size_t i = 0; i < m_size; ++i)
        {
            m_digits[i] += other.m_digits[i];

            if (m_digits[i] >= s_base)
            {
                m_digits[i] -= s_base;
                m_digits[i + 1]++;
            }
        }

        m_size += static_cast<std::size_t>(m_digits[m_size] != 0);

        return *this;
    }

    auto subtract(Integer const & other) -> Integer &
    {
        for (std::size_t i = 0; i < m_size; ++i)
        {
            m_digits[i] -= other.m_digits[i];

            if (m_digits[i] < 0)
            {
                m_digits[i] += s_base;
                m_digits[i + 1]--;
            }
        }

        reduce();

        return *this;
    }

    auto less(Integer const & other) const -> bool
    {
        if (m_size != other.m_size)
        {
            return m_size < other.m_size;
        }

        for (int i = static_cast<int>(m_size) - 1; i >= 0; --i)
        {
            if (m_digits[static_cast<std::size_t>(i)] != other.m_digits[static_cast<std::size_t>(i)])
            {
                return m_digits[static_cast<std::size_t>(i)] < other.m_digits[static_cast<std::size_t>(i)];
            }
        }

        return false;
    }

    auto is_zero() const -> bool
    {
        return m_size == 1 && m_digits[0] == 0;
    }

    bool m_is_negative = false;
    std::vector<digit_t> m_digits;
    std::size_t m_size = 0;

    static inline constexpr std::size_t s_size = 1000;
    static inline constexpr int s_step = std::numeric_limits<digit_t>::digits10 / 2;
    static inline constexpr digit_t s_base = 1000000000LL;
};

int main()
{
    Integer x = std::string(32, '1');
    Integer y = std::string(32, '2');

    assert((x += y) == "+33333333333333333333333333333333"s);
    assert((x -= y) == "+11111111111111111111111111111111"s);
    assert((x *= y) == "+246913580246913580246913580246908641975308641975308641975308642"s);
    assert((x /= y) == "+11111111111111111111111111111111"s);

    assert((x++) == "+11111111111111111111111111111111"s);
    assert((x--) == "+11111111111111111111111111111112"s);
    assert((++y) == "+22222222222222222222222222222223"s);
    assert((--y) == "+22222222222222222222222222222222"s);

    assert((x + y) == "+33333333333333333333333333333333"s);
    assert((x - y) == "-11111111111111111111111111111111"s);
    assert((x * y) == "+246913580246913580246913580246908641975308641975308641975308642"s);
    assert((x / y) == "+0"s);

    assert((x < y) == 1);
    assert((x > y) == 0);
    assert((x <= y) == 1);
    assert((x >= y) == 0);
    assert((x == y) == 0);
    assert((x != y) == 1);

    std::stringstream stream_1(std::string(32, '1'));
    std::stringstream stream_2;

    stream_1 >> x;
    stream_2 << x;

    assert(stream_2.str() == stream_1.str());

    assert(sqrt(multiply(x, x)) == x);

    Integer a = "+100"s;
    Integer b = "+30"s;

    assert((a % b) == "+10"s);
    a %= b;
    assert(a == "+10"s);

    Integer c = "-100"s;
    assert((c % b) == "-10"s);

    Integer d = "+123"s;
    Integer e = "-123"s;
    Integer z = "+0"s;

    assert(d.sign() == 1);
    assert(e.sign() == -1);
    assert(z.sign() == 0);

    assert(d.abs() == "+123"s);
    assert(e.abs() == "+123"s);

    Integer base = "+2"s;

    assert(pow(base, 10u) == "+1024"s);
    assert(pow(base, 0u) == "+1"s);

    Integer negative_zero = "-0"s;

    assert(negative_zero == "+0"s);
    assert(negative_zero.sign() == 0);

    std::cout << "x = " << x << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "100 % 30 = " << (Integer("+100") % Integer("+30")) << '\n';
    std::cout << "-100 % 30 = " << (Integer("-100") % Integer("+30")) << '\n';
    std::cout << "pow(2, 10) = " << pow(Integer("+2"), 10u) << '\n';
    std::cout << "abs(-123) = " << Integer("-123").abs() << '\n';
    std::cout << "sign(+123) = " << Integer("+123").sign() << '\n';
    std::cout << "sign(-123) = " << Integer("-123").sign() << '\n';
    std::cout << "sign(0) = " << Integer("+0").sign() << '\n';
}