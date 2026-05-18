#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

struct EntryInfo
{
    char type;
    std::string permissions;
    std::uintmax_t size;
    std::string name;

    bool operator==(EntryInfo const& other) const = default;
};

char make_type(std::filesystem::file_status const& status)
{
    char result = '?';

    if (std::filesystem::is_directory(status)) {
        result = 'd';
    } else if (std::filesystem::is_regular_file(status)) {
        result = 'f';
    } else if (std::filesystem::is_symlink(status)) {
        result = 'l';
    }

    return result;
}

std::string make_permissions(std::filesystem::perms permissions)
{
    const char yes_read = 'r';
    const char yes_write = 'w';
    const char yes_execute = 'x';
    const char no_permission = '-';
    std::string result = {};

    result.push_back((permissions & std::filesystem::perms::owner_read)
            == std::filesystem::perms::none
        ? no_permission
        : yes_read);
    result.push_back((permissions & std::filesystem::perms::owner_write)
            == std::filesystem::perms::none
        ? no_permission
        : yes_write);
    result.push_back((permissions & std::filesystem::perms::owner_exec)
            == std::filesystem::perms::none
        ? no_permission
        : yes_execute);

    return result;
}

std::uintmax_t regular_file_size(std::filesystem::path const& path)
{
    std::error_code error = {};
    const std::uintmax_t failed_size = 0U;
    const std::uintmax_t result = std::filesystem::file_size(path, error);

    if (error) {
        return failed_size;
    }

    return result;
}

std::uintmax_t directory_size(std::filesystem::path const& path)
{
    std::uintmax_t result = 0U;
    std::error_code error = {};
    std::filesystem::recursive_directory_iterator iterator(path, error);
    const std::filesystem::recursive_directory_iterator end = {};

    while (!error && iterator != end) {
        if (std::filesystem::is_regular_file(iterator->status(error))) {
            result += regular_file_size(iterator->path());
        }

        iterator.increment(error);
    }

    return result;
}

std::uintmax_t entry_size(std::filesystem::directory_entry const& entry)
{
    std::uintmax_t result = 0U;
    std::error_code error = {};
    const std::filesystem::file_status status = entry.status(error);

    if (!error && std::filesystem::is_regular_file(status)) {
        result = regular_file_size(entry.path());
    } else if (!error && std::filesystem::is_directory(status)) {
        result = directory_size(entry.path());
    }

    return result;
}

EntryInfo make_entry_info(std::filesystem::directory_entry const& entry)
{
    std::error_code error = {};
    const std::filesystem::file_status status = entry.status(error);
    EntryInfo info = {'?', "---", 0U, ""};

    if (!error) {
        info.type = make_type(status);
        info.permissions = make_permissions(status.permissions());
        info.size = entry_size(entry);
    }

    info.name = entry.path().filename().string();

    return info;
}

std::vector<EntryInfo> list_matching_entries(std::filesystem::path const& path,
    std::regex const& pattern)
{
    std::vector<EntryInfo> entries = {};
    std::error_code error = {};
    std::filesystem::directory_iterator iterator(path, error);
    const std::filesystem::directory_iterator end = {};
    std::string name = {};

    if (error || !std::filesystem::exists(path)
        || !std::filesystem::is_directory(path)) {
        return entries;
    }

    while (iterator != end) {
        name = iterator->path().filename().string();

        if (std::regex_search(name, pattern)) {
            entries.push_back(make_entry_info(*iterator));
        }

        iterator.increment(error);

        if (error) {
            break;
        }
    }

    std::ranges::sort(entries, {}, &EntryInfo::name);

    return entries;
}

void show(std::filesystem::path const& path, std::regex const& pattern)
{
    const int size_width = 10;
    const std::vector<EntryInfo> entries = list_matching_entries(path, pattern);
    std::size_t index = 0U;

    for (index = 0U; index < entries.size(); ++index) {
        std::cout << entries[index].type << " | "
                  << entries[index].permissions << " | "
                  << std::setw(size_width) << entries[index].size << " | "
                  << entries[index].name << '\n';
    }
}

void write_file(std::filesystem::path const& path, std::string const& text)
{
    std::fstream stream(path, std::ios::out);

    assert(stream.is_open());

    stream << text;
}

std::vector<std::string> names_from_entries(std::vector<EntryInfo> const& entries)
{
    std::vector<std::string> names = {};
    std::size_t index = 0U;

    for (index = 0U; index < entries.size(); ++index) {
        names.push_back(entries[index].name);
    }

    return names;
}

void test_filtering()
{
    const std::filesystem::path directory = "test_13_04_directory";
    const std::filesystem::path file_txt = directory / "alpha.txt";
    const std::filesystem::path file_cpp = directory / "beta.cpp";
    const std::filesystem::path file_md = directory / "notes.md";
    const std::filesystem::path subdirectory = directory / "docs";
    const std::regex pattern("([.]cpp$)|(^docs$)",
        std::regex_constants::extended);
    const std::vector<std::string> expected = {"beta.cpp", "docs"};
    std::vector<EntryInfo> entries = {};
    std::vector<std::string> names = {};

    std::filesystem::remove_all(directory);
    std::filesystem::create_directory(directory);
    std::filesystem::create_directory(subdirectory);
    write_file(file_txt, "text");
    write_file(file_cpp, "code");
    write_file(file_md, "notes");

    entries = list_matching_entries(directory, pattern);
    names = names_from_entries(entries);

    assert(names == expected);

    std::filesystem::remove_all(directory);
}

void test_permissions()
{
    const std::string permissions = make_permissions(
        std::filesystem::perms::owner_read
        | std::filesystem::perms::owner_write);

    assert(permissions == "rw-");
}

void test_all()
{
    test_filtering();
    test_permissions();
}

int main(int argc, char* argv[])
{
    const std::filesystem::path path = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::current_path();
    std::string pattern_text = {};
    std::regex pattern;

    test_all();

    std::cout << "Self-check: OK\n";
    std::cout << "Directory: " << path.string() << '\n';
    std::cout << "Enter regular expression: ";

    if (!std::getline(std::cin, pattern_text)) {
        return 0;
    }

    try {
        pattern = std::regex(pattern_text, std::regex_constants::extended);
        show(path, pattern);
    } catch (std::regex_error const& error) {
        std::cerr << "Regex error: " << error.what() << '\n';

        return 1;
    }

    return 0;
}


// regular expressions examples:
// ^13_      <-> ls -1 | grep -E '13_'
// [.]cpp$   <-> ls -1 | grep -E '[.]cpp$' 
//(output)|(source)