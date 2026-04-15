#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// Median of three values.
template <typename T>
const T& median_of_three(const T& x, const T& y, const T& z)
{
    if (x < y)
    {
        if (y < z)
        {
            return y;
        }
        return (x < z) ? z : x;
    }

    if (x < z)
    {
        return x;
    }

    return (y < z) ? z : y;
}

// Insertion sort on range [first, last).
template <std::random_access_iterator RandomIt>
void order(RandomIt first, RandomIt last)
{
    RandomIt i = first;
    RandomIt j = first;
    RandomIt previous = first;
    const auto size = std::distance(first, last);

    if (size <= 1)
    {
        return;
    }

    i = std::next(first);
    for (; i != last; ++i)
    {
        j = i;
        while (j != first)
        {
            previous = std::prev(j);
            if (!(*j < *previous))
            {
                break;
            }
            std::iter_swap(j, previous);
            j = previous;
        }
    }
}

// Select pivot on range [first, last).
template <std::random_access_iterator RandomIt>
auto select_pivot(RandomIt first, RandomIt last)
{
    RandomIt middle = first;
    RandomIt last_element = first;
    const auto size = std::distance(first, last);

    std::advance(middle, size / 2);
    last_element = std::prev(last);

    return median_of_three(*first, *middle, *last_element);
}

// Hoare partition on range [first, last).
// Returns iterator middle such that recursive calls use
// [first, middle) and [middle, last).
template <std::random_access_iterator RandomIt>
RandomIt partition_hoare(RandomIt first, RandomIt last)
{
    RandomIt left = first;
    RandomIt right = first;
    const auto pivot = select_pivot(first, last);

    right = std::prev(last);

    while (true)
    {
        while (*left < pivot)
        {
            ++left;
        }

        while (pivot < *right)
        {
            --right;
        }

        if (left >= right)
        {
            return std::next(right);
        }

        std::iter_swap(left, right);
        ++left;
        --right;
    }
}

// Hybrid sort on range [first, last).
template <std::random_access_iterator RandomIt>
void split(RandomIt first, RandomIt last)
{
    constexpr int kInsertionLimit = 16;
    const auto size = std::distance(first, last);
    RandomIt middle = first;

    if (size <= 1)
    {
        return;
    }

    if (size <= kInsertionLimit)
    {
        order(first, last);
        return;
    }

    middle = partition_hoare(first, last);
    split(first, middle);
    split(middle, last);
}

template <std::random_access_iterator RandomIt>
void sort_range(RandomIt first, RandomIt last)
{
    split(first, last);
}

// Type for template demo tests.
struct Box
{
    int key;
};

bool operator<(const Box& a, const Box& b)
{
    return a.key < b.key;
}

void test_iterator_functions()
{
    std::vector<int> values = {1, 2, 3, 4, 5};
    std::vector<int>::iterator iterator = values.begin();

    assert(std::distance(iterator, values.end()) == 5);

    std::advance(iterator, 2);

    assert(*std::next(iterator, 1) == 4);
    assert(*std::prev(iterator, 1) == 2);

    std::iter_swap(values.begin(), iterator);

    assert((values == std::vector<int>{3, 2, 1, 4, 5}));
}

void test_int_vector()
{
    constexpr int kSelfTestSize = 1000;
    std::vector<int> values(kSelfTestSize, 0);
    int i = 0;

    for (; i < kSelfTestSize; ++i)
    {
        values[static_cast<std::size_t>(i)] = kSelfTestSize - i;
    }

    sort_range(values.begin(), values.end());
    assert(std::is_sorted(values.begin(), values.end()));
}

void test_double_vector()
{
    std::vector<double> values = {3.0, -1.5, 2.0, 2.0, 0.0};

    sort_range(values.begin(), values.end());
    assert(std::is_sorted(values.begin(), values.end()));
}

void test_string_vector()
{
    std::vector<std::string> values = {
        "pear",
        "apple",
        "banana",
        "banana",
        "apricot"
    };

    sort_range(values.begin(), values.end());
    assert(std::is_sorted(values.begin(), values.end()));
}

void test_box_vector()
{
    std::vector<Box> values = {{3}, {1}, {2}, {2}, {0}};

    sort_range(values.begin(), values.end());
    assert(std::is_sorted(
        values.begin(),
        values.end(),
        [](const Box& a, const Box& b)
        {
            return a.key < b.key;
        }));
}

void test_deque()
{
    std::deque<int> values = {7, 3, 9, 1, 5, 1};

    sort_range(values.begin(), values.end());
    assert(std::is_sorted(values.begin(), values.end()));
}

int main()
{
    std::vector<int> demo = {9, 4, 7, 1, 3, 8, 2, 6, 5};
    std::size_t i = 0U;

    test_iterator_functions();
    test_int_vector();
    test_double_vector();
    test_string_vector();
    test_box_vector();
    test_deque();

    sort_range(demo.begin(), demo.end());

    std::cout << "Sorted sequence:\n";
    for (i = 0U; i < demo.size(); ++i)
    {
        if (i != 0U)
        {
            std::cout << ' ';
        }
        std::cout << demo[i];
    }
    std::cout << '\n';

    return 0;
}