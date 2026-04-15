#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>
#include <stdexcept>

template <typename D>
class Entity
{
public:
    static void reset_counters() noexcept
    {
        new_calls_ = 0U;
        delete_calls_ = 0U;
        new_array_calls_ = 0U;
        delete_array_calls_ = 0U;
        new_nothrow_calls_ = 0U;
        delete_nothrow_calls_ = 0U;
        new_array_nothrow_calls_ = 0U;
        delete_array_nothrow_calls_ = 0U;
    }

    static std::size_t new_calls() noexcept
    {
        return new_calls_;
    }

    static std::size_t delete_calls() noexcept
    {
        return delete_calls_;
    }

    static std::size_t new_array_calls() noexcept
    {
        return new_array_calls_;
    }

    static std::size_t delete_array_calls() noexcept
    {
        return delete_array_calls_;
    }

    static std::size_t new_nothrow_calls() noexcept
    {
        return new_nothrow_calls_;
    }

    static std::size_t delete_nothrow_calls() noexcept
    {
        return delete_nothrow_calls_;
    }

    static std::size_t new_array_nothrow_calls() noexcept
    {
        return new_array_nothrow_calls_;
    }

    static std::size_t delete_array_nothrow_calls() noexcept
    {
        return delete_array_nothrow_calls_;
    }

    static void* operator new(const std::size_t size)
    {
        ++new_calls_;
        std::cout << "Entity::operator new\n";
        return ::operator new(size);
    }

    static void operator delete(void* pointer) noexcept
    {
        ++delete_calls_;
        std::cout << "Entity::operator delete\n";
        ::operator delete(pointer);
    }

    static void* operator new[](const std::size_t size)
    {
        ++new_array_calls_;
        std::cout << "Entity::operator new[]\n";
        return ::operator new[](size);
    }

    static void operator delete[](void* pointer) noexcept
    {
        ++delete_array_calls_;
        std::cout << "Entity::operator delete[]\n";
        ::operator delete[](pointer);
    }

    static void* operator new(
        const std::size_t size,
        const std::nothrow_t& tag) noexcept
    {
        ++new_nothrow_calls_;
        std::cout << "Entity::operator new nothrow\n";
        return ::operator new(size, tag);
    }

    static void operator delete(
        void* pointer,
        const std::nothrow_t& tag) noexcept
    {
        ++delete_nothrow_calls_;
        std::cout << "Entity::operator delete nothrow\n";
        ::operator delete(pointer, tag);
    }

    static void* operator new[](
        const std::size_t size,
        const std::nothrow_t& tag) noexcept
    {
        ++new_array_nothrow_calls_;
        std::cout << "Entity::operator new[] nothrow\n";
        return ::operator new[](size, tag);
    }

    static void operator delete[](
        void* pointer,
        const std::nothrow_t& tag) noexcept
    {
        ++delete_array_nothrow_calls_;
        std::cout << "Entity::operator delete[] nothrow\n";
        ::operator delete[](pointer, tag);
    }

protected:
    Entity() = default;
    ~Entity() = default;

private:
    static inline std::size_t new_calls_ = 0U;
    static inline std::size_t delete_calls_ = 0U;
    static inline std::size_t new_array_calls_ = 0U;
    static inline std::size_t delete_array_calls_ = 0U;
    static inline std::size_t new_nothrow_calls_ = 0U;
    static inline std::size_t delete_nothrow_calls_ = 0U;
    static inline std::size_t new_array_nothrow_calls_ = 0U;
    static inline std::size_t delete_array_nothrow_calls_ = 0U;
};

class Client : private Entity<Client>
{
public:
    Client() noexcept
    {
        std::cout << "Client::Client\n";
    }

    ~Client() noexcept
    {
        std::cout << "Client::~Client\n";
    }

    using Entity<Client>::operator new;
    using Entity<Client>::operator delete;
    using Entity<Client>::operator new[];
    using Entity<Client>::operator delete[];

