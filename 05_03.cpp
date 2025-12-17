#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

class GameLog
{
public:
    void add_line(const std::string& s)
    {
        m_text += s;
        m_text += '\n';
    }

    const std::string& str() const
    {
        return m_text;
    }

private:
    std::string m_text{};
};

enum class Team
{
    Red,
    Blue
};

static const char* team_to_cstr(Team t)
{
    const char* s = "Unknown";
    if (t == Team::Red)
    {
        s = "Red";
    }
    else if (t == Team::Blue)
    {
        s = "Blue";
    }
    return s;
}

class Entity
{
public:
    virtual ~Entity() = default;

    void take_turn(GameLog& log) const
    {
        log.add_line(std::string("TurnBegin: ") + type_name() + " " + label());

        on_begin_turn(log);
        on_act(log);
        on_end_turn(log);

        log.add_line(std::string("TurnEnd: ") + type_name() + " " + label());
    }

private:
    virtual void on_begin_turn(GameLog& log) const
    {
        (void)log;
    }

    virtual void on_act(GameLog& log) const = 0;

    virtual void on_end_turn(GameLog& log) const
    {
        (void)log;
    }

    virtual const char* type_name() const = 0;
    virtual const std::string& label() const = 0;
};

struct Stats
{
    int hp{};
    int attack{};
};

class Unit : public virtual Entity
{
public:
    Unit(std::string name, Team team, Stats stats)
        : m_name(std::move(name)), m_team(team), m_stats(stats)
    {
    }

    int hp() const { return m_stats.hp; }
    int attack() const { return m_stats.attack; }
    Team team() const { return m_team; }

private:
    void on_begin_turn(GameLog& log) const override
    {
        log.add_line(std::string("  Ready: ") + m_name + " team=" + team_to_cstr(m_team));
    }

    void on_act(GameLog& log) const override
    {
        log.add_line(std::string("  Act: ") + m_name +
                     " attack=" + std::to_string(m_stats.attack) +
                     " hp=" + std::to_string(m_stats.hp));
    }

    void on_end_turn(GameLog& log) const override
    {
        log.add_line(std::string("  Rest: ") + m_name);
    }

    const char* type_name() const override
    {
        return "Unit";
    }

    const std::string& label() const override
    {
        return m_name;
    }

private:
    std::string m_name{};
    Team m_team{Team::Red};
    Stats m_stats{};
};

class Group : public virtual Entity
{
public:
    explicit Group(std::string name) : m_name(std::move(name))
    {
    }

    void add(std::unique_ptr<Entity> e)
    {
        m_children.push_back(std::move(e));
    }

    std::size_t size() const
    {
        return m_children.size();
    }

private:
    void on_begin_turn(GameLog& log) const override
    {
        log.add_line(std::string("  GroupBegin: ") + m_name);
    }

    void on_act(GameLog& log) const override
    {
        std::size_t i = 0U;
        for (i = 0U; i < m_children.size(); ++i)
        {
            m_children[i]->take_turn(log);
        }
    }

    void on_end_turn(GameLog& log) const override
    {
        log.add_line(std::string("  GroupEnd: ") + m_name);
    }

    const char* type_name() const override
    {
        return "Group";
    }

    const std::string& label() const override
    {
        return m_name;
    }

private:
    std::string m_name{};
    std::vector<std::unique_ptr<Entity>> m_children{};
};

class UnitBuilder
{
public:
    UnitBuilder()
        : m_name("Unnamed"), m_team(Team::Red), m_hp(kDefaultHp), m_attack(kDefaultAttack)
    {
    }

    UnitBuilder& name(const std::string& v)
    {
        m_name = v;
        return *this;
    }

    UnitBuilder& team(Team v)
    {
        m_team = v;
        return *this;
    }

    UnitBuilder& hp(int v)
    {
        m_hp = v;
        return *this;
    }

    UnitBuilder& attack(int v)
    {
        m_attack = v;
        return *this;
    }

    std::unique_ptr<Unit> get() const
    {
        Stats s{};
        s.hp = m_hp;
        s.attack = m_attack;
        return std::make_unique<Unit>(m_name, m_team, s);
    }

private:
    static constexpr int kDefaultHp = 100;
    static constexpr int kDefaultAttack = 10;

    std::string m_name{};
    Team m_team{};
    int m_hp{};
    int m_attack{};
};

int main()
{
    // Self-check: Builder + Composite + Template Method together.
    {
        GameLog log{};

        UnitBuilder builder{};
        std::unique_ptr<Unit> u1 = builder.name("Ivan").team(Team::Red).hp(120).attack(15).get();
        std::unique_ptr<Unit> u2 = builder.name("Olga").team(Team::Blue).hp(90).attack(12).get();

        Group squad("Squad1");
        assert(squad.size() == 0U);

        squad.add(std::move(u1));
        squad.add(std::move(u2));
        assert(squad.size() == 2U);

        squad.take_turn(log);

        const std::string& out = log.str();

        assert(out.find("TurnBegin: Group Squad1\n") != std::string::npos);
        assert(out.find("TurnBegin: Unit Ivan\n") != std::string::npos);
        assert(out.find("  Act: Ivan attack=15 hp=120\n") != std::string::npos);
        assert(out.find("TurnBegin: Unit Olga\n") != std::string::npos);
        assert(out.find("  Act: Olga attack=12 hp=90\n") != std::string::npos);
        assert(out.find("TurnEnd: Group Squad1\n") != std::string::npos);
    }

    // Minimal demo output (avoid excessive I/O).
    {
        GameLog log{};

        UnitBuilder builder{};
        Group army("Army");
        army.add(builder.name("Scout").team(Team::Red).hp(50).attack(6).get());
        army.add(builder.name("Knight").team(Team::Red).hp(200).attack(25).get());

        army.take_turn(log);

        std::cout << "Self-check: OK\n";
        std::cout << "Demo log:\n" << log.str();
    }

    return 0;
}
