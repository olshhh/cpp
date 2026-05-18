#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using Interval = std::pair<std::size_t, std::size_t>;

bool is_space(char symbol)
{
    const bool result = symbol == ' ' || symbol == '\t' || symbol == '\n'
        || symbol == '\r' || symbol == '\f' || symbol == '\v';

    return result;
}

bool has_non_space(std::string_view line)
{
    bool result = false;
    std::size_t index = 0U;

    for (index = 0U; index < line.size(); ++index) {
        if (!is_space(line[index])) {
            result = true;
        }
    }

    return result;
}

std::string read_file(std::string const& path)
{
    std::fstream stream(path, std::ios::in);
    std::stringstream buffer;

    assert(stream.is_open());

    buffer << stream.rdbuf();

    return buffer.str();
}

void write_file(std::string const& path, std::string const& text)
{
    std::fstream stream(path, std::ios::out);

    assert(stream.is_open());

    stream << text;
}

std::size_t find_raw_string_end(std::string const& text, std::size_t position)
{
    const std::size_t delimiter_begin = position + 2U;
    std::size_t delimiter_end = delimiter_begin;
    std::size_t closing_position = std::string::npos;
    std::string delimiter = {};
    std::string closing = {};

    while (delimiter_end < text.size() && text[delimiter_end] != '('
        && text[delimiter_end] != '\n') {
        ++delimiter_end;
    }

    if (delimiter_end == text.size() || text[delimiter_end] != '(') {
        return std::string::npos;
    }

    delimiter = text.substr(delimiter_begin, delimiter_end - delimiter_begin);
    closing = ")" + delimiter + "\"";
    closing_position = text.find(closing, delimiter_end + 1U);

    if (closing_position == std::string::npos) {
        return text.size();
    }

    return closing_position + closing.size();
}

std::size_t skip_quoted_literal(std::string const& text, std::size_t position,
    char quote)
{
    std::size_t index = position + 1U;
    bool escaped = false;

    while (index < text.size()) {
        if (escaped) {
            escaped = false;
        } else if (text[index] == '\\') {
            escaped = true;
        } else if (text[index] == quote) {
            ++index;
            return index;
        }

        ++index;
    }

    return index;
}

std::string remove_comments(std::string const& text)
{
    const char slash = '/';
    const char star = '*';
    const char double_quote = '"';
    const char single_quote = '\'';
    std::string result = {};
    std::size_t index = 0U;
    std::size_t end = 0U;
    bool raw_string = false;

    while (index < text.size()) {
        raw_string = index + 1U < text.size() && text[index] == 'R'
            && text[index + 1U] == double_quote;

        if (raw_string) {
            end = find_raw_string_end(text, index);

            if (end != std::string::npos) {
                result.append(text.substr(index, end - index));
                index = end;
            } else {
                result.push_back(text[index]);
                ++index;
            }
        } else if (text[index] == double_quote || text[index] == single_quote) {
            end = skip_quoted_literal(text, index, text[index]);
            result.append(text.substr(index, end - index));
            index = end;
        } else if (index + 1U < text.size() && text[index] == slash
            && text[index + 1U] == slash) {
            index += 2U;

            while (index < text.size() && text[index] != '\n') {
                ++index;
            }
        } else if (index + 1U < text.size() && text[index] == slash
            && text[index + 1U] == star) {
            index += 2U;

            while (index + 1U < text.size()
                && !(text[index] == star && text[index + 1U] == slash)) {
                ++index;
            }

            if (index + 1U < text.size()) {
                index += 2U;
            }
        } else {
            result.push_back(text[index]);
            ++index;
        }
    }

    return result;
}

std::vector<Interval> find_raw_string_intervals(std::string const& text)
{
    const char double_quote = '"';
    const char single_quote = '\'';
    std::vector<Interval> intervals = {};
    std::size_t index = 0U;
    std::size_t end = 0U;
    bool raw_string = false;

    while (index < text.size()) {
        raw_string = index + 1U < text.size() && text[index] == 'R'
            && text[index + 1U] == double_quote;

        if (raw_string) {
            end = find_raw_string_end(text, index);

            if (end != std::string::npos) {
                intervals.push_back(Interval{index, end});
                index = end;
            } else {
                ++index;
            }
        } else if (text[index] == double_quote || text[index] == single_quote) {
            index = skip_quoted_literal(text, index, text[index]);
        } else {
            ++index;
        }
    }

    return intervals;
}

