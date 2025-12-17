#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Insertion sort on range [left, right)
template <typename T>
void order(std::vector<T>& data, std::size_t left, std::size_t right)
{
    std::size_t i = 0;
    std::size_t j = 0;

    i = left + 1;
    for (; i < right; ++i)
    {
        j = i;
        for (; j > left; --j)
        {
            if (data[j - 1] > data[j])
            {
                std::swap(data[j], data[j - 1]);
            }
        }
    }
}

// pivot as median of first, middle and last elements (inclusive [low, high])
template <typename T>
T select_pivot(const std::vector<T>& data, std::ptrdiff_t low, std::ptrdiff_t high)
{
    std::ptrdiff_t middle = 0;
    std::size_t low_index = 0;
    std::size_t middle_index = 0;
    std::size_t high_index = 0;

    T x = T{};
    T y = T{};
    T z = T{};
    T pivot = T{};

    middle = low + (high - low) / 2;

    low_index = static_cast<std::size_t>(low);
    middle_index = static_cast<std::size_t>(middle);
    high_index = static_cast<std::size_t>(high);

    x = data[low_index];
    y = data[middle_index];
    z = data[high_index];

    pivot = x;

    if ((x <= y && y <= z) || (z <= y && y <= x))
    {
        pivot = y;
    }
    else if ((x <= z && z <= y) || (y <= z && z <= x))
    {
        pivot = z;
    }

    return pivot;
}

// Hoare partition on inclusive [low, high]
template <typename T>
std::ptrdiff_t partition_hoare(std::vector<T>& data, std::ptrdiff_t low, std::ptrdiff_t high)
{
    T pivot = select_pivot(data, low, high);
    std::ptrdiff_t i = low - 1;
    std::ptrdiff_t j = high + 1;

    while (true)
    {
        do
        {
            ++i;
        } while (data[static_cast<std::size_t>(i)] < pivot);

        do
        {
            --j;
        } while (data[static_cast<std::size_t>(j)] > pivot);

        if (i >= j)
        {
            return j;
        }

        std::swap(data[static_cast<std::size_t>(i)], data[static_cast<std::size_t>(j)]);
    }
}

////sort with Hoare partition and insertion sort
template <typename T>
void split(std::vector<T>& data, std::size_t left, std::size_t right)
{
    constexpr std::size_t kInsertionLimit = 16;

    std::size_t size = 0;
    std::ptrdiff_t low = 0;
    std::ptrdiff_t high = 0;
    std::ptrdiff_t pivot_pos = 0;
    std::size_t middle = 0;

    size = right - left;

    if (size <= 1)
    {
        return;
    }

    if (size <= kInsertionLimit)
    {
        order(data, left, right);
        return;
    }

    low = static_cast<std::ptrdiff_t>(left);
    high = static_cast<std::ptrdiff_t>(right) - 1;

    pivot_pos = partition_hoare(data, low, high);
    middle = static_cast<std::size_t>(pivot_pos) + 1;

    split(data, left, middle);
    split(data, middle, right);
}

template <typename T>
void sort(std::vector<T>& data)
{
    split(data, 0, data.size());
}

// Type for template demo tests
struct Box
{
    int key;
};

bool operator<(const Box& a, const Box& b) { return a.key < b.key; }
bool operator>(const Box& a, const Box& b) { return b < a; }
bool operator<=(const Box& a, const Box& b) { return !(b < a); }

int main()
{
    {
        constexpr std::size_t kSelfTestSize = 1000;

        std::size_t i = 0;
        std::vector<int> v(kSelfTestSize, 0);

        i = 0;
        for (; i < kSelfTestSize; ++i)
        {
            v[i] = static_cast<int>(kSelfTestSize - i);
        }

        sort(v);
        assert(std::is_sorted(v.begin(), v.end()));
    }

    {
        std::vector<double> v = {3.0, -1.5, 2.0, 2.0, 0.0};
        sort(v);
        assert(std::is_sorted(v.begin(), v.end()));
    }

    {
        std::vector<std::string> v = {"pear", "apple", "banana", "banana", "apricot"};
        sort(v);
        assert(std::is_sorted(v.begin(), v.end()));
    }

    {
        std::vector<Box> v = {{3}, {1}, {2}, {2}, {0}};
        sort(v);
        assert(std::is_sorted(v.begin(), v.end(),
                              [](const Box& a, const Box& b) { return a.key < b.key; }));
    }

    {
        std::size_t n = 0;
        std::size_t i = 0;

        std::cout << "Enter how many numbers you want to sort: ";

        if (!(std::cin >> n))
        {
            return 0;
        }

        std::vector<int> user(n, 0);

        i = 0;
        for (; i < n; ++i)
        {
            std::cin >> user[i];
        }

        sort(user);

        std::cout << "Sorted sequence:\n";

        i = 0;
        for (; i < n; ++i)
        {
            if (i != 0)
            {
                std::cout << ' ';
            }
            std::cout << user[i];
        }
        std::cout << '\n';
    }

    return 0;
}
