#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

template <typename T>
class List
{
private:
    struct Node
    {
        T value;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;

        explicit Node(const T& new_value)
            : value(new_value), next(nullptr), prev() {
        }
    };

public:
    class Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(
            std::shared_ptr<Node> node = nullptr,
            std::weak_ptr<Node> tail = std::weak_ptr<Node>())
            : node_(node), tail_(tail) {
        }

        Iterator& operator++()
        {
            assert(node_ != nullptr);
            node_ = node_->next;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator old = *this;
            ++(*this);
            return old;
        }

        Iterator& operator--()
        {
            std::shared_ptr<Node> previous = nullptr;

            if (node_ == nullptr)
            {
                previous = tail_.lock();
                assert(previous != nullptr);
                node_ = previous;
                return *this;
            }

            previous = node_->prev.lock();
            assert(previous != nullptr);
            node_ = previous;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator old = *this;
            --(*this);
            return old;
        }

        reference operator*() const
        {
            assert(node_ != nullptr);
            return node_->value;
        }

        pointer operator->() const
        {
            assert(node_ != nullptr);
            return &node_->value;
        }

        friend bool operator==(const Iterator& left, const Iterator& right)
        {
            return left.node_ == right.node_;
        }

        friend bool operator!=(const Iterator& left, const Iterator& right)
        {
            return !(left == right);
        }

    private:
        std::shared_ptr<Node> node_;
        std::weak_ptr<Node> tail_;
    };

    List() : head_(nullptr), tail_(nullptr) {
    }

    Iterator begin()
    {
        return Iterator(head_, tail_);
    }

    Iterator end()
    {
        return Iterator(nullptr, tail_);
    }

    Iterator begin() const
    {
        return Iterator(head_, tail_);
    }

    Iterator end() const
    {
        return Iterator(nullptr, tail_);
    }

    void push_back(const T& value)
    {
        std::shared_ptr<Node> node = nullptr;

        node = std::make_shared<Node>(value);

        if (tail_ != nullptr)
        {
            node->prev = tail_;
            tail_->next = node;
            tail_ = node;
        }
        else
        {
            head_ = node;
            tail_ = node;
        }
    }

private:
    std::shared_ptr<Node> head_;
    std::shared_ptr<Node> tail_;
};

void test_forward_iteration()
{
    List<int> list;
    List<int>::Iterator iterator = List<int>::Iterator();
    std::vector<int> values = {};

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    iterator = list.begin();
    while (iterator != list.end())
    {
        values.push_back(*iterator);
        ++iterator;
    }

    assert((values == std::vector<int>{1, 2, 3}));
}

void test_increment_operators()
{
    List<int> list;
    List<int>::Iterator iterator = List<int>::Iterator();
    List<int>::Iterator old_iterator = List<int>::Iterator();

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    iterator = list.begin();
    old_iterator = iterator++;

    assert(*old_iterator == 1);
    assert(*iterator == 2);

    ++iterator;
    assert(*iterator == 3);
}

void test_decrement_operators()
{
    List<int> list;
    List<int>::Iterator iterator = List<int>::Iterator();
    List<int>::Iterator old_iterator = List<int>::Iterator();
    std::vector<int> values = {};

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    iterator = list.end();
    old_iterator = iterator--;

    assert(old_iterator == list.end());
    assert(*iterator == 3);

    values.push_back(*iterator);

    --iterator;
    values.push_back(*iterator);

    --iterator;
    values.push_back(*iterator);

    assert((values == std::vector<int>{3, 2, 1}));
}

void test_operator_arrow()
{
    List<std::string> list;
    List<std::string>::Iterator iterator = List<std::string>::Iterator();

    list.push_back("abc");
    iterator = list.begin();

    assert(iterator->size() == 3U);
    assert(*iterator == "abc");
}

void test_range_based_for()
{
    List<int> list;
    int sum = 0;

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    for (const int value : list)
    {
        sum += value;
    }

    assert(sum == 6);
}

int main()
{
    List<int> list;
    List<int>::Iterator iterator = List<int>::Iterator();
    bool first = true;

    test_forward_iteration();
    test_increment_operators();
    test_decrement_operators();
    test_operator_arrow();
    test_range_based_for();

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    std::cout << "Forward:\n";
    iterator = list.begin();
    first = true;
    while (iterator != list.end())
    {
        if (!first)
        {
            std::cout << ' ';
        }

        std::cout << *iterator;
        first = false;
        ++iterator;
    }
    std::cout << '\n';

    std::cout << "Backward:\n";
    iterator = list.end();
    first = true;
    while (iterator != list.begin())
    {
        --iterator;

        if (!first)
        {
            std::cout << ' ';
        }

        std::cout << *iterator;
        first = false;
    }
    std::cout << '\n';

    std::cout << "Range-based:\n";
    first = true;
    for (const int value : list)
    {
        if (!first)
        {
            std::cout << ' ';
        }

        std::cout << value;
        first = false;
    }
    std::cout << '\n';

    return 0;
}