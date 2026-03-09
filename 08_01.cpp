#include <cassert>
#include <cstring>
#include <iostream>
#include <type_traits>

class Entity_v1
{
public:
    explicit constexpr Entity_v1(int value = 0) noexcept : secret_(value)
    {
    }

    [[nodiscard]] constexpr int get() const noexcept
    {
        return secret_;
    }

private:
    int secret_ = 0;
};

class Entity_v2
{
public:
    int exposed = 0;
};

static_assert(std::is_standard_layout_v<Entity_v1>);
static_assert(std::is_standard_layout_v<Entity_v2>);
static_assert(std::is_trivially_copyable_v<Entity_v1>);
static_assert(std::is_trivially_copyable_v<Entity_v2>);
static_assert(sizeof(Entity_v1) == sizeof(Entity_v2));
static_assert(alignof(Entity_v1) == alignof(Entity_v2));
static_assert(sizeof(Entity_v1) == sizeof(int));

Entity_v2 &forge_view(Entity_v1 &entity) noexcept
{
    Entity_v2 *view = nullptr;

    view = reinterpret_cast<Entity_v2 *>(&entity);
    return *view;
}

const Entity_v2 &forge_view(const Entity_v1 &entity) noexcept
{
    const Entity_v2 *view = nullptr;

    view = reinterpret_cast<const Entity_v2 *>(&entity);
    return *view;
}

void hack_with_reinterpret_cast(Entity_v1 &entity, int new_value) noexcept
{
    forge_view(entity).exposed = new_value;
}

void hack_with_memcpy(Entity_v1 &entity, int new_value) noexcept
{
    void *destination = nullptr;
    const void *source = nullptr;

    destination = static_cast<void *>(&entity);
    source = static_cast<const void *>(&new_value);

    std::memcpy(destination, source, sizeof(new_value));
}

void test_initial_value()
{
    const Entity_v1 entity(10);

    assert(entity.get() == 10);
    assert(forge_view(entity).exposed == 10);
}

void test_reinterpret_cast_hack()
{
    Entity_v1 entity(7);

    assert(entity.get() == 7);

    hack_with_reinterpret_cast(entity, 25);

    assert(entity.get() == 25);
    assert(forge_view(entity).exposed == 25);
}

void test_memcpy_hack()
{
    Entity_v1 entity(-4);

    assert(entity.get() == -4);

    hack_with_memcpy(entity, 99);

    assert(entity.get() == 99);
    assert(forge_view(entity).exposed == 99);
}

void test_many_updates()
{
    Entity_v1 entity(0);

    hack_with_reinterpret_cast(entity, 100);
    assert(entity.get() == 100);

    hack_with_memcpy(entity, -200);
    assert(entity.get() == -200);

    hack_with_reinterpret_cast(entity, 300);
    assert(entity.get() == 300);

    hack_with_memcpy(entity, -400);
    assert(entity.get() == -400);
}

void test_independent_entities()
{
    Entity_v1 left(1);
    Entity_v1 right(2);

    hack_with_reinterpret_cast(left, 111);
    hack_with_memcpy(right, 222);

    assert(left.get() == 111);
    assert(right.get() == 222);
    assert(forge_view(left).exposed == 111);
    assert(forge_view(right).exposed == 222);
}

void run_tests()
{
    test_initial_value();
    test_reinterpret_cast_hack();
    test_memcpy_hack();
    test_many_updates();
    test_independent_entities();
}

int main()
{
    Entity_v1 first(123);
    Entity_v1 second(456);

    run_tests();

    std::cout << "Before reinterpret_cast hack: " << first.get() << '\n';
    hack_with_reinterpret_cast(first, 999);
    std::cout << "After reinterpret_cast hack:  " << first.get() << '\n';

    std::cout << "Before memcpy hack:           " << second.get() << '\n';
    hack_with_memcpy(second, -111);
    std::cout << "After memcpy hack:            " << second.get() << '\n';

    std::cout << "All tests passed\n";
    return 0;
}