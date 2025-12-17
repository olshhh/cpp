#include <type_traits>

template <bool C, typename T, typename F>
struct conditional
{
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F>
{
    using type = F;
};

template <bool C, typename T, typename F>
using conditional_t = typename conditional<C, T, F>::type;

template <typename T>
struct add_const
{
    using type = const T;
};

template <typename T>
using add_const_t = typename add_const<T>::type;

template <typename T>
struct remove_const
{
    using type = T;
};

template <typename T>
struct remove_const<const T>
{
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

// -----------------------------
// helpers for decay
// -----------------------------
template <typename T>
struct remove_reference
{
    using type = T;
};

template <typename T>
struct remove_reference<T&>
{
    using type = T;
};

template <typename T>
struct remove_reference<T&&>
{
    using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

template <typename T>
struct remove_volatile
{
    using type = T;
};

template <typename T>
struct remove_volatile<volatile T>
{
    using type = T;
};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T>
struct remove_cv
{
    using type = remove_volatile_t<remove_const_t<T>>;
};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename T>
struct is_array : std::false_type
{
};

template <typename T, std::size_t N>
struct is_array<T[N]> : std::true_type
{
};

template <typename T>
struct is_array<T[]> : std::true_type
{
};

template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

template <typename T>
struct remove_extent
{
    using type = T;
};

template <typename T, std::size_t N>
struct remove_extent<T[N]>
{
    using type = T;
};

template <typename T>
struct remove_extent<T[]>
{
    using type = T;
};

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

template <typename T>
struct add_pointer
{
    using type = T*;
};

template <typename T>
using add_pointer_t = typename add_pointer<T>::type;

// Minimal is_function for decay (covers common cases)
template <typename T>
struct is_function : std::false_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...)> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args..., ...)> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) &> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const &> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile &> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile &> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) &&> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const &&> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile &&> : std::true_type
{
};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile &&> : std::true_type
{
};

template <typename T>
inline constexpr bool is_function_v = is_function<T>::value;

/////
// decay

template <typename T>
struct decay
{
private:
    using U = remove_reference_t<T>;

    using A = add_pointer_t<remove_extent_t<U>>;
    using F = add_pointer_t<U>;
    using C = remove_cv_t<U>;

public:
    using type = conditional_t<
        is_array_v<U>,
        A,
        conditional_t<is_function_v<U>, F, C>>;
};

template <typename T>
using decay_t = typename decay<T>::type;


template <typename T>
struct is_class
{
private:
    using U = remove_cv_t<T>;

    template <typename X>
    static char test(int X::*);

    template <typename X>
    static int test(...);

    static constexpr bool kHasMemberPointer = (sizeof(test<U>(nullptr)) == sizeof(char));

#if defined(__clang__) || defined(__GNUC__)
    static constexpr bool kIsUnion = __is_union(U);
#else
    static constexpr bool kIsUnion = false;
#endif

public:
    static constexpr bool value = kHasMemberPointer && !kIsUnion;
};

template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

/// tests
struct S
{
    int x;
};

class C
{
};

union U
{
    int a;
    double b;
};

static_assert(is_class_v<S>);
static_assert(is_class_v<C>);
static_assert(!is_class_v<int>);
static_assert(!is_class_v<int*>);
static_assert(!is_class_v<int&>);
static_assert(!is_class_v<U>);

static_assert(std::is_same_v<add_const_t<int>, const int>);
static_assert(std::is_same_v<add_const_t<const int>, const int>);

static_assert(std::is_same_v<remove_const_t<int>, int>);
static_assert(std::is_same_v<remove_const_t<const int>, int>);

static_assert(std::is_same_v<conditional_t<true, int, double>, int>);
static_assert(std::is_same_v<conditional_t<false, int, double>, double>);

static_assert(std::is_same_v<decay_t<int>, int>);
static_assert(std::is_same_v<decay_t<const int&>, int>);
static_assert(std::is_same_v<decay_t<int[3]>, int*>);

using fn_t = int(double);
static_assert(std::is_same_v<decay_t<fn_t>, int (*)(double)>);

int main()
{
    return 0;
}
