#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <vector>

// 09.10 - Common allocator interface

class Allocator
{
public:
    virtual ~Allocator() = default;

    virtual void* allocate(
        std::size_t size,
        std::size_t alignment = alignof(std::max_align_t)) = 0;

    virtual void deallocate(void* pointer) = 0;

protected:
    template <typename T>
    T* get(void* pointer) const noexcept
    {
        return static_cast<T*>(pointer);
    }

    template <typename T>
    const T* get(const void* pointer) const noexcept
    {
        return static_cast<const T*>(pointer);
    }

    static constexpr std::size_t default_alignment =
        alignof(std::max_align_t);

    static constexpr std::size_t align_up(
        const std::size_t value,
        const std::size_t alignment) noexcept
    {
        return ((value + alignment - 1U) / alignment) * alignment;
    }
};

bool is_aligned(const void* pointer, const std::size_t alignment)
{
    const std::uintptr_t value =
        reinterpret_cast<std::uintptr_t>(pointer);

    return value % alignment == 0U;
}

// 09.28 - Linear allocator

class LinearAllocator final : public Allocator
{
public:
    explicit LinearAllocator(const std::size_t size)
        : size_(size), offset_(0U), begin_(nullptr)
    {
        begin_ = ::operator new(size_, std::align_val_t(default_alignment));
    }

    ~LinearAllocator() override
    {
        ::operator delete(begin_, std::align_val_t(default_alignment));
    }

    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    void* allocate(
        const std::size_t size,
        const std::size_t alignment = default_alignment) override
    {
        void* aligned = nullptr;
        std::size_t free = 0U;
        std::byte* current = nullptr;

        if (alignment > default_alignment)
        {
            return nullptr;
        }

        current = get<std::byte>(begin_) + offset_;
        aligned = current;
        free = size_ - offset_;

        if (std::align(alignment, size, aligned, free) == nullptr)
        {
            return nullptr;
        }

        offset_ = size_ - free + size;
        return aligned;
    }

    void deallocate(void*) override
    {
    }

private:
    std::size_t size_;
    std::size_t offset_;
    void* begin_;
};

// 09.29 - Stack allocator

class StackAllocator final : public Allocator
{
public:
    explicit StackAllocator(const std::size_t size)
        : size_(size), offset_(0U), begin_(nullptr)
    {
        begin_ = ::operator new(size_, std::align_val_t(default_alignment));
    }

    ~StackAllocator() override
    {
        ::operator delete(begin_, std::align_val_t(default_alignment));
    }

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    void* allocate(
        const std::size_t size,
        const std::size_t alignment = default_alignment) override
    {
        void* aligned = nullptr;
        std::size_t free = 0U;
        std::byte* current = nullptr;
        Header* header = nullptr;

        if (alignment > default_alignment)
        {
            return nullptr;
        }

        if (offset_ + sizeof(Header) > size_)
        {
            return nullptr;
        }

        current = get<std::byte>(begin_) + offset_ + sizeof(Header);
        aligned = current;
        free = size_ - offset_ - sizeof(Header);

        if (std::align(alignment, size, aligned, free) == nullptr)
        {
            return nullptr;
        }

        header = get<Header>(get<std::byte>(aligned) - sizeof(Header));
        header->previous_offset = offset_;

        offset_ =
            static_cast<std::size_t>(
                get<std::byte>(aligned) - get<std::byte>(begin_)) +
            size;

        return aligned;
    }

    void deallocate(void* pointer) override
    {
        Header* header = nullptr;

        if (pointer == nullptr)
        {
            return;
        }

        header = get<Header>(get<std::byte>(pointer) - sizeof(Header));
        offset_ = header->previous_offset;
    }

private:
    struct Header
    {
        std::size_t previous_offset = 0U;
    };

    std::size_t size_;
    std::size_t offset_;
    void* begin_;
};

// 09.30 - List allocator