bool overlaps_interval(std::size_t begin, std::size_t end,
    std::vector<Interval> const& intervals)
{
    bool result = false;
    std::size_t index = 0U;

    for (index = 0U; index < intervals.size(); ++index) {
        if (intervals[index].first < end && begin < intervals[index].second) {
            result = true;
        }
    }

    return result;
}

std::string remove_empty_lines(std::string const& text)
{
    const std::vector<Interval> raw_intervals = find_raw_string_intervals(text);
    std::string result = {};
    std::size_t line_begin = 0U;
    std::size_t line_end = 0U;
    std::size_t line_stop = 0U;
    std::string_view line = {};
    bool protected_line = false;

    while (line_begin < text.size()) {
        line_end = text.find('\n', line_begin);

        if (line_end == std::string::npos) {
            line_end = text.size();
        }

        line_stop = line_end < text.size() ? line_end + 1U : line_end;
        line = std::string_view(text.data() + line_begin, line_end - line_begin);
        protected_line = overlaps_interval(line_begin, line_stop, raw_intervals);

        if (has_non_space(line) || protected_line) {
            result.append(text.substr(line_begin, line_stop - line_begin));
        }

        line_begin = line_stop;
    }

    return result;
}

std::string transform_text(std::string const& text)
{
    const std::string without_comments = remove_comments(text);
    const std::string without_empty_lines = remove_empty_lines(without_comments);

    return without_empty_lines;
}

void transform(std::string const& input_path, std::string const& output_path)
{
    const std::string source = read_file(input_path);
    const std::string output = transform_text(source);

    write_file(output_path, output);
}

void test_remove_empty_lines()
{
    const std::string source = "int a = 1;\n\n   \n\t\nint b = 2;\n";
    const std::string expected = "int a = 1;\nint b = 2;\n";
    const std::string result = remove_empty_lines(source);

    assert(result == expected);
}

void test_remove_comments()
{
    const std::string source =
        "int a = 1;// line comment\n"
        "int b = 2;/* block comment */\n"
        "const char* s = \"/* not comment */\";\n";
    const std::string expected =
        "int a = 1;\n"
        "int b = 2;\n"
        "const char* s = \"/* not comment */\";\n";
    const std::string result = remove_comments(source);

    assert(result == expected);
}

void test_raw_string_literal()
{
    const std::string source = R"cpp(int main()
{
    const char* text = R"raw(first

// not a comment
/* not a comment */
last)raw";

    int value = 1;// line comment
    /* block comment */

    return value;
}
)cpp";
    const std::string expected = R"cpp(int main()
{
    const char* text = R"raw(first

// not a comment
/* not a comment */
last)raw";
    int value = 1;
    return value;
}
)cpp";
    const std::string result = transform_text(source);

    assert(result == expected);
}

void test_file_transform()
{
    const std::string input_path = "test_source_13_03.cpp";
    const std::string output_path = "test_output_13_03.cpp";
    const std::string source =
        "int main()\n"
        "{\n"
        "    int x = 1;// comment\n"
        "\n"
        "    int y = 2;\n"
        "}\n";
    const std::string expected =
        "int main()\n"
        "{\n"
        "    int x = 1;\n"
        "    int y = 2;\n"
        "}\n";
    std::string result = {};

    write_file(input_path, source);
    transform(input_path, output_path);
    result = read_file(output_path);

    assert(result == expected);

    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
}

void test_all()
{
    test_remove_empty_lines();
    test_remove_comments();
    test_raw_string_literal();
    test_file_transform();
}

void create_demo_source(std::string const& path)
{
    const std::string source = R"cpp(#include <iostream>

int main()
{
    const char* text = R"raw(first

// not a comment
/* not a comment */
last)raw";

    int value = 1;// line comment

    std::cout << text << value << '\n';
}
)cpp";

    write_file(path, source);
}

int main(int argc, char* argv[])
{
    const std::string default_input_path = "source.cpp";
    const std::string default_output_path = "output.cpp";
    const std::string input_path = argc > 1 ? argv[1] : default_input_path;
    const std::string output_path = argc > 2 ? argv[2] : default_output_path;
    const bool create_demo = argc == 1
        && !std::filesystem::exists(default_input_path);

    test_all();

    if (create_demo) {
        create_demo_source(input_path);
    }

    transform(input_path, output_path);

    std::cout << "Self-check: OK\n";
    std::cout << "Input file: " << input_path << '\n';
    std::cout << "Output file: " << output_path << '\n';

    return 0;
}