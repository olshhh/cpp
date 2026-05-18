#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

class Fibonacci : public std::ranges::view_interface<Fibonacci>
{
private:
    using Value = std::uint64_t;

    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;

        Iterator(Value current, Value next, std::size_t index, std::size_t limit)
            : current_(current), next_(next), index_(index), limit_(limit)
        {
        }

        value_type operator*() const
        {
            return current_;
        }

        Iterator& operator++()
        {
            const Value next_value = current_ + next_;

            current_ = next_;
            next_ = next_value;
            ++index_;

            return *this;
        }

        Iterator operator++(int)
        {
            Iterator old = *this;

            ++(*this);

            return old;
        }

        friend bool operator==(Iterator const& left, Iterator const& right)
        {
            return left.index_ == right.index_ && left.limit_ == right.limit_;
        }

        friend bool operator==(Iterator const& iterator, std::default_sentinel_t)
        {
            return iterator.index_ >= iterator.limit_;
        }

        friend bool operator==(std::default_sentinel_t sentinel,
            Iterator const& iterator)
        {
            return iterator == sentinel;
        }

    private:
        Value current_ = 0U;
        Value next_ = 1U;
        std::size_t index_ = 0U;
        std::size_t limit_ = 0U;
    };

public:
    Fibonacci() = default;

    explicit Fibonacci(std::size_t count) : count_(count)
    {
    }

    auto begin() const
    {
        return Iterator(0U, 1U, 0U, count_);
    }

    auto end() const
    {
        return std::default_sentinel;
    }

    std::size_t size() const
    {
        return count_;
    }

private:
    std::size_t count_ = 0U;
};

static_assert(std::ranges::view<Fibonacci>);
static_assert(std::ranges::forward_range<Fibonacci>);

template <std::ranges::input_range Range>
auto to_vector(Range&& range)
{
    using Value = std::ranges::range_value_t<std::remove_cvref_t<Range>>;

    std::vector<Value> values = {};

    if constexpr (std::ranges::sized_range<Range>) {
        values.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }

    std::ranges::copy(range, std::back_inserter(values));

    return values;
}

template <std::ranges::input_range Range, typename Output, typename Predicate,
    typename Operation>
Output transform_if(Range&& range, Output output, Predicate predicate,
    Operation operation)
{
    using Reference = std::ranges::range_reference_t<Range>;
    using Result = std::remove_cvref_t<std::invoke_result_t<Operation&, Reference>>;

    std::vector<std::optional<Result>> transformed = {};
    std::vector<std::optional<Result>> filtered = {};
    const auto make_optional = [&](auto&& value) -> std::optional<Result> {
        if (predicate(value)) {
            return operation(value);
        }

        return std::nullopt;
    };
    const auto has_value = [](std::optional<Result> const& value) {
        return value.has_value();
    };
    const auto extract = [](std::optional<Result> const& value) {
        return *value;
    };

    if constexpr (std::ranges::sized_range<Range>) {
        transformed.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }

    std::ranges::transform(range, std::back_inserter(transformed), make_optional);

    filtered.reserve(transformed.size());
    std::ranges::copy_if(transformed, std::back_inserter(filtered), has_value);

    return std::ranges::transform(filtered, output, extract).out;
}

double mean_absolute_error(std::vector<double> const& expected,
    std::vector<double> const& actual)
{
    const auto sum = [](double left, double right) {
        return left + right;
    };
    const auto absolute_difference = [](double left, double right) {
        const double difference = left - right;

        return std::abs(difference);
    };
    const bool same_size = expected.size() == actual.size();
    const bool empty = expected.empty();
    const double count = static_cast<double>(expected.size());
    double error = 0.0;

    if (!same_size) {
        throw std::invalid_argument("MAE requires equal sizes");
    }

    if (empty) {
        return error;
    }

    error = std::transform_reduce(expected.begin(), expected.end(), actual.begin(),
        0.0, sum, absolute_difference);

    return error / count;
}