class ListAllocator final : public Allocator
{
public:
    ListAllocator(const std::size_t page_size, const std::size_t step)
        : page_size_(page_size),
          block_size_(0U),
          free_head_(nullptr),
          current_page_(nullptr),
          current_offset_(0U),
          pages_()
    {
        block_size_ = step;
        if (block_size_ < sizeof(Node))
        {
            block_size_ = sizeof(Node);
        }

        block_size_ = align_up(block_size_, default_alignment);

        assert(page_size_ >= block_size_);
        assert(page_size_ % block_size_ == 0U);

        add_page();
    }

    ~ListAllocator() override
    {
        std::size_t i = 0U;

        for (i = 0U; i < pages_.size(); ++i)
        {
            ::operator delete(
                pages_[i],
                std::align_val_t(default_alignment));
        }
    }

    ListAllocator(const ListAllocator&) = delete;
    ListAllocator& operator=(const ListAllocator&) = delete;

    void* allocate(
        const std::size_t size,
        const std::size_t alignment = default_alignment) override
    {
        Node* node = nullptr;
        void* result = nullptr;

        if (size > block_size_ || alignment > default_alignment)
        {
            return nullptr;
        }

        if (free_head_ != nullptr)
        {
            node = free_head_;
            free_head_ = free_head_->next;
            return node;
        }

        if (current_page_ == nullptr ||
            current_offset_ + block_size_ > page_size_)
        {
            add_page();
        }

        result = get<std::byte>(current_page_) + current_offset_;
        current_offset_ += block_size_;

        return result;
    }

    void deallocate(void* pointer) override
    {
        Node* node = nullptr;

        if (pointer == nullptr)
        {
            return;
        }

        node = get<Node>(pointer);
        node->next = free_head_;
        free_head_ = node;
    }

private:
    struct Node
    {
        Node* next = nullptr;
    };

    void add_page()
    {
        current_page_ =
            ::operator new(page_size_, std::align_val_t(default_alignment));
        current_offset_ = 0U;
        pages_.push_back(current_page_);
    }

    std::size_t page_size_;
    std::size_t block_size_;
    Node* free_head_;
    void* current_page_;
    std::size_t current_offset_;
    std::vector<void*> pages_;
};

// 09.31 - Free list allocator

class FreeListAllocator final : public Allocator
{
public:
    explicit FreeListAllocator(const std::size_t size)
        : size_(align_up(size, default_alignment)),
          begin_(nullptr),
          head_(nullptr)
    {
        assert(size_ >= min_free_block_size_);

        begin_ = ::operator new(size_, std::align_val_t(default_alignment));
        head_ = get<Node>(begin_);
        head_->size = size_;
        head_->next = nullptr;
    }

    ~FreeListAllocator() override
    {
        ::operator delete(begin_, std::align_val_t(default_alignment));
    }

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    void* allocate(
        const std::size_t size,
        const std::size_t alignment = default_alignment) override
    {
        std::size_t total = 0U;
        std::size_t remainder = 0U;
        std::pair<Node*, Node*> found = std::make_pair(nullptr, nullptr);
        Node* current = nullptr;
        Node* previous = nullptr;
        Node* next_free = nullptr;
        Header* header = nullptr;

        if (size == 0U || alignment > default_alignment)
        {
            return nullptr;
        }

        total = align_up(sizeof(Header) + size, default_alignment);
        found = find(total);
        current = found.first;
        previous = found.second;

        if (current == nullptr)
        {
            return nullptr;
        }

        remainder = current->size - total;

        if (remainder >= min_free_block_size_)
        {
            next_free = get<Node>(get<std::byte>(current) + total);
            next_free->size = remainder;
            next_free->next = current->next;
        }
        else
        {
            total = current->size;
            next_free = current->next;
        }

        if (previous == nullptr)
        {
            head_ = next_free;
        }
        else
        {
            previous->next = next_free;
        }

        header = get<Header>(current);
        header->size = total;

        return get<std::byte>(header) + sizeof(Header);
    }

