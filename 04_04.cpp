#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>

template <typename T>
class Vector
{
public:
    Vector() : m_array(nullptr), m_size(0U), m_capacity(0U) {}

    Vector(std::initializer_list<T> list)
        : m_array(nullptr), m_size(list.size()), m_capacity(list.size())
    {
        if (m_capacity > 0U)
        {
            m_array = allocate(m_capacity);
            std::copy(list.begin(), list.end(), m_array);
        }
    }

    Vector(Vector const& other)
        : m_array(nullptr), m_size(other.m_size), m_capacity(other.m_size)
    {
        if (m_capacity > 0U)
        {
            m_array = allocate(m_capacity);
            std::copy(other.m_array, other.m_array + other.m_size, m_array);
        }
    }

    Vector(Vector&& other)
        : m_array(other.m_array), m_size(other.m_size), m_capacity(other.m_capacity)
    {
        other.m_array = nullptr;
        other.m_size = 0U;
        other.m_capacity = 0U;
    }

    ~Vector()
    {
        delete[] m_array;
    }

    Vector& operator=(Vector other)
    {
        swap(other);
        return *this;
    }

    void swap(Vector& other)
    {
        using std::swap;
        swap(m_array, other.m_array);
        swap(m_size, other.m_size);
        swap(m_capacity, other.m_capacity);
    }

    std::size_t size() const { return m_size; }

    std::size_t capacity() const { return m_capacity; }

    bool empty() const { return m_size == 0U; }

    void clear() { m_size = 0U; }

    void push_back(T value)
    {
        if (m_size == m_capacity)
        {
            grow_for_push();
        }

        m_array[m_size] = value;
        ++m_size;
    }

    T& operator[](std::size_t index)
    {
        assert(index < m_size);
        return m_array[index];
    }

    T const& operator[](std::size_t index) const
    {
        assert(index < m_size);
        return m_array[index];
    }

private:
    static constexpr std::size_t kInitialCapacity = 1U;
    static constexpr std::size_t kGrowthFactor = 2U;

    static T* allocate(std::size_t count)
    {
        T* ptr = nullptr;
        ptr = (count == 0U) ? nullptr : new T[count]{};
        return ptr;
    }

    void reallocate_to(std::size_t new_capacity)
    {
        assert(new_capacity >= m_size);

        T* new_array = nullptr;

        new_array = allocate(new_capacity);
        if (m_array != nullptr)
        {
            std::copy(m_array, m_array + m_size, new_array);
        }

        delete[] m_array;
        m_array = new_array;
        m_capacity = new_capacity;
    }

    void grow_for_push()
    {
        std::size_t new_capacity = 0U;

        new_capacity = (m_capacity == 0U) ? kInitialCapacity : (m_capacity * kGrowthFactor);
        reallocate_to(new_capacity);
    }

private:
    T* m_array;
    std::size_t m_size;
    std::size_t m_capacity;
};

template <typename T>
void swap(Vector<T>& lhs, Vector<T>& rhs)
{
    lhs.swap(rhs);
}

int main()
{
    // Tests with int.
    {
        Vector<int> v0;
        assert(v0.empty());
        assert(v0.size() == 0U);
        assert(v0.capacity() == 0U);

        Vector<int> v1 = {1, 2, 3};
        assert(!v1.empty());
        assert(v1.size() == 3U);
        assert(v1.capacity() >= 3U);
        assert(v1[0] == 1 && v1[1] == 2 && v1[2] == 3);

        Vector<int> v2 = v1;
        assert(v2.size() == v1.size());
        assert(v2[1] == 2);

        Vector<int> v3;
        v3 = v1;
        assert(v3.size() == 3U);
        assert(v3[2] == 3);

        Vector<int> v4 = std::move(v3);
        assert(v4.size() == 3U);
        assert(v4[0] == 1 && v4[2] == 3);

        Vector<int> v;
        constexpr std::size_t kN = 10U;
        std::size_t i = 0U;

        for (i = 0U; i < kN; ++i)
        {
            v.push_back(static_cast<int>(i));
        }

        assert(v.size() == kN);
        assert(v.capacity() >= v.size());

        for (i = 0U; i < kN; ++i)
        {
            assert(v[i] == static_cast<int>(i));
        }

        {
            std::size_t cap_before = 0U;
            cap_before = v.capacity();

            v.clear();
            assert(v.size() == 0U);
            assert(v.capacity() == cap_before);
            assert(v.empty());
        }
    }

    // Tests with std::string.
    {
        Vector<std::string> s1 = {"pear", "apple", "banana"};
        assert(!s1.empty());
        assert(s1.size() == 3U);
        assert(s1[0] == "pear");
        assert(s1[1] == "apple");
        assert(s1[2] == "banana");

        Vector<std::string> s2 = s1;
        assert(s2.size() == s1.size());
        assert(s2[2] == "banana");

        Vector<std::string> s3;
        s3 = s1;
        assert(s3.size() == 3U);
        assert(s3[1] == "apple");

        Vector<std::string> s4 = std::move(s3);
        assert(s4.size() == 3U);
        assert(s4[0] == "pear");

        s4.push_back(std::string("apricot"));
        assert(s4.size() == 4U);
        assert(s4[3] == "apricot");
    }

    std::cout << "Self-check: OK\n";

    // Demo: interactive input.
    {
        Vector<int> demo;

        std::size_t count = 0U;
        std::size_t i = 0U;

        std::cout << "Enter how many integers you want to store: ";
        if (!(std::cin >> count) || count == 0U)
        {
            std::cout << "Invalid count\n";
            return 0;
        }

        std::cout << "Enter " << count << " integers:\n";
        for (i = 0U; i < count; ++i)
        {
            int value = 0;
            if (!(std::cin >> value))
            {
                std::cout << "Invalid input\n";
                return 0;
            }
            demo.push_back(value);
        }

        std::cout << "Stored values: ";
        for (i = 0U; i < demo.size(); ++i)
        {
            std::cout << demo[i] << (i + 1U == demo.size() ? '\n' : ' ');
        }
    }

    return 0;
}
