#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace fibonacci {

constexpr int kStartA = 0;
constexpr int kStartB = 1;

class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    Iterator() : current_(kStartA), next_(kStartB) {
    }

    Iterator(const int current, const int next)
        : current_(current), next_(next) {
    }

    Iterator& operator++()
    {
        const int new_next = current_ + next_;

        current_ = next_;
        next_ = new_next;

        return *this;
    }

    Iterator operator++(int)
    {
        Iterator old = *this;

        ++(*this);

        return old;
    }

    reference operator*() const
    {
        return current_;
    }

    pointer operator->() const
    {
        return &current_;
    }

    friend bool operator==(const Iterator& left, const Iterator& right)
    {
        return left.current_ == right.current_ && left.next_ == right.next_;
    }

    friend bool operator!=(const Iterator& left, const Iterator& right)
    {
        return !(left == right);
    }

private:
    int current_;
    int next_;
};

class BoostIterator
    : public boost::iterator_facade<
          BoostIterator,
          const int,
          boost::forward_traversal_tag>
{
public:
    using iterator_category = std::forward_iterator_tag;

    BoostIterator() : current_(kStartA), next_(kStartB) {
    }

    BoostIterator(const int current, const int next)
        : current_(current), next_(next) {
    }

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        const int new_next = current_ + next_;

        current_ = next_;
        next_ = new_next;
    }

    bool equal(const BoostIterator& other) const
    {
        return current_ == other.current_ && next_ == other.next_;
    }

    const int& dereference() const
    {
        return current_;
    }

    int current_;
    int next_;
};

template <typename IteratorType>
class Sequence
{
public:
    explicit Sequence(const int count) : count_(count) {
    }

    IteratorType begin() const
    {
        return IteratorType(kStartA, kStartB);
    }

    IteratorType end() const
    {
        IteratorType iterator = begin();
        int i = 0;

        for (; i < count_; ++i)
        {
            ++iterator;
        }

        return iterator;
    }

private:
    int count_;
};

template <typename IteratorType>
std::vector<int> collect(const Sequence<IteratorType>& sequence)
{
    std::vector<int> values = {};
    IteratorType iterator = sequence.begin();
    IteratorType last = sequence.end();

    for (; iterator != last; ++iterator)
    {
        values.push_back(*iterator);
    }

    return values;
}

void test_manual_iterator()
{
    Iterator iterator = Iterator();
    Iterator old = Iterator();

    assert(*iterator == 0);

    old = iterator++;
    assert(*old == 0);
    assert(*iterator == 1);

    ++iterator;
    assert(*iterator == 1);

    ++iterator;
    assert(*iterator == 2);
}

void test_boost_iterator()
{
    BoostIterator iterator = BoostIterator();
    BoostIterator old = BoostIterator();

    assert(*iterator == 0);

    old = iterator++;
    assert(*old == 0);
    assert(*iterator == 1);

    ++iterator;
    assert(*iterator == 1);

    ++iterator;
    assert(*iterator == 2);
}

void test_empty_sequence()
{
    const Sequence<Iterator> manual_sequence(0);
    const Sequence<BoostIterator> boost_sequence(0);
    const std::vector<int> expected = {};
    const std::vector<int> manual_values = collect(manual_sequence);
    const std::vector<int> boost_values = collect(boost_sequence);

    assert(manual_values == expected);
    assert(boost_values == expected);
}

void test_first_ten_numbers()
{
    const Sequence<Iterator> manual_sequence(10);
    const Sequence<BoostIterator> boost_sequence(10);
    const std::vector<int> expected = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    const std::vector<int> manual_values = collect(manual_sequence);
    const std::vector<int> boost_values = collect(boost_sequence);

    assert(manual_values == expected);
    assert(boost_values == expected);
}

void test_both_algorithms_match()
{
    const Sequence<Iterator> manual_sequence(15);
    const Sequence<BoostIterator> boost_sequence(15);
    const std::vector<int> manual_values = collect(manual_sequence);
    const std::vector<int> boost_values = collect(boost_sequence);

    assert(manual_values == boost_values);
}

} // namespace fibonacci

int main()
{
    const fibonacci::Sequence<fibonacci::Iterator> manual_sequence(12);
    const fibonacci::Sequence<fibonacci::BoostIterator> boost_sequence(12);
    const std::vector<int> manual_values = fibonacci::collect(manual_sequence);
    const std::vector<int> boost_values = fibonacci::collect(boost_sequence);
    std::size_t i = 0U;

    fibonacci::test_manual_iterator();
    fibonacci::test_boost_iterator();
    fibonacci::test_empty_sequence();
    fibonacci::test_first_ten_numbers();
    fibonacci::test_both_algorithms_match();

    std::cout << "Manual iterator:\n";
    for (i = 0U; i < manual_values.size(); ++i)
    {
        if (i != 0U)
        {
            std::cout << ' ';
        }

        std::cout << manual_values[i];
    }
    std::cout << '\n';

    std::cout << "Boost facade iterator:\n";
    for (i = 0U; i < boost_values.size(); ++i)
    {
        if (i != 0U)
        {
            std::cout << ' ';
        }

        std::cout << boost_values[i];
    }
    std::cout << '\n';

    return 0;
}