    void deallocate(void* pointer) override
    {
        Header* header = nullptr;
        Node* node = nullptr;
        Node* previous = nullptr;
        Node* current = nullptr;

        if (pointer == nullptr)
        {
            return;
        }

        header = get<Header>(get<std::byte>(pointer) - sizeof(Header));
        node = get<Node>(header);
        node->size = header->size;
        node->next = nullptr;

        previous = nullptr;
        current = head_;

        while (current != nullptr && current < node)
        {
            previous = current;
            current = current->next;
        }

        node->next = current;

        if (previous == nullptr)
        {
            head_ = node;
        }
        else
        {
            previous->next = node;
        }

        merge(previous, node);
    }

private:
    struct alignas(std::max_align_t) Node
    {
        std::size_t size = 0U;
        Node* next = nullptr;
    };

    struct alignas(std::max_align_t) Header
    {
        std::size_t size = 0U;
    };

    static constexpr std::size_t min_allocated_block_size_ =
        ((sizeof(Header) + 1U + default_alignment - 1U) /
         default_alignment) *
        default_alignment;

    static constexpr std::size_t min_free_block_size_ =
        sizeof(Node) > min_allocated_block_size_
            ? sizeof(Node)
            : min_allocated_block_size_;

    std::pair<Node*, Node*> find(const std::size_t size) const
    {
        Node* current = nullptr;
        Node* previous = nullptr;

        previous = nullptr;
        current = head_;

        while (current != nullptr)
        {
            if (current->size >= size)
            {
                return std::make_pair(current, previous);
            }

            previous = current;
            current = current->next;
        }

        return std::make_pair(nullptr, nullptr);
    }

    void merge(Node* previous, Node* node)
    {
        Node* next = nullptr;

        next = node->next;

        if (next != nullptr &&
            get<std::byte>(node) + node->size == get<std::byte>(next))
        {
            node->size += next->size;
            node->next = next->next;
        }

        if (previous != nullptr &&
            get<std::byte>(previous) + previous->size == get<std::byte>(node))
        {
            previous->size += node->size;
            previous->next = node->next;
        }
    }

    std::size_t size_;
    void* begin_;
    Node* head_;
};

// 09.10 - Tests

void test_linear_allocator()
{
    constexpr std::size_t kBufferSize = 1024U;
    constexpr std::size_t kOne = 1U;
    constexpr std::size_t kTwo = 2U;
    constexpr std::size_t kFour = 4U;
    constexpr std::size_t kEight = 8U;

    LinearAllocator allocator(kBufferSize);
    void* first = nullptr;
    void* second = nullptr;
    void* third = nullptr;
    void* fourth = nullptr;

    first = allocator.allocate(kOne, kOne);
    second = allocator.allocate(kTwo, kTwo);
    third = allocator.allocate(kFour, kFour);
    fourth = allocator.allocate(kEight, kEight);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(third != nullptr);
    assert(fourth != nullptr);

    assert(is_aligned(first, kOne));
    assert(is_aligned(second, kTwo));
    assert(is_aligned(third, kFour));
    assert(is_aligned(fourth, kEight));

    assert(first < second);
    assert(second < third);
    assert(third < fourth);
}

void test_stack_allocator()
{
    constexpr std::size_t kBufferSize = 1024U;
    constexpr std::size_t kFour = 4U;
    constexpr std::size_t kEight = 8U;

    StackAllocator allocator(kBufferSize);
    void* first = nullptr;
    void* second = nullptr;
    void* third = nullptr;

    first = allocator.allocate(kFour, kFour);
    second = allocator.allocate(kEight, kEight);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(is_aligned(first, kFour));
    assert(is_aligned(second, kEight));

    allocator.deallocate(second);
    third = allocator.allocate(kEight, kEight);

    assert(third == second);

    allocator.deallocate(third);
    allocator.deallocate(first);
}

