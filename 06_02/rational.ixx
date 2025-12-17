export module rational;

import <compare>;
import <iosfwd>;

export namespace edu
{
    class Rational
    {
    public:
        static constexpr int kDefaultNum = 0;
        static constexpr int kDefaultDen = 1;

        Rational(int num = kDefaultNum, int den = kDefaultDen);

        int numerator() const noexcept { return m_num; }
        int denominator() const noexcept { return m_den; }

        explicit operator double() const
        {
            double result = 0.0;
            result = static_cast<double>(m_num) / static_cast<double>(m_den);
            return result;
        }

        Rational& operator+=(Rational const& other);
        Rational& operator-=(Rational const& other);
        Rational& operator*=(Rational const& other);
        Rational& operator/=(Rational const& other);

        Rational operator++(int);
        Rational operator--(int);
        Rational& operator++();
        Rational& operator--();

        friend Rational operator+(Rational lhs, Rational const& rhs)
        {
            lhs += rhs;
            return lhs;
        }

        friend Rational operator-(Rational lhs, Rational const& rhs)
        {
            lhs -= rhs;
            return lhs;
        }

        friend Rational operator*(Rational lhs, Rational const& rhs)
        {
            lhs *= rhs;
            return lhs;
        }

        friend Rational operator/(Rational lhs, Rational const& rhs)
        {
            lhs /= rhs;
            return lhs;
        }

        friend bool operator==(Rational const& lhs, Rational const& rhs) noexcept
        {
            return (lhs.m_num == rhs.m_num) && (lhs.m_den == rhs.m_den);
        }

        friend std::strong_ordering operator<=>(Rational const& lhs, Rational const& rhs) noexcept;

        friend std::istream& operator>>(std::istream& stream, Rational& r);
        friend std::ostream& operator<<(std::ostream& stream, Rational const& r);

    private:
        void reduce();

        int m_num;
        int m_den;
    };
}

