#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

class Entity
{
public:
    class Implementation;

    Entity() noexcept;
    ~Entity() noexcept;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

    void test() const;
    void set_value(int value) noexcept;
    int get_value() const noexcept;

    Implementation* get() noexcept;
    const Implementation* get() const noexcept;

private:
    static constexpr std::size_t kBufferSize = 16U;

    alignas(std::max_align_t) std::array<std::byte, kBufferSize> buffer_{};
};

class Entity::Implementation
{
public:
    Implementation() noexcept = default;

    explicit Implementation(const int new_value) noexcept
        : value(new_value) {
    }

    Implementation(Implementation&& other) noexcept = default;
    Implementation& operator=(Implementation&& other) noexcept = default;

    void test() const
    {
        std::cout << "Entity::test : value = " << value << '\n';
    }

    int value{0};
};

Entity::Entity() noexcept
{
    static_assert(sizeof(Implementation) <= kBufferSize);
    static_assert(alignof(Implementation) <= alignof(std::max_align_t));

    ::new (static_cast<void*>(buffer_.data())) Implementation();
}

Entity::~Entity() noexcept
{
    std::destroy_at(get());
}

Entity::Entity(Entity&& other) noexcept
    : Entity()
{
    *get() = std::move(*other.get());
}

Entity& Entity::operator=(Entity&& other) noexcept
{
    if (this != &other)
    {
        *get() = std::move(*other.get());
    }

    return *this;
}

void Entity::test() const
{
    get()->test();
}

void Entity::set_value(const int value) noexcept
{
    get()->value = value;
}

int Entity::get_value() const noexcept
{
    return get()->value;
}

Entity::Implementation* Entity::get() noexcept
{
    std::byte* raw = nullptr;
    Implementation* pointer = nullptr;

    raw = buffer_.data();
    pointer = std::bit_cast<Implementation*>(raw);

    return std::launder(pointer);
}

const Entity::Implementation* Entity::get() const noexcept
{
    const std::byte* raw = nullptr;
    const Implementation* pointer = nullptr;

    raw = buffer_.data();
    pointer = std::bit_cast<const Implementation*>(raw);

    return std::launder(pointer);
}

void test_default_construction()
{
    constexpr int kValue = 42;

    Entity entity;

    entity.set_value(kValue);

    assert(entity.get() != nullptr);
    assert(entity.get_value() == kValue);
}

void test_const_and_nonconst_get()
{
    constexpr int kValue = 777;

    Entity entity;
    Entity::Implementation* implementation = nullptr;
    const Entity& const_entity = entity;
    const Entity::Implementation* const_implementation = nullptr;

    implementation = entity.get();
    implementation->value = kValue;
    const_implementation = const_entity.get();

    assert(implementation != nullptr);
    assert(const_implementation != nullptr);
    assert(const_implementation->value == kValue);
}

void test_move_constructor()
{
    constexpr int kValue = 100;

    Entity source;
    Entity target(std::move(source));

    source.set_value(kValue);
    target = std::move(source);

    assert(target.get() != nullptr);
    assert(target.get_value() == kValue);
}

void test_move_assignment()
{
    constexpr int kValue = 55;

    Entity source;
    Entity target;

    source.set_value(kValue);
    target = std::move(source);

    assert(target.get() != nullptr);
    assert(target.get_value() == kValue);
}

void test_get_returns_stable_pointer()
{
    Entity entity;
    Entity::Implementation* first = nullptr;
    Entity::Implementation* second = nullptr;

    first = entity.get();
    second = entity.get();

    assert(first != nullptr);
    assert(first == second);
}

int main()
{
    constexpr int kDemoValue = 42;

    Entity demo;

    test_default_construction();
    test_const_and_nonconst_get();
    test_move_constructor();
    test_move_assignment();
    test_get_returns_stable_pointer();

    demo.set_value(kDemoValue);
    demo.test();

    return 0;
}