void test_list_allocator()
{
    constexpr std::size_t kPageSize = 32U;
    constexpr std::size_t kStep = 8U;

    ListAllocator allocator(kPageSize, kStep);
    void* first = nullptr;
    void* second = nullptr;
    void* third = nullptr;
    void* fourth = nullptr;
    void* reused = nullptr;

    first = allocator.allocate(kStep);
    second = allocator.allocate(kStep);
    third = allocator.allocate(kStep);
    fourth = allocator.allocate(kStep);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(third != nullptr);
    assert(fourth != nullptr);

    allocator.deallocate(second);
    allocator.deallocate(third);

    reused = allocator.allocate(kStep);
    assert(reused == third);
}

void test_free_list_allocator()
{
    constexpr std::size_t kBufferSize = 1024U;
    constexpr std::size_t kSmall = 16U;
    constexpr std::size_t kLarge = 32U;

    FreeListAllocator allocator(kBufferSize);
    void* first = nullptr;
    void* second = nullptr;
    void* third = nullptr;
    void* fourth = nullptr;
    void* merged = nullptr;

    first = allocator.allocate(kSmall);
    second = allocator.allocate(kSmall);
    third = allocator.allocate(kSmall);
    fourth = allocator.allocate(kSmall);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(third != nullptr);
    assert(fourth != nullptr);

    allocator.deallocate(second);
    allocator.deallocate(first);

    merged = allocator.allocate(kLarge);
    assert(merged == first);
}

void test_polymorphic_allocators()
{
    constexpr std::size_t kPoolSize = 1024U;
    constexpr std::size_t kPageSize = 64U;
    constexpr std::size_t kStep = 16U;
    constexpr std::size_t kRequest = 16U;

    std::vector<std::unique_ptr<Allocator>> allocators = {};
    std::vector<void*> pointers = {};
    std::size_t i = 0U;

    allocators.push_back(std::make_unique<LinearAllocator>(kPoolSize));
    allocators.push_back(std::make_unique<StackAllocator>(kPoolSize));
    allocators.push_back(std::make_unique<ListAllocator>(kPageSize, kStep));
    allocators.push_back(std::make_unique<FreeListAllocator>(kPoolSize));

    pointers.resize(allocators.size(), nullptr);

    for (i = 0U; i < allocators.size(); ++i)
    {
        pointers[i] = allocators[i]->allocate(kRequest);
        assert(pointers[i] != nullptr);
    }

    for (i = 0U; i < allocators.size(); ++i)
    {
        allocators[i]->deallocate(pointers[i]);
    }
}

int main()
{
    constexpr std::size_t kPoolSize = 256U;
    constexpr std::size_t kPageSize = 64U;
    constexpr std::size_t kStep = 16U;
    constexpr std::size_t kRequest = 16U;

    std::vector<std::unique_ptr<Allocator>> allocators = {};
    std::vector<const char*> names = {};
    std::vector<void*> pointers = {};
    std::size_t i = 0U;

    test_linear_allocator();
    test_stack_allocator();
    test_list_allocator();
    test_free_list_allocator();
    test_polymorphic_allocators();

    allocators.push_back(std::make_unique<LinearAllocator>(kPoolSize));
    allocators.push_back(std::make_unique<StackAllocator>(kPoolSize));
    allocators.push_back(std::make_unique<ListAllocator>(kPageSize, kStep));
    allocators.push_back(std::make_unique<FreeListAllocator>(kPoolSize));

    names.push_back("09.28 LinearAllocator");
    names.push_back("09.29 StackAllocator");
    names.push_back("09.30 ListAllocator");
    names.push_back("09.31 FreeListAllocator");

    pointers.resize(allocators.size(), nullptr);

    std::cout << "Polymorphic allocators demo:\n";

    for (i = 0U; i < allocators.size(); ++i)
    {
        pointers[i] = allocators[i]->allocate(kRequest);
        assert(pointers[i] != nullptr);

        std::cout << names[i] << " -> " << pointers[i] << '\n';
    }

    for (i = 0U; i < allocators.size(); ++i)
    {
        allocators[i]->deallocate(pointers[i]);
    }

    return 0;
}