#include <cassert>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

class Parser
{
public:
    explicit Parser(std::string_view text) : text_(text), position_(0U)
    {
    }

    double parse()
    {
        double value = 0.0;

        value = expression();
        skip_spaces();

        if (!is_end()) {
            throw std::runtime_error("Unexpected token");
        }

        return value;
    }

private:
    bool is_end() const
    {
        const bool result = position_ >= text_.size();

        return result;
    }

    static bool is_space(char symbol)
    {
        const bool result = symbol == ' ' || symbol == '\t' || symbol == '\n'
            || symbol == '\r' || symbol == '\f' || symbol == '\v';

        return result;
    }

    void skip_spaces()
    {
        while (!is_end() && is_space(text_[position_])) {
            ++position_;
        }
    }

    char peek()
    {
        const char end_symbol = '\0';
        char result = end_symbol;

        skip_spaces();

        if (!is_end()) {
            result = text_[position_];
        }

        return result;
    }

    bool match(char symbol)
    {
        bool result = false;

        skip_spaces();

        if (!is_end() && text_[position_] == symbol) {
            ++position_;
            result = true;
        }

        return result;
    }

    void expect(char symbol)
    {
        const bool matched = match(symbol);

        if (!matched) {
            throw std::runtime_error("Missing closing bracket");
        }
    }

    double expression()
    {
        double value = 0.0;
        char operation = '\0';

        value = term();
        operation = peek();

        while (operation == '+' || operation == '-') {
            match(operation);

            if (operation == '+') {
                value += term();
            } else {
                value -= term();
            }

            operation = peek();
        }

        return value;
    }

    double term()
    {
        double value = 0.0;
        char operation = '\0';
        double right = 0.0;

        value = power();
        operation = peek();

        while (operation == '*' || operation == '/' || operation == '%') {
            match(operation);
            right = power();

            if (operation == '*') {
                value *= right;
            } else if (operation == '/') {
                value /= right;
            } else {
                value = std::fmod(value, right);
            }

            operation = peek();
        }

        return value;
    }

    double power()
    {
        double value = 0.0;
        double exponent = 0.0;

        value = unary();

        if (match('^')) {
            exponent = power();
            value = std::pow(value, exponent);
        }

        return value;
    }

    double unary()
    {
        double value = 0.0;

        if (match('+')) {
            value = unary();
        } else if (match('-')) {
            value = -unary();
        } else {
            value = postfix();
        }

        return value;
    }

    double postfix()
    {
        double value = 0.0;

        value = primary();

        while (match('!')) {
            value = factorial(value);
        }

        return value;
    }

    double primary()
    {
        char symbol = '\0';
        char close = '\0';
        double value = 0.0;

        symbol = peek();

        if (symbol == '(' || symbol == '[' || symbol == '{') {
            close = closing_bracket(symbol);
            match(symbol);
            value = expression();
            expect(close);

            return value;
        }

        return number();
    }

    double number()
    {
        const char* begin = text_.data() + position_;
        const char* end = text_.data() + text_.size();
        double value = 0.0;
        std::from_chars_result result = {};

        skip_spaces();
        begin = text_.data() + position_;
        result = std::from_chars(begin, end, value);

        if (result.ec != std::errc() || result.ptr == begin) {
            throw std::runtime_error("Number expected");
        }

        position_ = static_cast<std::size_t>(result.ptr - text_.data());

        return value;
    }

    static char closing_bracket(char symbol)
    {
        char result = '\0';

        if (symbol == '(') {
            result = ')';
        } else if (symbol == '[') {
            result = ']';
        } else if (symbol == '{') {
            result = '}';
        } else {
            throw std::runtime_error("Opening bracket expected");
        }

        return result;
    }

    static double factorial(double value)
    {
        const double rounded = std::round(value);
        const double epsilon = 1e-9;
        const bool negative = value < 0.0;
        const bool integer = std::abs(value - rounded) <= epsilon;
        unsigned long long limit = 0U;
        unsigned long long index = 0U;
        double result = 1.0;

        if (negative || !integer) {
            throw std::runtime_error(
                "Factorial argument must be a non-negative integer");
        }

        limit = static_cast<unsigned long long>(rounded);

        for (index = 2U; index <= limit; ++index) {
            result *= static_cast<double>(index);
        }

        return result;
    }

    std::string_view text_;
    std::size_t position_;
};

struct TestCase
{
    std::string expression;
    double expected;
};

double calculate(std::string_view text)
{
    Parser parser(text);
    double value = 0.0;

    value = parser.parse();

    return value;
}

