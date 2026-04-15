#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace
{
using Addresses = std::vector<std::uintptr_t>;
using Sizes = std::vector<std::size_t>;
using Ratio = std::pair<std::size_t, std::size_t>;

Sizes collect_vector_capacities(const std::size_t push_count)
{
    Sizes capacities = {};
    std::vector<int> values = {};
    const int* old_data = nullptr;
    std::size_t old_capacity = 0U;
    std::size_t old_size = 0U;
    std::size_t i = 0U;
    std::size_t j = 0U;

    for (i = 0U; i < push_count; ++i)
    {
        old_capacity = values.capacity();
        old_size = values.size();
        old_data = values.data();

        values.push_back(static_cast<int>(i));

        assert(values.size() == old_size + 1U);
        assert(values[i] == static_cast<int>(i));

        if (old_size < old_capacity)
        {
            assert(values.capacity() == old_capacity);
            assert(values.data() == old_data);
        }
        else if (old_capacity != 0U)
        {
            assert(values.capacity() > old_capacity);
        }

        for (j = 0U; j < values.size(); ++j)
        {
            assert(values[j] == static_cast<int>(j));
        }

        if (values.capacity() != old_capacity)
        {
            capacities.push_back(values.capacity());
        }
    }

    return capacities;
}

Ratio detect_vector_growth_factor(const Sizes& capacities)
{
    std::vector<Ratio> ratios = {};
    Ratio factor = {0U, 0U};
    std::size_t gcd_value = 0U;
    std::size_t i = 0U;
    std::size_t suffix_length = 1U;
    const std::size_t kMinStableSuffixLength = 2U;

    assert(capacities.size() >= 2U);

    for (i = 1U; i < capacities.size(); ++i)
    {
        gcd_value = std::gcd(capacities[i], capacities[i - 1U]);
        ratios.push_back({capacities[i] / gcd_value, capacities[i - 1U] / gcd_value});
    }

    factor = ratios.back();

    for (i = ratios.size(); i > 1U; --i)
    {
        if (ratios[i - 2U] != factor)
        {
            break;
        }

        ++suffix_length;
    }

    assert(suffix_length >= kMinStableSuffixLength);
    assert(factor.first > factor.second);

    return factor;
}

Addresses collect_deque_addresses(const std::size_t push_count)
{
    Addresses addresses = {};
    std::deque<int> values = {};
    std::uintptr_t current_address = 0U;
    std::size_t old_size = 0U;
    std::size_t i = 0U;
    std::size_t j = 0U;

    for (i = 0U; i < push_count; ++i)
    {
        old_size = values.size();
        values.push_back(static_cast<int>(i));

        assert(values.size() == old_size + 1U);
        assert(values.back() == static_cast<int>(i));

        for (j = 0U; j < old_size; ++j)
        {
            assert(values[j] == static_cast<int>(j));
            assert(reinterpret_cast<std::uintptr_t>(std::addressof(values[j])) == addresses[j]);
        }

        current_address = reinterpret_cast<std::uintptr_t>(std::addressof(values.back()));
        addresses.push_back(current_address);
        assert(reinterpret_cast<std::uintptr_t>(std::addressof(values[old_size])) == addresses[old_size]);
    }

    return addresses;
}

Sizes build_deque_runs(const Addresses& addresses)
{
    Sizes runs = {};
    const std::uintptr_t kStep = sizeof(int);
    std::size_t run_length = 0U;
    std::size_t i = 0U;
    std::size_t element_count = 0U;

    assert(!addresses.empty());

    run_length = 1U;

    for (i = 1U; i < addresses.size(); ++i)
    {
        if (addresses[i] > addresses[i - 1U] && addresses[i] - addresses[i - 1U] == kStep)
        {
            ++run_length;
        }
        else
        {
            runs.push_back(run_length);
            run_length = 1U;
        }
    }

    runs.push_back(run_length);

    for (i = 0U; i < runs.size(); ++i)
    {
        element_count += runs[i];
    }

    assert(element_count == addresses.size());

    return runs;
}

std::size_t detect_deque_page_size(const Sizes& runs)
{
    std::map<std::size_t, std::size_t> frequency = {};
    std::size_t best_run = 0U;
    std::size_t best_count = 0U;
    std::size_t i = 0U;

    for (i = 0U; i < runs.size(); ++i)
    {
        ++frequency[runs[i]];
    }

    for (const auto& entry : frequency)
    {
        if (entry.second > best_count || (entry.second == best_count && entry.first > best_run))
        {
            best_run = entry.first;
            best_count = entry.second;
        }
    }

    assert(best_run != 0U);
    assert(best_count >= 2U);

    return best_run;
}

void print_sizes(const char* const label, const Sizes& values)
{
    std::size_t i = 0U;

    std::cout << label;

    for (i = 0U; i < values.size(); ++i)
    {
        if (i != 0U)
        {
            std::cout << ' ';
        }

        std::cout << values[i];
    }

    std::cout << '\n';
}
} // namespace

int main()
{
    const std::size_t kVectorPushCount = 512U;
    const std::size_t kDequePushCount = 2048U;
    Sizes vector_capacities = {};
    Ratio vector_factor = {0U, 0U};
    Addresses deque_addresses = {};
    Sizes deque_runs = {};
    std::size_t deque_page_size = 0U;
    std::size_t complete_pages = 0U;
    std::size_t i = 0U;

    static_assert(std::random_access_iterator<std::deque<int>::iterator>);
    static_assert(!std::contiguous_iterator<std::deque<int>::iterator>);
    static_assert(std::contiguous_iterator<std::vector<int>::iterator>);

    vector_capacities = collect_vector_capacities(kVectorPushCount);
    vector_factor = detect_vector_growth_factor(vector_capacities);

    assert(vector_capacities.size() >= 8U);
    assert(std::is_sorted(vector_capacities.begin(), vector_capacities.end()));

    deque_addresses = collect_deque_addresses(kDequePushCount);
    deque_runs = build_deque_runs(deque_addresses);
    deque_page_size = detect_deque_page_size(deque_runs);

    for (i = 0U; i < deque_runs.size(); ++i)
    {
        complete_pages += deque_runs[i] / deque_page_size;
    }

    assert(deque_page_size > 1U);
    assert(complete_pages >= 2U);

    print_sizes("Vector capacity changes:", vector_capacities);
    std::cout << "Vector growth factor: " << vector_factor.first << '/' << vector_factor.second << '\n';
    print_sizes("Deque contiguous run lengths:", deque_runs);
    std::cout << "Deque page size in elements: " << deque_page_size << '\n';
    std::cout << "Deque page size in bytes for int: " << deque_page_size * sizeof(int) << '\n';
}