double mean_squared_error(std::vector<double> const& expected,
    std::vector<double> const& actual)
{
    const auto sum = [](double left, double right) {
        return left + right;
    };
    const auto squared_difference = [](double left, double right) {
        const double difference = left - right;

        return difference * difference;
    };
    const bool same_size = expected.size() == actual.size();
    const bool empty = expected.empty();
    const double count = static_cast<double>(expected.size());
    double error = 0.0;

    if (!same_size) {
        throw std::invalid_argument("MSE requires equal sizes");
    }

    if (empty) {
        return error;
    }

    error = std::transform_reduce(expected.begin(), expected.end(), actual.begin(),
        0.0, sum, squared_difference);

    return error / count;
}

bool almost_equal(double left, double right)
{
    const double epsilon = 1e-9;
    const double difference = std::abs(left - right);

    return difference <= epsilon;
}

void test_replace()
{
    const int old_value = 1;
    const int new_value = 9;
    std::vector<int> values = {1, 2, 1, 3};
    const std::vector<int> expected = {9, 2, 9, 3};

    std::ranges::replace(values, old_value, new_value);

    assert(values == expected);
}

void test_fill()
{
    const std::size_t size = 4U;
    const int value = 7;
    std::vector<int> values(size);
    const std::vector<int> expected = {7, 7, 7, 7};

    std::ranges::fill(values, value);

    assert(values == expected);
}

void test_unique()
{
    std::vector<int> values = {1, 1, 2, 2, 2, 3};
    const std::vector<int> expected = {1, 2, 3};
    auto tail = std::ranges::unique(values);

    values.erase(tail.begin(), tail.end());

    assert(values == expected);
}

void test_rotate()
{
    const std::ptrdiff_t offset = 2;
    std::vector<int> values = {1, 2, 3, 4, 5};
    const std::vector<int> expected = {3, 4, 5, 1, 2};
    auto middle = std::ranges::next(values.begin(), offset);

    std::ranges::rotate(values, middle);

    assert(values == expected);
}

void test_sample()
{
    const std::size_t sample_size = 3U;
    const unsigned int seed = 42U;
    std::vector<int> values = {1, 2, 3, 4, 5};
    std::vector<int> sample = {};
    std::mt19937 generator(seed);

    std::ranges::sample(values, std::back_inserter(sample), sample_size, generator);

    assert(sample.size() == sample_size);

    for (int value : sample) {
        assert(std::ranges::find(values, value) != values.end());
    }
}

void test_transform_if()
{
    const auto is_even = [](int value) {
        const int divisor = 2;

        return value % divisor == 0;
    };
    const auto square = [](int value) {
        return value * value;
    };
    std::vector<int> values = {1, 2, 3, 4, 5};
    std::vector<int> result = {};
    const std::vector<int> expected = {4, 16};

    transform_if(values, std::back_inserter(result), is_even, square);

    assert(result == expected);
}

void test_errors()
{
    const std::vector<double> expected = {1.0, 2.0, 3.0};
    const std::vector<double> actual = {2.0, 2.0, 5.0};
    const std::vector<double> empty = {};
    const std::vector<double> shorter = {1.0, 2.0};
    const double expected_mae = 1.0;
    const double expected_mse = 5.0 / 3.0;
    const double mae = mean_absolute_error(expected, actual);
    const double mse = mean_squared_error(expected, actual);
    bool mae_failed = false;
    bool mse_failed = false;

    assert(almost_equal(mae, expected_mae));
    assert(almost_equal(mse, expected_mse));
    assert(almost_equal(mean_absolute_error(empty, empty), 0.0));
    assert(almost_equal(mean_squared_error(empty, empty), 0.0));

    try {
        static_cast<void>(mean_absolute_error(expected, shorter));
    } catch (std::invalid_argument const&) {
        mae_failed = true;
    }

    try {
        static_cast<void>(mean_squared_error(expected, shorter));
    } catch (std::invalid_argument const&) {
        mse_failed = true;
    }

    assert(mae_failed);
    assert(mse_failed);
}

