#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::size_t cache_index(std::size_t row, std::size_t column, std::size_t size)
{
    const std::size_t index = row * size + column;

    return index;
}

std::string_view longest_palindrome(std::string_view text)
{
    const std::size_t size = text.size();
    const std::size_t empty_position = 0U;
    const std::size_t empty_length = 0U;
    const std::size_t single_length = 1U;
    const std::size_t pair_length = 2U;
    std::vector<bool> cache(size * size, false);
    std::size_t best_position = empty_position;
    std::size_t best_length = empty_length;
    std::size_t length = 0U;
    std::size_t left = 0U;
    std::size_t right = 0U;
    bool same_edges = false;
    bool short_palindrome = false;
    bool inner_palindrome = false;
    bool palindrome = false;
    std::string_view result = text.substr(empty_position, empty_length);

    if (size == 0U) {
        return result;
    }

    best_length = single_length;

    for (left = 0U; left < size; ++left) {
        cache[cache_index(left, left, size)] = true;
    }

    for (length = pair_length; length <= size; ++length) {
        for (left = 0U; left + length <= size; ++left) {
            right = left + length - 1U;
            same_edges = text[left] == text[right];
            short_palindrome = length == pair_length;
            inner_palindrome = length > pair_length
                && cache[cache_index(left + 1U, right - 1U, size)];
            palindrome = same_edges && (short_palindrome || inner_palindrome);

            cache[cache_index(left, right, size)] = palindrome;

            if (palindrome && length > best_length) {
                best_position = left;
                best_length = length;
            }
        }
    }

    result = text.substr(best_position, best_length);

    return result;
}

void test_empty_string()
{
    const std::string text = "";
    const std::string_view expected = "";
    const std::string_view result = longest_palindrome(text);

    assert(result == expected);
}

void test_single_character()
{
    const std::string text = "a";
    const std::string_view expected = "a";
    const std::string_view result = longest_palindrome(text);

    assert(result == expected);
}

void test_odd_palindrome()
{
    const std::string text = "babad";
    const std::string_view result = longest_palindrome(text);
    const bool valid = result == "bab" || result == "aba";

    assert(valid);
}

void test_even_palindrome()
{
    const std::string text = "cbbd";
    const std::string_view expected = "bb";
    const std::string_view result = longest_palindrome(text);

    assert(result == expected);
}

void test_full_palindrome()
{
    const std::string text = "racecar";
    const std::string_view expected = "racecar";
    const std::string_view result = longest_palindrome(text);

    assert(result == expected);
}

void test_long_palindrome_inside()
{
    const std::string text = "forgeeksskeegfor";
    const std::string_view expected = "geeksskeeg";
    const std::string_view result = longest_palindrome(text);

    assert(result == expected);
}

void test_no_long_palindrome()
{
    const std::string text = "abcd";
    const std::size_t expected_size = 1U;
    const std::string_view result = longest_palindrome(text);

    assert(result.size() == expected_size);
}

void test_all()
{
    test_empty_string();
    test_single_character();
    test_odd_palindrome();
    test_even_palindrome();
    test_full_palindrome();
    test_long_palindrome_inside();
    test_no_long_palindrome();
}

int main()
{
    std::string input = {};
    std::string_view result = {};

    test_all();
    std::cout << "Self-check: OK\n";

    std::cout << "Enter string: ";
    if (!std::getline(std::cin, input)) {
        return 0;
    }

    result = longest_palindrome(input);

    std::cout << "Longest palindrome substring: " << result << '\n';

    return 0;
}