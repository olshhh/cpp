#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

template <typename... Ts>
class Tuple;

template <>
class Tuple<>
{
public:
    constexpr Tuple() noexcept = default;

    constexpr std::size_t size() const noexcept
    {
        return 0U;
    }

    template <std::size_t I>
    constexpr auto get() const
    {
        static_assert(I != I, "Tuple<>: index out of range.");
        return 0;
    }
};

template <typename T, typename... Ts>
class Tuple<T, Ts...>
{
public:
    template <typename U, typename... Us>
    constexpr Tuple(U&& x, Us&&... xs) noexcept
        : m_head(std::forward<U>(x)),
          m_tail(std::forward<Us>(xs)...)
    {
    }

    constexpr std::size_t size() const noexcept
    {
        return 1U + sizeof...(Ts);
    }

    template <std::size_t I>
    constexpr auto get() const
    {
        if constexpr (I == 0U)
        {
            return m_head;
        }
        else
        {
            return m_tail.template get<I - 1U>();
        }
    }

private:
    T m_head;
    Tuple<Ts...> m_tail;
};

constexpr Tuple<int, long, char> ct_tuple(1, 2L, 'a');

static_assert(ct_tuple.size() == 3U);
static_assert(ct_tuple.get<0>() == 1);
static_assert(ct_tuple.get<1>() == 2L);
static_assert(ct_tuple.get<2>() == 'a');

constexpr Tuple<> empty_tuple{};
static_assert(empty_tuple.size() == 0U);

int main()
{
    Tuple<int, double, std::string> tuple(1, 2.0, "aaaaa");

    assert(tuple.size() == 3U);
    assert(tuple.get<0>() == 1);

    return 0;
}
