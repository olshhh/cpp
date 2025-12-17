#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <iostream>

template <typename Impl>
class Entity : public Impl
{
public:
    Entity() = default;

    void test() const
    {
        Impl::test();
    }
};

class Client
{
public:
    void test() const
    {
        std::cout << "Client::test\n";
    }
};

class Server
{
public:
    void test() const
    {
        std::cout << "Server::test\n";
    }
};

template <typename E>
static std::string capture_test_output(E const& e)
{
    std::ostringstream out;
    std::streambuf* old_buf = std::cout.rdbuf();

    std::cout.rdbuf(out.rdbuf());
    e.test();
    std::cout.rdbuf(old_buf);

    return out.str();
}

template <typename E>
void test_entity(E const& e)
{
    e.test();
}

int main()
{
    {
        Entity<Client> client;
        const std::string got = capture_test_output(client);
        const std::string expected = "Client::test\n";
        assert(got == expected);
    }

    {
        Entity<Server> server;
        const std::string got = capture_test_output(server);
        const std::string expected = "Server::test\n";
        assert(got == expected);
    }

    {
        Entity<Client> client;
        test_entity(client);
        assert(true);
    }

    return 0;
}
