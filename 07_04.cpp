#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// Insertion sort on range [left, right).
template <typename T>
void order(std::vector<T> &data, std::size_t left, std::size_t right)
{
    std::size_t i = 0U;
    std::size_t j = 0U;

    i = left + 1U;
    for (; i < right; ++i) {
        j = i;
        for (; j > left; --j) {
            if (data[j - 1U] > data[j]) {
                std::swap(data[j], data[j - 1U]);
            }
        }
    }
}

// Pivot as median of first, middle and last elements on inclusive [low, high].
template <typename T>
T select_pivot(const std::vector<T> &data, std::ptrdiff_t low, std::ptrdiff_t high)
{
    std::ptrdiff_t middle = 0;
    std::size_t low_index = 0U;
    std::size_t middle_index = 0U;
    std::size_t high_index = 0U;

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

    if ((x <= y && y <= z) || (z <= y && y <= x)) {
        pivot = y;
    } else if ((x <= z && z <= y) || (y <= z && z <= x)) {
        pivot = z;
    }

    return pivot;
}

// Hoare partition on inclusive [low, high].
template <typename T>
std::ptrdiff_t partition_hoare(std::vector<T> &data, std::ptrdiff_t low, std::ptrdiff_t high)
{
    T pivot = T{};
    std::ptrdiff_t i = 0;
    std::ptrdiff_t j = 0;

    pivot = select_pivot(data, low, high);
    i = low - 1;
    j = high + 1;

    while (true) {
        do {
            ++i;
        } while (data[static_cast<std::size_t>(i)] < pivot);

        do {
            --j;
        } while (data[static_cast<std::size_t>(j)] > pivot);

        if (i >= j) {
            return j;
        }

        std::swap(data[static_cast<std::size_t>(i)], data[static_cast<std::size_t>(j)]);
    }
}

// Sort with Hoare partition and insertion sort.
template <typename T>
void split(std::vector<T> &data, std::size_t left, std::size_t right)
{
    constexpr std::size_t kInsertionLimit = 16U;

    std::size_t size = 0U;
    std::ptrdiff_t low = 0;
    std::ptrdiff_t high = 0;
    std::ptrdiff_t pivot_pos = 0;
    std::size_t middle = 0U;

    size = right - left;

    if (size <= 1U) {
        return;
    }

    if (size <= kInsertionLimit) {
        order(data, left, right);
        return;
    }

    low = static_cast<std::ptrdiff_t>(left);
    high = static_cast<std::ptrdiff_t>(right) - 1;

    pivot_pos = partition_hoare(data, low, high);
    middle = static_cast<std::size_t>(pivot_pos) + 1U;

    split(data, left, middle);
    split(data, middle, right);
}

template <typename T>
void sort(std::vector<T> &data)
{
    split(data, 0U, data.size());
}

struct Box
{
    int key = 0;
};

bool operator<(const Box &left, const Box &right)
{
    return left.key < right.key;
}

bool operator>(const Box &left, const Box &right)
{
    return right < left;
}

bool operator<=(const Box &left, const Box &right)
{
    return !(right < left);
}

bool operator==(const Box &left, const Box &right)
{
    return left.key == right.key;
}

TEST(SortTest, EmptyVector)
{
    std::vector<int> data = {};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
    ASSERT_TRUE(data.empty());
}

TEST(SortTest, OneElement)
{
    std::vector<int> data = {42};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
    ASSERT_EQ(data.size(), 1U);
    ASSERT_EQ(data[0], 42);
}

TEST(SortTest, IntegersDescending)
{
    constexpr std::size_t kSize = 1000U;

    std::size_t i = 0U;
    std::vector<int> data(kSize, 0);

    for (i = 0U; i < kSize; ++i) {
        data[i] = static_cast<int>(kSize - i);
    }

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
}

TEST(SortTest, IntegersWithDuplicates)
{
    std::vector<int> data = {5, 1, 3, 3, 2, 5, 0, 0, -1};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
    ASSERT_EQ(data.front(), -1);
    ASSERT_EQ(data.back(), 5);
}

TEST(SortTest, AlreadySortedIntegers)
{
    std::vector<int> data = {1, 2, 3, 4, 5, 6};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
    ASSERT_EQ(data[0], 1);
    ASSERT_EQ(data[5], 6);
}

TEST(SortTest, Doubles)
{
    std::vector<double> data = {3.0, -1.5, 2.0, 2.0, 0.0};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
}

TEST(SortTest, Strings)
{
    std::vector<std::string> data = {"pear", "apple", "banana", "banana", "apricot"};

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
    ASSERT_EQ(data[0], "apple");
    ASSERT_EQ(data[4], "pear");
}

TEST(SortTest, CustomTypeBox)
{
    std::vector<Box> data = {{3}, {1}, {2}, {2}, {0}};

    sort(data);

    ASSERT_TRUE(std::is_sorted(
        data.begin(),
        data.end(),
        [](const Box &left, const Box &right) { return left.key < right.key; }));

    ASSERT_EQ(data[0], Box{0});
    ASSERT_EQ(data[1], Box{1});
    ASSERT_EQ(data[2], Box{2});
    ASSERT_EQ(data[3], Box{2});
    ASSERT_EQ(data[4], Box{3});
}

TEST(SortTest, LargeRepeatedValues)
{
    constexpr std::size_t kSize = 128U;

    std::size_t i = 0U;
    std::vector<int> data(kSize, 0);

    for (i = 0U; i < kSize; ++i) {
        data[i] = static_cast<int>(i % 7U);
    }

    sort(data);

    ASSERT_TRUE(std::is_sorted(data.begin(), data.end()));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}