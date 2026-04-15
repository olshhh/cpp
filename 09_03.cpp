#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// 05.01 - Builder

namespace example_05_01 {

struct Entity {
    int x{0};
    int y{0};
};

class Builder {
public:
    virtual ~Builder() = default;

    std::unique_ptr<Entity> make_entity() {
        entity_ = std::make_unique<Entity>();
        set_x();
        set_y();
        return std::move(entity_);
    }

protected:
    virtual void set_x() = 0;
    virtual void set_y() = 0;

    std::unique_ptr<Entity> entity_{};
};

class BuilderClient final : public Builder {
protected:
    void set_x() override {
        entity_->x = 1;
    }

    void set_y() override {
        entity_->y = 1;
    }
};

class BuilderServer final : public Builder {
protected:
    void set_x() override {
        entity_->x = 1;
    }

    void set_y() override {
        entity_->y = 1;
    }
};

void test() {
    std::unique_ptr<Builder> builder = std::make_unique<BuilderClient>();
    std::unique_ptr<Entity> entity = nullptr;

    entity = builder->make_entity();
    assert(entity != nullptr);
    assert(entity->x == 1);
    assert(entity->y == 1);

    builder = std::make_unique<BuilderServer>();
    entity = builder->make_entity();
    assert(entity != nullptr);
    assert(entity->x == 1);
    assert(entity->y == 1);
}

void demo() {
    std::unique_ptr<Builder> builder = std::make_unique<BuilderClient>();
    std::unique_ptr<Entity> entity = nullptr;

    entity = builder->make_entity();
    std::cout << "05.01: Entity(" << entity->x << ", " << entity->y << ")\n";
}

}

// 05.03 - Abstract Factory

namespace example_05_03 {

class Entity {
public:
    virtual ~Entity() = default;
};

class Client final : public Entity {
};

class Server final : public Entity {
};

class Factory {
public:
    virtual ~Factory() = default;
    virtual std::unique_ptr<Entity> make_entity() const = 0;
};

class FactoryClient final : public Factory {
public:
    std::unique_ptr<Entity> make_entity() const override {
        return std::make_unique<Client>();
    }
};

class FactoryServer final : public Factory {
public:
    std::unique_ptr<Entity> make_entity() const override {
        return std::make_unique<Server>();
    }
};

void test() {
    std::unique_ptr<Factory> factory = std::make_unique<FactoryClient>();
    std::unique_ptr<Entity> entity = nullptr;

    entity = factory->make_entity();
    assert(entity != nullptr);
    assert(dynamic_cast<Client*>(entity.get()) != nullptr);

    factory = std::make_unique<FactoryServer>();
    entity = factory->make_entity();
    assert(entity != nullptr);
    assert(dynamic_cast<Server*>(entity.get()) != nullptr);
}

void demo() {
    std::unique_ptr<Factory> factory = std::make_unique<FactoryClient>();
    std::unique_ptr<Entity> entity = nullptr;

    entity = factory->make_entity();
    std::cout << "05.03: Client created = "
              << (dynamic_cast<Client*>(entity.get()) != nullptr)
              << '\n';
}

}

//05.04Prototype

namespace example_05_04 {

class Entity {
public:
    virtual ~Entity() = default;
    virtual std::unique_ptr<Entity> copy() const = 0;
};

class Client final : public Entity {
public:
    std::unique_ptr<Entity> copy() const override {
        return std::make_unique<Client>(*this);
    }
};

class Server final : public Entity {
public:
    std::unique_ptr<Entity> copy() const override {
        return std::make_unique<Server>(*this);
    }
};

class Prototype {
public:
    Prototype() {
        prototypes_.push_back(std::make_unique<Client>());
        prototypes_.push_back(std::make_unique<Server>());
    }

    std::unique_ptr<Entity> make_client() const {
        return prototypes_.at(0)->copy();
    }

    std::unique_ptr<Entity> make_server() const {
        return prototypes_.at(1)->copy();
    }

private:
    std::vector<std::unique_ptr<Entity>> prototypes_{};
};

void test() {
    Prototype prototype;
    std::unique_ptr<Entity> client = nullptr;
    std::unique_ptr<Entity> server = nullptr;

    client = prototype.make_client();
    server = prototype.make_server();

    assert(client != nullptr);
    assert(server != nullptr);
    assert(dynamic_cast<Client*>(client.get()) != nullptr);
    assert(dynamic_cast<Server*>(server.get()) != nullptr);
}

void demo() {
    Prototype prototype;
    std::unique_ptr<Entity> client = nullptr;
    std::unique_ptr<Entity> server = nullptr;

    client = prototype.make_client();
    server = prototype.make_server();

    std::cout << "05.04: client copy = "
              << (dynamic_cast<Client*>(client.get()) != nullptr)
              << ", server copy = "
              << (dynamic_cast<Server*>(server.get()) != nullptr)
              << '\n';
}

}

// 05.09 - Composite

