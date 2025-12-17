#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Inserting only int values into a container via push_back; ignores other types.
template <typename Container>
void handle(Container& container, int value)
{
    container.push_back(value);
}

// Ignoring  any non-int argument types.
template <typename Container, typename T>
void handle(Container&, const T&)
{
    // Intentionally ignored.
}

// Inserting a variadic pack into a container, keeping only arguments of type int.
template <typename Container, typename... Args>
void insert_ints(Container& container, Args&&... args)
{
    (handle(container, std::forward<Args>(args)), ...);
}

int main()
{
    {
        std::vector<int> v;
        std::vector<int> expected;

        constexpr int kOne = 1;
        constexpr int kTwo = 2;
        constexpr int kThree = 3;
        constexpr int kFour = 4;
        constexpr int kFive = 5;

        expected = {kOne, kTwo, kThree, kFour, kFive};

        insert_ints(v,
                    kOne,
                    2.0,                    // ignored
                    std::string("x"),       // ignored
                    kTwo,
                    'A',                    // ignored (char is not int)
                    kThree,
                    7L,                     // ignored (long is not int)
                    kFour,
                    true,                   // ignored (bool is not int)
                    kFive);

        assert(v == expected);
    }

    {
        // example
        std::vector<int> v;

        int a = 0;
        int b = 0;
        int c = 0;

        std::cout << "Enter three integers: ";
        if (!(std::cin >> a >> b >> c))
        {
            std::cout << "Invalid input\n";
            return 0;
        }

        insert_ints(v, a, 1.5, b, "ignored", c);

        std::cout << "Inserted ints: ";
        for (std::size_t i = 0U; i < v.size(); ++i)
        {
            if (i != 0U)
            {
                std::cout << ' ';
            }
            std::cout << v[i];
        }
        std::cout << '\n';
    }

    return 0;
}
