#include <algorithm>
#include <cassert>
#include <cstddef>
#include <new>
#include <random>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

class Allocator
{
public:
    enum class FitPolicy
    {
        first_fit,
        best_fit
    };

    explicit Allocator(
        std::size_t size,
        const FitPolicy policy = FitPolicy::first_fit)
        : size_(align_up(size)), begin_(nullptr), head_(nullptr), policy_(policy)
    {
        assert(size_ >= kMinFreeBlockSize);

        begin_ = ::operator new(size_, std::align_val_t(kAlignment));
        head_ = node_from(begin_);
        head_->size = size_;
        head_->next = nullptr;
    }

    ~Allocator()
    {
        ::operator delete(begin_, std::align_val_t(kAlignment));
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    void* allocate(const std::size_t size)
    {
        std::size_t total = 0U;
        std::size_t remainder = 0U;
        std::pair<Node*, Node*> found = std::make_pair(nullptr, nullptr);
        Node* current = nullptr;
        Node* previous = nullptr;
        Node* next_free = nullptr;
        Header* header = nullptr;

        if (size == 0U)
        {
            return nullptr;
        }

        total = align_up(sizeof(Header) + size);
        found = find(total);
        current = found.first;
        previous = found.second;

        if (current == nullptr)
        {
            return nullptr;
        }

        remainder = current->size - total;

        if (remainder >= kMinFreeBlockSize)
        {
            next_free = node_from(byte_ptr(current) + total);
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

        header = header_from(current);
        header->size = total;

        return byte_ptr(header) + sizeof(Header);
    }

    void deallocate(void* pointer)
    {
        Header* header = nullptr;
        Node* node = nullptr;
        Node* previous = nullptr;
        Node* current = nullptr;

        if (pointer == nullptr)
        {
            return;
        }

        header = header_from(byte_ptr(pointer) - sizeof(Header));
        node = node_from(header);
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

    std::size_t get_largest_free_block_size() const
    {
        std::size_t result = 0U;
        Node* current = nullptr;

        current = head_;
        while (current != nullptr)
        {
            result = std::max(result, current->size);
            current = current->next;
        }

        return result;
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

    static constexpr std::size_t kAlignment = alignof(std::max_align_t);

    static constexpr std::size_t align_up(const std::size_t size)
    {
        return ((size + kAlignment - 1U) / kAlignment) * kAlignment;
    }

    static constexpr std::size_t kMinAllocatedBlockSize =
        ((sizeof(Header) + 1U + kAlignment - 1U) / kAlignment) * kAlignment;

    static constexpr std::size_t kMinFreeBlockSize =
        sizeof(Node) > kMinAllocatedBlockSize ? sizeof(Node) : kMinAllocatedBlockSize;

    std::byte* byte_ptr(void* pointer) const
    {
        return static_cast<std::byte*>(pointer);
    }

    const std::byte* byte_ptr(const void* pointer) const
    {
        return static_cast<const std::byte*>(pointer);
    }

    Node* node_from(void* pointer) const
    {
        return static_cast<Node*>(pointer);
    }

    Header* header_from(void* pointer) const
    {
        return static_cast<Header*>(pointer);
    }

    std::pair<Node*, Node*> find(const std::size_t size) const
    {
        if (policy_ == FitPolicy::first_fit)
        {
            return find_first(size);
        }

        return find_best(size);
    }

    std::pair<Node*, Node*> find_first(const std::size_t size) const
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

    std::pair<Node*, Node*> find_best(const std::size_t size) const
    {
        Node* current = nullptr;
        Node* previous = nullptr;
        Node* best = nullptr;
        Node* best_previous = nullptr;
        std::size_t best_size = 0U;

        previous = nullptr;
        current = head_;
        best = nullptr;
        best_previous = nullptr;
        best_size = 0U;

        while (current != nullptr)
        {
            if (current->size >= size)
            {
                if (best == nullptr || current->size < best_size)
                {
                    best = current;
                    best_previous = previous;
                    best_size = current->size;

                    if (best_size == size)
                    {
                        break;
                    }
                }
            }

            previous = current;
            current = current->next;
        }

        return std::make_pair(best, best_previous);
    }

    void merge(Node* previous, Node* node)
    {
        Node* next = nullptr;

        next = node->next;

        if (next != nullptr && byte_ptr(node) + node->size == byte_ptr(next))
        {
            node->size += next->size;
            node->next = next->next;
        }

        if (previous != nullptr &&
            byte_ptr(previous) + previous->size == byte_ptr(node))
        {
            previous->size += node->size;
            previous->next = node->next;
        }
    }

    std::size_t size_;
    void* begin_;
    Node* head_;
    FitPolicy policy_;
};

std::vector<std::size_t> make_sizes(
    const std::size_t count,
    const std::size_t minimum,
    const std::size_t maximum,
    const unsigned seed)
{
    std::mt19937 engine(seed);
    std::uniform_int_distribution<std::size_t> distribution(minimum, maximum);
    std::vector<std::size_t> sizes(count, 0U);
    std::size_t i = 0U;

    for (i = 0U; i < count; ++i)
    {
        sizes[i] = distribution(engine);
    }

    return sizes;
}

void test_reuse_after_merge()
{
    constexpr std::size_t kPoolSize = 1024U;
    constexpr std::size_t kSmallSize = 64U;
    constexpr std::size_t kLargeSize = 128U;

    Allocator allocator(kPoolSize, Allocator::FitPolicy::first_fit);
    void* first = nullptr;
    void* second = nullptr;
    void* third = nullptr;
    void* merged = nullptr;

    first = allocator.allocate(kSmallSize);
    second = allocator.allocate(kSmallSize);
    third = allocator.allocate(kSmallSize);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(third != nullptr);

    allocator.deallocate(second);
    allocator.deallocate(first);

    merged = allocator.allocate(kLargeSize);
    assert(merged == first);
}

void prepare_divergence_layout(
    Allocator& allocator,
    void*& first_large,
    void*& separator_one,
    void*& second_large,
    void*& separator_two)
{
    constexpr std::size_t kLargeOne = 1000U;
    constexpr std::size_t kSeparator = 16U;
    constexpr std::size_t kLargeTwo = 250U;

    first_large = allocator.allocate(kLargeOne);
    separator_one = allocator.allocate(kSeparator);
    second_large = allocator.allocate(kLargeTwo);
    separator_two = allocator.allocate(kSeparator);

    assert(first_large != nullptr);
    assert(separator_one != nullptr);
    assert(second_large != nullptr);
    assert(separator_two != nullptr);

    allocator.deallocate(first_large);
    allocator.deallocate(second_large);
}

void test_strategy_difference()
{
    constexpr std::size_t kPoolSize = 2048U;
    constexpr std::size_t kRequest = 150U;

    Allocator first_fit(kPoolSize, Allocator::FitPolicy::first_fit);
    Allocator best_fit(kPoolSize, Allocator::FitPolicy::best_fit);

    void* first_large = nullptr;
    void* separator_one = nullptr;
    void* second_large = nullptr;
    void* separator_two = nullptr;
    void* request_first = nullptr;
    void* request_best = nullptr;

    prepare_divergence_layout(
        first_fit,
        first_large,
        separator_one,
        second_large,
        separator_two);

    prepare_divergence_layout(
        best_fit,
        first_large,
        separator_one,
        second_large,
        separator_two);

    request_first = first_fit.allocate(kRequest);
    request_best = best_fit.allocate(kRequest);

    assert(request_first != nullptr);
    assert(request_best != nullptr);
    assert(first_fit.get_largest_free_block_size() <
           best_fit.get_largest_free_block_size());
}

void benchmark_allocator(
    benchmark::State& state,
    const Allocator::FitPolicy policy)
{
    constexpr std::size_t kPoolSize = 128U * 1024U * 1024U;
    constexpr std::size_t kCount = 1024U;
    constexpr std::size_t kReleaseStep = 4U;
    constexpr std::size_t kMinSize = 1024U;
    constexpr std::size_t kMaxSize = 64U * 1024U;
    const std::vector<std::size_t> initial_sizes =
        make_sizes(kCount, kMinSize, kMaxSize, 42U);
    const std::vector<std::size_t> refill_sizes =
        make_sizes(kCount, kMinSize, kMaxSize, 4242U);
    std::vector<void*> pointers(kCount, nullptr);
    std::size_t i = 0U;

    for (auto _ : state)
    {
        state.PauseTiming();
        Allocator allocator(kPoolSize, policy);
        std::fill(pointers.begin(), pointers.end(), nullptr);
        state.ResumeTiming();

        for (i = 0U; i < kCount; ++i)
        {
            pointers[i] = allocator.allocate(initial_sizes[i]);

            if (pointers[i] == nullptr)
            {
                state.SkipWithError("Initial allocation failed.");
                return;
            }

            benchmark::DoNotOptimize(pointers[i]);
        }

        for (i = 0U; i < kCount; i += kReleaseStep)
        {
            allocator.deallocate(pointers[i]);
            pointers[i] = nullptr;
        }

        for (i = 0U; i < kCount; i += kReleaseStep)
        {
            pointers[i] = allocator.allocate(refill_sizes[i]);

            if (pointers[i] == nullptr)
            {
                state.SkipWithError("Refill allocation failed.");
                return;
            }

            benchmark::DoNotOptimize(pointers[i]);
        }

        for (i = 0U; i < kCount; ++i)
        {
            allocator.deallocate(pointers[i]);
            pointers[i] = nullptr;
        }

        benchmark::ClobberMemory();
        (void)_;
    }
}

static void BM_AllocatorFirstFit(benchmark::State& state)
{
    benchmark_allocator(state, Allocator::FitPolicy::first_fit);
}

static void BM_AllocatorBestFit(benchmark::State& state)
{
    benchmark_allocator(state, Allocator::FitPolicy::best_fit);
}

BENCHMARK(BM_AllocatorFirstFit)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_AllocatorBestFit)->Unit(benchmark::kMillisecond);

int main(int argc, char** argv)
{
    test_reuse_after_merge();
    test_strategy_difference();

    benchmark::Initialize(&argc, argv);

    if (benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}

//// g++ -std=c++20 -Wall -Wextra -Wpedantic 09_09.cpp -lbenchmark -pthread -o 09_09
//// g++ -std=c++20 -Wall -Wextra -Wpedantic 09_09.cpp -Iinclude ./build/src/libbenchmark.a -pthread -o 09_09
/*
Load Average: 0.64, 1.53, 0.95
***WARNING*** ASLR is enabled, the results may have unreproducible noise in them.
---------------------------------------------------------------
Benchmark                     Time             CPU   Iterations
---------------------------------------------------------------
BM_AllocatorFirstFit       2.01 ms         2.01 ms          305
BM_AllocatorBestFit        2.25 ms         2.25 ms          296


First fit was faster than best fit in the same benchmark (~2.01 ms vs ~2.25 ms), 
because it usually stops searching earlier, while best fit spends more time choosing 
a tighter block.
*/