namespace example_05_09 {

class Entity {
public:
    virtual ~Entity() = default;
    virtual int test() const = 0;
};

class Client final : public Entity {
public:
    int test() const override {
        return 1;
    }
};

class Server final : public Entity {
public:
    int test() const override {
        return 2;
    }
};

class Composite final : public Entity {
public:
    void add(std::unique_ptr<Entity> entity) {
        entities_.push_back(std::move(entity));
    }

    int test() const override {
        int sum = 0;

        for (const auto& entity : entities_) {
            if (entity != nullptr) {
                sum += entity->test();
            }
        }

        return sum;
    }

private:
    std::vector<std::unique_ptr<Entity>> entities_{};
};

std::unique_ptr<Entity> make_composite(
    const std::size_t client_count,
    const std::size_t server_count) {
    std::unique_ptr<Composite> composite = std::make_unique<Composite>();
    std::size_t i = 0U;

    for (i = 0U; i < client_count; ++i) {
        composite->add(std::make_unique<Client>());
    }

    for (i = 0U; i < server_count; ++i) {
        composite->add(std::make_unique<Server>());
    }

    return composite;
}

void test() {
    constexpr std::size_t kCompositeCount = 5U;
    constexpr std::size_t kClientCount = 1U;
    constexpr std::size_t kServerCount = 1U;
    constexpr int kExpected = 15;

    std::unique_ptr<Composite> composite = std::make_unique<Composite>();
    std::unique_ptr<Entity> entity = nullptr;
    std::size_t i = 0U;

    for (i = 0U; i < kCompositeCount; ++i) {
        composite->add(make_composite(kClientCount, kServerCount));
    }

    entity = std::move(composite);
    assert(entity != nullptr);
    assert(entity->test() == kExpected);
}

void demo() {
    constexpr std::size_t kCompositeCount = 5U;
    constexpr std::size_t kClientCount = 1U;
    constexpr std::size_t kServerCount = 1U;

    std::unique_ptr<Composite> composite = std::make_unique<Composite>();
    std::size_t i = 0U;

    for (i = 0U; i < kCompositeCount; ++i) {
        composite->add(make_composite(kClientCount, kServerCount));
    }

    std::cout << "05.09: total = " << composite->test() << '\n';
}

} 

// 05.13 - Observer

namespace example_05_13 {

class Observer {
public:
    virtual ~Observer() = default;
    virtual void test(int x) const = 0;
};

class Entity {
public:
    void add(std::shared_ptr<Observer> observer) {
        observers_.push_back(std::move(observer));
    }

    void set(const int x) {
        x_ = x;
        notify_all();
    }

private:
    void notify_all() const {
        for (const auto& observer : observers_) {
            if (observer != nullptr) {
                observer->test(x_);
            }
        }
    }

    int x_{0};
    std::vector<std::shared_ptr<Observer>> observers_{};
};

class Client final : public Observer {
public:
    explicit Client(std::ostream& out = std::cout) noexcept : out_(out) {
    }

    void test(const int x) const override {
        out_ << "Client::test : x = " << x << '\n';
    }

private:
    std::ostream& out_;
};

class Server final : public Observer {
public:
    explicit Server(std::ostream& out = std::cout) noexcept : out_(out) {
    }

    void test(const int x) const override {
        out_ << "Server::test : x = " << x << '\n';
    }

private:
    std::ostream& out_;
};

void test() {
    constexpr int kFirstValue = 1;
    constexpr int kSecondValue = 2;
    constexpr long long kSharedCountWithEntity = 2LL;
    constexpr long long kSharedCountOutsideOnly = 1LL;
    constexpr std::string_view kExpected =
        "Client::test : x = 1\n"
        "Server::test : x = 1\n"
        "Client::test : x = 2\n"
        "Server::test : x = 2\n";

    std::ostringstream out;
    std::shared_ptr<Observer> client = std::make_shared<Client>(out);
    std::shared_ptr<Observer> server = std::make_shared<Server>(out);

    {
        Entity entity;

        entity.add(client);
        entity.add(server);

        assert(client.use_count() == kSharedCountWithEntity);
        assert(server.use_count() == kSharedCountWithEntity);

        entity.set(kFirstValue);
        entity.set(kSecondValue);
    }

    assert(client.use_count() == kSharedCountOutsideOnly);
    assert(server.use_count() == kSharedCountOutsideOnly);
    assert(out.str() == kExpected);
}

void demo() {
    constexpr int kFirstValue = 1;
    constexpr int kSecondValue = 2;

    Entity entity;
    std::shared_ptr<Observer> client = std::make_shared<Client>();
    std::shared_ptr<Observer> server = std::make_shared<Server>();

    entity.add(client);
    entity.add(server);

    std::cout << "05.13:\n";
    entity.set(kFirstValue);
    entity.set(kSecondValue);
}

} 

void run_tests() {
    example_05_01::test();
    example_05_03::test();
    example_05_04::test();
    example_05_09::test();
    example_05_13::test();
}

void run_demos() {
    example_05_01::demo();
    example_05_03::demo();
    example_05_04::demo();
    example_05_09::demo();
    example_05_13::demo();
}

int main() {
    run_tests();
    run_demos();
    return 0;
}