void test_filter()
{
    const auto is_even = [](int value) {
        const int divisor = 2;

        return value % divisor == 0;
    };
    std::vector<int> values = {1, 2, 3, 4, 5, 6};
    auto view = values | std::views::filter(is_even);
    const std::vector<int> result = to_vector(view);
    const std::vector<int> expected = {2, 4, 6};

    assert(result == expected);
}

void test_drop()
{
    const std::size_t count = 2U;
    std::vector<int> values = {1, 2, 3, 4};
    auto view = values | std::views::drop(count);
    const std::vector<int> result = to_vector(view);
    const std::vector<int> expected = {3, 4};

    assert(result == expected);
}

void test_join()
{
    std::vector<std::vector<int>> values = {{1, 2}, {3}, {4, 5}};
    auto view = values | std::views::join;
    const std::vector<int> result = to_vector(view);
    const std::vector<int> expected = {1, 2, 3, 4, 5};

    assert(result == expected);
}

void test_zip()
{
    const auto add_pair = [](auto const& pair) {
        return std::get<0>(pair) + std::get<1>(pair);
    };
    std::vector<int> left = {1, 2, 3};
    std::vector<int> right = {4, 5, 6};
    std::vector<int> result = {};
    const std::vector<int> expected = {5, 7, 9};
    auto view = std::views::zip(left, right);

    std::ranges::transform(view, std::back_inserter(result), add_pair);

    assert(result == expected);
}

void test_stride()
{
    const int first = 0;
    const int last = 10;
    const int step = 3;
    auto view = std::views::iota(first, last) | std::views::stride(step);
    const std::vector<int> result = to_vector(view);
    const std::vector<int> expected = {0, 3, 6, 9};

    assert(result == expected);
}

void test_fibonacci()
{
    const std::size_t count = 10U;
    Fibonacci view(count);
    const std::vector<std::uint64_t> result = to_vector(view);
    const std::vector<std::uint64_t> expected = {0U, 1U, 1U, 2U, 3U,
        5U, 8U, 13U, 21U, 34U};

    assert(std::ranges::view<Fibonacci>);
    assert(result == expected);
}

void test_empty_fibonacci()
{
    const std::size_t count = 0U;
    Fibonacci view(count);
    const std::vector<std::uint64_t> result = to_vector(view);
    const std::vector<std::uint64_t> expected = {};

    assert(result == expected);
}

void test_fibonacci_drop_take()
{
    const std::size_t count = 10U;
    const std::size_t drop_count = 2U;
    const std::size_t take_count = 5U;
    Fibonacci view(count);
    auto middle = view | std::views::drop(drop_count) | std::views::take(take_count);
    const std::vector<std::uint64_t> result = to_vector(middle);
    const std::vector<std::uint64_t> expected = {1U, 2U, 3U, 5U, 8U};

    assert(result == expected);
}

void test_all()
{
    test_replace();
    test_fill();
    test_unique();
    test_rotate();
    test_sample();
    test_transform_if();
    test_errors();
    test_filter();
    test_drop();
    test_join();
    test_zip();
    test_stride();
    test_fibonacci();
    test_empty_fibonacci();
    test_fibonacci_drop_take();
}

template <typename T>
void print_vector(std::string_view title, std::vector<T> const& values)
{
    std::size_t index = 0U;

    std::cout << title;

    for (index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            std::cout << ' ';
        }

        std::cout << values[index];
    }

    std::cout << '\n';
}

int main()
{
    const std::size_t fibonacci_count = 12U;
    Fibonacci fibonacci(fibonacci_count);
    const std::vector<std::uint64_t> fibonacci_values = to_vector(fibonacci);

    test_all();

    std::cout << "Self-check: OK\n";
    print_vector("Fibonacci view: ", fibonacci_values);

    return 0;
}