#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::uint8_t make_byte(unsigned int value)
{
    return static_cast<std::uint8_t>(value);
}

std::string bytes_to_hex(std::vector<std::uint8_t> const& bytes)
{
    const int output_width = 2;
    const char fill_symbol = '0';
    std::stringstream stream;
    std::size_t index = 0U;
    unsigned int value = 0U;

    stream << std::hex << std::right << std::setfill(fill_symbol);

    for (index = 0U; index < bytes.size(); ++index) {
        value = static_cast<unsigned int>(bytes[index]);
        stream << std::setw(output_width) << value;
    }

    return stream.str();
}

std::uint8_t hex_digit_to_value(char digit)
{
    const char zero = '0';
    const char nine = '9';
    const char lower_a = 'a';
    const char lower_f = 'f';
    const unsigned int decimal_base = 10U;
    unsigned int value = 0U;
    const bool decimal_digit = zero <= digit && digit <= nine;
    const bool lower_hex_digit = lower_a <= digit && digit <= lower_f;

    if (decimal_digit) {
        value = static_cast<unsigned int>(digit - zero);
    } else if (lower_hex_digit) {
        value = decimal_base + static_cast<unsigned int>(digit - lower_a);
    } else {
        throw std::invalid_argument("Only lowercase hexadecimal digits are allowed");
    }

    return make_byte(value);
}

std::vector<std::uint8_t> hex_to_bytes(std::string const& text)
{
    const std::size_t digits_per_byte = 2U;
    const unsigned int half_byte_shift = 4U;
    std::vector<std::uint8_t> bytes = {};
    std::size_t index = 0U;
    std::uint8_t high = 0U;
    std::uint8_t low = 0U;
    unsigned int value = 0U;

    if (text.size() % digits_per_byte != 0U) {
        throw std::invalid_argument("Hex string length must be even");
    }

    bytes.reserve(text.size() / digits_per_byte);

    for (index = 0U; index < text.size(); index += digits_per_byte) {
        high = hex_digit_to_value(text[index]);
        low = hex_digit_to_value(text[index + 1U]);
        value = static_cast<unsigned int>(high);
        value = (value << half_byte_shift) | static_cast<unsigned int>(low);

        bytes.push_back(make_byte(value));
    }

    return bytes;
}

void test_bytes_to_hex()
{
    const std::vector<std::uint8_t> bytes = {
        make_byte(0U),
        make_byte(15U),
        make_byte(16U),
        make_byte(171U),
        make_byte(255U)
    };
    const std::string expected = "000f10abff";
    const std::string result = bytes_to_hex(bytes);

    assert(result == expected);
}

void test_hex_to_bytes()
{
    const std::string text = "000f10abff";
    const std::vector<std::uint8_t> expected = {
        make_byte(0U),
        make_byte(15U),
        make_byte(16U),
        make_byte(171U),
        make_byte(255U)
    };
    const std::vector<std::uint8_t> result = hex_to_bytes(text);

    assert(result == expected);
}

void test_empty_input()
{
    const std::vector<std::uint8_t> bytes = {};
    const std::string text = "";
    const std::string hex = bytes_to_hex(bytes);
    const std::vector<std::uint8_t> parsed = hex_to_bytes(text);

    assert(hex.empty());
    assert(parsed.empty());
}

void test_round_trip()
{
    const std::vector<std::uint8_t> bytes = {
        make_byte(1U),
        make_byte(2U),
        make_byte(127U),
        make_byte(128U),
        make_byte(254U)
    };
    const std::string hex = bytes_to_hex(bytes);
    const std::vector<std::uint8_t> parsed = hex_to_bytes(hex);

    assert(parsed == bytes);
}

void test_invalid_odd_length()
{
    bool failed = false;

    try {
        static_cast<void>(hex_to_bytes("abc"));
    } catch (std::invalid_argument const&) {
        failed = true;
    }

    assert(failed);
}

void test_invalid_uppercase()
{
    bool failed = false;

    try {
        static_cast<void>(hex_to_bytes("0A"));
    } catch (std::invalid_argument const&) {
        failed = true;
    }

    assert(failed);
}

void test_invalid_symbol()
{
    bool failed = false;

    try {
        static_cast<void>(hex_to_bytes("0g"));
    } catch (std::invalid_argument const&) {
        failed = true;
    }

    assert(failed);
}

void test_all()
{
    test_bytes_to_hex();
    test_hex_to_bytes();
    test_empty_input();
    test_round_trip();
    test_invalid_odd_length();
    test_invalid_uppercase();
    test_invalid_symbol();
}

void print_bytes(std::vector<std::uint8_t> const& bytes)
{
    std::size_t index = 0U;

    for (index = 0U; index < bytes.size(); ++index) {
        if (index != 0U) {
            std::cout << ' ';
        }

        std::cout << static_cast<unsigned int>(bytes[index]);
    }

    std::cout << '\n';
}

int main()
{
    const std::vector<std::uint8_t> bytes = {
        make_byte(0U),
        make_byte(15U),
        make_byte(16U),
        make_byte(171U),
        make_byte(255U)
    };
    const std::string hex = bytes_to_hex(bytes);
    const std::vector<std::uint8_t> parsed = hex_to_bytes(hex);

    test_all();

    std::cout << "Self-check: OK\n";
    std::cout << "Hex string: " << hex << '\n';
    std::cout << "Parsed bytes: ";
    print_bytes(parsed);

    return 0;
}