    using Entity<Client>::reset_counters;
    using Entity<Client>::new_calls;
    using Entity<Client>::delete_calls;
    using Entity<Client>::new_array_calls;
    using Entity<Client>::delete_array_calls;
    using Entity<Client>::new_nothrow_calls;
    using Entity<Client>::delete_nothrow_calls;
    using Entity<Client>::new_array_nothrow_calls;
    using Entity<Client>::delete_array_nothrow_calls;
};

class ThrowingClient : private Entity<ThrowingClient>
{
public:
    ThrowingClient()
    {
        throw std::runtime_error("Constructor failure");
    }

    using Entity<ThrowingClient>::operator new;
    using Entity<ThrowingClient>::operator delete;
    using Entity<ThrowingClient>::operator new[];
    using Entity<ThrowingClient>::operator delete[];

    using Entity<ThrowingClient>::reset_counters;
    using Entity<ThrowingClient>::new_calls;
    using Entity<ThrowingClient>::delete_calls;
    using Entity<ThrowingClient>::new_array_calls;
    using Entity<ThrowingClient>::delete_array_calls;
    using Entity<ThrowingClient>::new_nothrow_calls;
    using Entity<ThrowingClient>::delete_nothrow_calls;
    using Entity<ThrowingClient>::new_array_nothrow_calls;
    using Entity<ThrowingClient>::delete_array_nothrow_calls;
};

void test_regular_allocation()
{
    Client* pointer = nullptr;

    Client::reset_counters();

    pointer = new Client;
    assert(pointer != nullptr);
    assert(Client::new_calls() == 1U);
    assert(Client::delete_calls() == 0U);

    delete pointer;
    assert(Client::delete_calls() == 1U);
}

void test_array_allocation()
{
    constexpr int kCount = 3;
    Client* pointer = nullptr;

    Client::reset_counters();

    pointer = new Client[kCount];
    assert(pointer != nullptr);
    assert(Client::new_array_calls() == 1U);
    assert(Client::delete_array_calls() == 0U);

    delete[] pointer;
    assert(Client::delete_array_calls() == 1U);
}

void test_regular_nothrow_allocation()
{
    Client* pointer = nullptr;

    Client::reset_counters();

    pointer = new (std::nothrow) Client;
    assert(pointer != nullptr);
    assert(Client::new_nothrow_calls() == 1U);
    assert(Client::delete_nothrow_calls() == 0U);

    delete pointer;
    assert(Client::delete_calls() == 1U);
    assert(Client::delete_nothrow_calls() == 0U);
}

void test_array_nothrow_allocation()
{
    constexpr int kCount = 2;
    Client* pointer = nullptr;

    Client::reset_counters();

    pointer = new (std::nothrow) Client[kCount];
    assert(pointer != nullptr);
    assert(Client::new_array_nothrow_calls() == 1U);
    assert(Client::delete_array_nothrow_calls() == 0U);

    delete[] pointer;
    assert(Client::delete_array_calls() == 1U);
    assert(Client::delete_array_nothrow_calls() == 0U);
}

void test_nothrow_delete_on_constructor_exception()
{
    bool exception_was_caught = false;

    ThrowingClient::reset_counters();

    try
    {
        [[maybe_unused]] ThrowingClient* pointer =
            new (std::nothrow) ThrowingClient;
    }
    catch (const std::exception&)
    {
        exception_was_caught = true;
    }

    assert(exception_was_caught);
    assert(ThrowingClient::new_nothrow_calls() == 1U);
    assert(ThrowingClient::delete_nothrow_calls() == 1U);
}

int main()
{
    test_regular_allocation();
    test_array_allocation();
    test_regular_nothrow_allocation();
    test_array_nothrow_allocation();
    test_nothrow_delete_on_constructor_exception();

    std::cout << "Demo:\n";

    {
        Client* one = nullptr;
        Client* many = nullptr;

        one = new Client;
        delete one;

        many = new Client[2];
        delete[] many;
    }

    return 0;
}