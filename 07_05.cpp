#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <benchmark/benchmark.h>

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

template <typename T>
void split(std::vector<T> &data, std::size_t left, std::size_t right, std::size_t insertion_limit)
{
    std::size_t size = 0U;
    std::ptrdiff_t low = 0;
    std::ptrdiff_t high = 0;
    std::ptrdiff_t pivot_pos = 0;
    std::size_t middle = 0U;

    size = right - left;

    if (size <= 1U) {
        return;
    }

    if (size <= insertion_limit) {
        order(data, left, right);
        return;
    }

    low = static_cast<std::ptrdiff_t>(left);
    high = static_cast<std::ptrdiff_t>(right) - 1;

    pivot_pos = partition_hoare(data, low, high);
    middle = static_cast<std::size_t>(pivot_pos) + 1U;

    split(data, left, middle, insertion_limit);
    split(data, middle, right, insertion_limit);
}

template <typename T>
void sort(std::vector<T> &data, std::size_t insertion_limit)
{
    split(data, 0U, data.size(), insertion_limit);
}

std::vector<double> make_reversed_data(std::size_t size)
{
    std::size_t i = 0U;
    std::vector<double> data(size, 0.0);

    for (i = 0U; i < size; ++i) {
        data[i] = static_cast<double>(size - i);
    }

    return data;
}

void self_test()
{
    {
        std::vector<double> data = {5.0, 4.0, 3.0, 2.0, 1.0};
        sort(data, 16U);
        assert(std::is_sorted(data.begin(), data.end()));
    }

    {
        std::vector<double> data = {3.0, -1.5, 2.0, 2.0, 0.0};
        sort(data, 8U);
        assert(std::is_sorted(data.begin(), data.end()));
    }

    {
        std::vector<double> data = {};
        sort(data, 4U);
        assert(std::is_sorted(data.begin(), data.end()));
    }
}

static void benchmark_sort(benchmark::State &state)
{
    const std::size_t data_size = 10000U;
    const std::size_t insertion_limit = static_cast<std::size_t>(state.range(0));
    const std::vector<double> source = make_reversed_data(data_size);
    std::vector<double> data = {};

    for (auto _ : state) {
        state.PauseTiming();
        data = source;
        state.ResumeTiming();

        sort(data, insertion_limit);
        benchmark::DoNotOptimize(data);
    }

    state.SetLabel("threshold=" + std::to_string(insertion_limit));
}

BENCHMARK(benchmark_sort)->DenseRange(2, 32, 2);

int main(int argc, char **argv)
{
    self_test();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}