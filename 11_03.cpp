#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

template <typename T, typename Compare>
const T& median_of_three(const T& x, const T& y, const T& z, Compare compare)
{
    if (compare(x, y))
    {
        if (compare(y, z))
        {
            return y;
        }
        return compare(x, z) ? z : x;
    }

    if (compare(x, z))
    {
        return x;
    }

    return compare(y, z) ? z : y;
}

template <std::random_access_iterator RandomIt, typename Compare>
void insertion_sort(RandomIt first, RandomIt last, Compare compare)
{
    RandomIt i = first;
    RandomIt j = first;
    RandomIt previous = first;
    const auto size = last - first;

    if (size <= 1)
    {
        return;
    }

    i = first + 1;
    for (; i != last; ++i)
    {
        j = i;
        while (j != first)
        {
            previous = j - 1;
            if (!compare(*j, *previous))
            {
                break;
            }
            std::iter_swap(j, previous);
            j = previous;
        }
    }
}

template <std::random_access_iterator RandomIt, typename Compare>
auto select_pivot(RandomIt first, RandomIt last, Compare compare)
{
    RandomIt middle = first;
    RandomIt last_element = first;
    const auto size = last - first;

    middle = first + size / 2;
    last_element = last - 1;

    return median_of_three(*first, *middle, *last_element, compare);
}

template <std::random_access_iterator RandomIt, typename Compare>
RandomIt partition_hoare(RandomIt first, RandomIt last, Compare compare)
{
    RandomIt left = first;
    RandomIt right = last - 1;
    const auto pivot = select_pivot(first, last, compare);

    while (true)
    {
        while (compare(*left, pivot))
        {
            ++left;
        }

        while (compare(pivot, *right))
        {
            --right;
        }

        if (left >= right)
        {
            return right + 1;
        }

        std::iter_swap(left, right);
        ++left;
        --right;
    }
}

template <std::random_access_iterator RandomIt, typename Compare>
void split(RandomIt first, RandomIt last, Compare compare)
{
    using Difference = typename std::iterator_traits<RandomIt>::difference_type;

    constexpr Difference kInsertionLimit = 16;
    RandomIt middle = first;
    const Difference size = last - first;

    if (size <= 1)
    {
        return;
    }

    if (size <= kInsertionLimit)
    {
        insertion_sort(first, last, compare);
        return;
    }

    middle = partition_hoare(first, last, compare);
    split(first, middle, compare);
    split(middle, last, compare);
}

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
void sort_range(RandomIt first, RandomIt last, Compare compare = Compare{})
{
    split(first, last, compare);
}

bool greater_int(int lhs, int rhs)
{
    return lhs > rhs;
}

template <typename Container>
void print_sequence(const Container& values)
{
    typename Container::const_iterator it = values.begin();
    bool first = true;

    for (; it != values.end(); ++it)
    {
        if (!first)
        {
            std::cout << ' ';
        }
        std::cout << *it;
        first = false;
    }
    std::cout << '\n';
}

void test_default_comparator()
{
    std::vector<double> values = {3.0, -1.5, 2.0, 2.0, 0.0};

    sort_range(values.begin(), values.end());

    assert(std::is_sorted(values.begin(), values.end()));
}

void test_free_function_comparator()
{
    std::vector<int> values = {5, 1, 4, 2, 3};
    std::vector<int> expected = {5, 4, 3, 2, 1};

    sort_range(values.begin(), values.end(), greater_int);

    assert(std::is_sorted(values.begin(), values.end(), greater_int));
    assert(values == expected);
}

void test_std_less_comparator()
{
    std::deque<int> values = {7, 3, 9, 1, 5, 1};
    std::deque<int> expected = {1, 1, 3, 5, 7, 9};
    std::less<> less = {};

    sort_range(values.begin(), values.end(), less);

    assert(std::is_sorted(values.begin(), values.end(), less));
    assert(values == expected);
}

void test_lambda_comparator()
{
    std::vector<std::string> values = {
        "pear",
        "fig",
        "banana",
        "kiwi",
        "apple",
        "plum"
    };
    std::vector<std::string> expected = {
        "fig",
        "kiwi",
        "pear",
        "plum",
        "apple",
        "banana"
    };
    auto by_length_then_lex =
        [](const std::string& lhs, const std::string& rhs)
        {
            return (lhs.size() < rhs.size()) ||
                   (lhs.size() == rhs.size() && lhs < rhs);
        };

    sort_range(values.begin(), values.end(), by_length_then_lex);

    assert(std::is_sorted(values.begin(), values.end(), by_length_then_lex));
    assert(values == expected);
}

void test_small_ranges()
{
    std::vector<int> empty = {};
    std::vector<int> single = {42};

    sort_range(empty.begin(), empty.end());
    sort_range(single.begin(), single.end());

    assert(empty.empty());
    assert(single.size() == 1U);
    assert(single[0] == 42);
}

int main()
{
    std::vector<int> free_demo = {5, 1, 4, 2, 3};
    std::deque<int> less_demo = {7, 3, 9, 1, 5, 1};
    std::vector<std::string> lambda_demo = {
        "pear",
        "fig",
        "banana",
        "kiwi",
        "apple",
        "plum"
    };
    std::less<> less = {};
    auto by_length_then_lex =
        [](const std::string& lhs, const std::string& rhs)
        {
            return (lhs.size() < rhs.size()) ||
                   (lhs.size() == rhs.size() && lhs < rhs);
        };

    test_default_comparator();
    test_free_function_comparator();
    test_std_less_comparator();
    test_lambda_comparator();
    test_small_ranges();

    sort_range(free_demo.begin(), free_demo.end(), greater_int);
    sort_range(less_demo.begin(), less_demo.end(), less);
    sort_range(lambda_demo.begin(), lambda_demo.end(), by_length_then_lex);

    std::cout << "Free function comparator:\n";
    print_sequence(free_demo);

    std::cout << "std::less comparator:\n";
    print_sequence(less_demo);

    std::cout << "Lambda comparator:\n";
    print_sequence(lambda_demo);

    return 0;
}