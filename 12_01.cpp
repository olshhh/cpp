#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

std::string rub_code()
{
    return "RUB";
}

long double rub_per_usd_rate()
{
    const long double rate = 90.0L;

    return rate;
}

long double decimal_multiplier(int digits)
{
    long double result = 1.0L;
    int index = 0;

    for (index = 0; index < digits; ++index) {
        result *= 10.0L;
    }

    return result;
}

bool almost_equal(long double left, long double right)
{
    const long double epsilon = 1e-9L;
    const long double difference = std::abs(left - right);

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

bool starts_with_rub(std::string const& text)
{
    const std::string code = rub_code();
    const std::string trimmed = trim_copy(text);
    const bool result = trimmed.size() >= code.size()
        && trimmed.compare(0U, code.size(), code) == 0;

    return result;
}

bool ends_with_rub(std::string const& text)
{
    const std::string code = rub_code();
    const std::string trimmed = trim_copy(text);
    const std::size_t offset = trimmed.size() >= code.size()
        ? trimmed.size() - code.size()
        : 0U;
    const bool result = trimmed.size() >= code.size()
        && trimmed.compare(offset, code.size(), code) == 0;

    return result;
}

bool has_rub_code(std::string const& text)
{
    const bool result = starts_with_rub(text) || ends_with_rub(text);

    return result;
}

std::string remove_rub_code(std::string const& text)
{
    const std::string code = rub_code();
    const std::string trimmed = trim_copy(text);
    std::string result = {};

    if (starts_with_rub(trimmed)) {
        result = trim_copy(trimmed.substr(code.size()));
    } else if (ends_with_rub(trimmed)) {
        result = trim_copy(trimmed.substr(0U, trimmed.size() - code.size()));
    } else {
        result = trimmed;
    }

    return result;
}

bool parse_money_with_get_money(std::string const& amount,
    std::locale const& locale, long double& subunits)
{
    long double value = 0.0L;
    std::stringstream stream(amount);

    stream.imbue(locale);
    stream >> std::get_money(value, false);
    stream >> std::ws;

    if (stream.fail() || !stream.eof()) {
        return false;
    }

    subunits = value;

    return true;
}

bool parse_decimal_money(std::string const& amount, std::locale const& locale,
    long double& subunits)
{
    const auto& punctuation = std::use_facet<std::moneypunct<char, false>>(
        locale);
    const char locale_decimal = punctuation.decimal_point();
    const char locale_separator = punctuation.thousands_sep();
    const int fraction_digits = punctuation.frac_digits();
    const long double multiplier = decimal_multiplier(fraction_digits);
    const std::string trimmed = trim_copy(amount);
    std::string normalized = {};
    std::size_t index = 0U;
    std::size_t parsed_count = 0U;
    long double major_units = 0.0L;
    bool has_decimal_point = false;
    bool parsed = false;

    for (index = 0U; index < trimmed.size(); ++index) {
        const char symbol = trimmed[index];
        const bool digit = '0' <= symbol && symbol <= '9';
        const bool decimal = symbol == locale_decimal || symbol == ','
            || symbol == '.';
        const bool separator = symbol == locale_separator || symbol == ' ';

        if (digit) {
            normalized.push_back(symbol);
        } else if (decimal && !has_decimal_point) {
            normalized.push_back('.');
            has_decimal_point = true;
        } else if (separator) {
        } else {
            return false;
        }
    }

    if (normalized.empty() || normalized == ".") {
        return false;
    }

    try {
        major_units = std::stold(normalized, &parsed_count);
        parsed = parsed_count == normalized.size();
    } catch (std::exception const&) {
        parsed = false;
    }

    if (!parsed) {
        return false;
    }

    subunits = std::round(major_units * multiplier);

    return true;
}

bool parse_rub_money(std::string const& text, std::locale const& locale,
    long double& subunits)
{
    const bool contains_code = has_rub_code(text);
    const std::string amount = remove_rub_code(text);
    bool parsed = false;

    if (!contains_code) {
        return false;
    }

    parsed = parse_money_with_get_money(amount, locale, subunits);

    if (!parsed) {
        parsed = parse_decimal_money(amount, locale, subunits);
    }

    return parsed;
}

std::string format_local_money(std::locale const& locale, long double subunits)
{
    const long double rounded = std::round(subunits);
    std::stringstream stream;

    stream.imbue(locale);
    stream << std::put_money(rounded, false);

    return stream.str();
}

std::string format_usd_money(std::locale const& locale, long double subunits)
{
    const long double rounded = std::round(subunits);
    std::stringstream stream;

    stream.imbue(locale);
    stream << std::showbase << std::put_money(rounded, true);

    return stream.str();
}

long double convert_rub_to_usd(long double rub_subunits, long double rate)
{
    const long double usd_subunits = std::round(rub_subunits / rate);

    return usd_subunits;
}

void test_conversion()
{
    const long double rub_subunits = 900000.0L;
    const long double expected_usd_subunits = 10000.0L;
    const long double usd_subunits = convert_rub_to_usd(rub_subunits,
        rub_per_usd_rate());

    assert(almost_equal(usd_subunits, expected_usd_subunits));
}

void test_get_money_generated_input(std::locale const& russian_locale)
{
    const long double expected_subunits = 12345.0L;
    const std::string amount = format_local_money(russian_locale,
        expected_subunits);
    const std::string text = rub_code() + " " + amount;
    long double parsed_subunits = 0.0L;
    const bool parsed = parse_rub_money(text, russian_locale, parsed_subunits);

    assert(parsed);
    assert(almost_equal(parsed_subunits, expected_subunits));
}

void test_parse_prefix_and_suffix(std::locale const& russian_locale)
{
    const std::string prefix_text = "RUB 123,45";
    const std::string suffix_text = "123,45 RUB";
    const std::string no_code_text = "123,45";
    const long double expected_subunits = 12345.0L;
    long double prefix_subunits = 0.0L;
    long double suffix_subunits = 0.0L;
    long double no_code_subunits = 0.0L;
    const bool prefix_parsed = parse_rub_money(prefix_text, russian_locale,
        prefix_subunits);
    const bool suffix_parsed = parse_rub_money(suffix_text, russian_locale,
        suffix_subunits);
    const bool no_code_parsed = parse_rub_money(no_code_text, russian_locale,
        no_code_subunits);

    assert(prefix_parsed);
    assert(suffix_parsed);
    assert(!no_code_parsed);
    assert(almost_equal(prefix_subunits, expected_subunits));
    assert(almost_equal(suffix_subunits, expected_subunits));
}

void test_format_usd(std::locale const& american_locale)
{
    const long double usd_subunits = 12345.0L;
    const std::string text = format_usd_money(american_locale, usd_subunits);

    assert(!text.empty());
}

void test_all(std::locale const& russian_locale, std::locale const& american_locale)
{
    test_conversion();
    test_get_money_generated_input(russian_locale);
    test_parse_prefix_and_suffix(russian_locale);
    test_format_usd(american_locale);
}

int main()
{
    const std::string russian_locale_name = "ru_RU.utf8";
    const std::string american_locale_name = "en_US.utf8";
    const long double rate = rub_per_usd_rate();
    std::locale russian_locale = std::locale::classic();
    std::locale american_locale = std::locale::classic();
    std::string input = {};
    long double rub_subunits = 0.0L;
    long double usd_subunits = 0.0L;
    bool parsed = false;

    try {
        russian_locale = std::locale(russian_locale_name.c_str());
        american_locale = std::locale(american_locale_name.c_str());
    } catch (std::runtime_error const& error) {
        std::cerr << "Locale error: " << error.what() << '\n';
        std::cerr << "Install locales: sudo locale-gen ru_RU.utf8 en_US.utf8\n";

        return 1;
    }

    test_all(russian_locale, american_locale);
    std::cout << "Self-check: OK\n";

    std::cout << "Exchange rate: " << static_cast<double>(rate)
              << " RUB for 1 USD\n";
    std::cout << "Enter RUB amount, for example RUB 123,45 or 123,45 RUB: ";

    if (!std::getline(std::cin, input)) {
        return 0;
    }

    parsed = parse_rub_money(input, russian_locale, rub_subunits);

    if (!parsed) {
        std::cerr << "Invalid RUB amount\n";

        return 1;
    }

    usd_subunits = convert_rub_to_usd(rub_subunits, rate);

    std::cout << "USD amount: " << format_usd_money(american_locale,
        usd_subunits) << '\n';

    return 0;
}