#include <cassert>
#include <type_traits>

class Wrapper {
public:
    using Function = Wrapper (*)() noexcept;

    Wrapper(Function function) noexcept
        : function_(function)
    {
    }

    operator Function() const noexcept
    {
        return function_;
    }

private:
    Function function_;
};

Wrapper test() noexcept
{
    return test;
}

int main()
{
    Wrapper f1 = test();
    Wrapper f2 = (*f1)();
    Wrapper f3 = (*f2)();
    Wrapper::Function p1 = static_cast<Wrapper::Function>(f1);
    Wrapper::Function p2 = static_cast<Wrapper::Function>(f2);
    Wrapper::Function p3 = static_cast<Wrapper::Function>(f3);

    static_assert(std::is_same_v<decltype(test()), Wrapper>);
    static_assert(std::is_same_v<decltype(&test), Wrapper::Function>);

    assert(p1 == &test);
    assert(p2 == &test);
    assert(p3 == &test);

    return 0;
}