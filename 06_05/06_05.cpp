#include <cassert>
#include <functional>
#include <iostream>
#include <string>

#include <boost/dll.hpp>

namespace
{
    using TestFunction = std::function<void()>;

    TestFunction import_test_function(const std::string& file_name)
    {
        boost::dll::fs::path executable_dir = boost::dll::program_location().parent_path();
        boost::dll::fs::path library_path = executable_dir / file_name;
        TestFunction function = boost::dll::import_symbol<void()>(library_path, "test");
        return function;
    }

    bool can_import_test(const std::string& file_name)
    {
        bool result = false;

        try
        {
            TestFunction function = import_test_function(file_name);
            result = static_cast<bool>(function);
        }
        catch (...)
        {
            result = false;
        }

        return result;
    }

    void run_tests()
    {
#ifdef _WIN32
        const std::string first_library = "version_a.dll";
        const std::string second_library = "version_b.dll";
#elif defined(__APPLE__)
        const std::string first_library = "libversion_a.dylib";
        const std::string second_library = "libversion_b.dylib";
#else
        const std::string first_library = "libversion_a.so";
        const std::string second_library = "libversion_b.so";
#endif

        assert(can_import_test(first_library));
        assert(can_import_test(second_library));
    }
}

int main()
{
    std::string file_name = "";

    run_tests();

    std::cout << "Enter shared library file name: ";
    std::cin >> file_name;

    try
    {
        TestFunction function = import_test_function(file_name);
        function();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Import error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}