bool almost_equal(double left, double right)
{
    const double epsilon = 1e-9;
    const double difference = std::abs(left - right);

    return difference <= epsilon;
}

std::string trim_copy(std::string const& text)
{
    const std::string spaces = " \t\n\r\f\v";
    const std::size_t first = text.find_first_not_of(spaces);
    const std::size_t last = text.find_last_not_of(spaces);
    std::string result = {};

    if (first == std::string::npos) {
        return result;
    }

    result = text.substr(first, last - first + 1U);

    return result;
}

TestCase parse_test_case(std::string const& line)
{
    const char separator = '=';
    const std::size_t position = line.find(separator);
    TestCase test_case = {"", 0.0};
    std::string expected_text = {};

    if (position == std::string::npos) {
        throw std::runtime_error("Missing expected value separator");
    }

    test_case.expression = trim_copy(line.substr(0U, position));
    expected_text = trim_copy(line.substr(position + 1U));

    if (test_case.expression.empty()) {
        throw std::runtime_error("Expression is empty");
    }

    if (expected_text.empty()) {
        throw std::runtime_error("Expected value is empty");
    }

    test_case.expected = calculate(expected_text);

    return test_case;
}

bool should_skip_line(std::string const& line)
{
    const char comment_symbol = '#';
    const std::string trimmed = trim_copy(line);
    const bool result = trimmed.empty() || trimmed.front() == comment_symbol;

    return result;
}

std::size_t run_file_tests(std::string const& path)
{
    std::fstream stream(path, std::ios::in);
    std::string line = {};
    TestCase test_case = {"", 0.0};
    double actual = 0.0;
    std::size_t line_number = 0U;
    std::size_t passed_count = 0U;

    if (!stream.is_open()) {
        throw std::runtime_error("Cannot open input file");
    }

    while (std::getline(stream, line)) {
        ++line_number;

        if (!should_skip_line(line)) {
            try {
                test_case = parse_test_case(line);
                actual = calculate(test_case.expression);

                if (!almost_equal(actual, test_case.expected)) {
                    std::cerr << "Failed line " << line_number << ": "
                              << line << '\n';
                    std::cerr << "Actual: " << actual
                              << ", expected: " << test_case.expected << '\n';
                }

                assert(almost_equal(actual, test_case.expected));
                ++passed_count;
            } catch (std::exception const& error) {
                std::cerr << "Error in line " << line_number << ": "
                          << line << '\n';
                throw;
            }
        }
    }

    return passed_count;
}

void test_parser()
{
    assert(almost_equal(calculate("2 + 3 * 4"), 14.0));
    assert(almost_equal(calculate("(2 + 3) * 4"), 20.0));
    assert(almost_equal(calculate("10 / 4"), 2.5));
    assert(almost_equal(calculate("10 % 3"), 1.0));
    assert(almost_equal(calculate("5.5 % 2"), 1.5));
    assert(almost_equal(calculate("2 ^ 3"), 8.0));
    assert(almost_equal(calculate("2 ^ 3 ^ 2"), 512.0));
    assert(almost_equal(calculate("0!"), 1.0));
    assert(almost_equal(calculate("5!"), 120.0));
    assert(almost_equal(calculate("3! + 2 ^ 3"), 14.0));
    assert(almost_equal(calculate("[2 + 3] * {4 + 1}"), 25.0));
    assert(almost_equal(calculate("{[2 + 3] * (4 + 1)} / 5"), 5.0));
}

void test_errors()
{
    bool failed = false;

    try {
        static_cast<void>(calculate("5.5!"));
    } catch (std::runtime_error const&) {
        failed = true;
    }

    assert(failed);
}

void test_parse_test_case()
{
    const std::string line = "[2 + 3] * {4 + 1} = 25";
    const TestCase test_case = parse_test_case(line);

    assert(test_case.expression == "[2 + 3] * {4 + 1}");
    assert(almost_equal(test_case.expected, 25.0));
}

void test_all()
{
    test_parser();
    test_errors();
    test_parse_test_case();
}

int main(int argc, char* argv[])
{
    const std::string default_path = "input.data";
    const std::string path = argc > 1 ? argv[1] : default_path;
    std::size_t passed_count = 0U;

    try {
        test_all();
        passed_count = run_file_tests(path);
    } catch (std::exception const& error) {
        std::cerr << "Error: " << error.what() << '\n';

        return 1;
    }

    std::cout << "Self-check: OK\n";
    std::cout << "File tests passed: " << passed_count << '\n';

    return 0;
}