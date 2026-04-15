#include <cassert>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

class Tracer {
public:
    explicit Tracer(
        const std::source_location location = std::source_location::current()) noexcept
        : location_(location) {
        Print(kEnterMessage);
    }

    ~Tracer() noexcept {
        Print(kExitMessage);
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

private:
    static constexpr std::string_view kEnterMessage = "Enter: ";
    static constexpr std::string_view kExitMessage = "Exit:  ";
    static constexpr std::string_view kBracketOpen = " [";
    static constexpr std::string_view kBracketClose = "]";
    static constexpr std::string_view kColon = ":";

    static std::string_view ShortFunctionName(
        const std::string_view function_name) noexcept {
        const std::size_t open_paren = function_name.find('(');
        const std::size_t prefix_end =
            open_paren == std::string_view::npos ? function_name.size() : open_paren;
        const std::string_view prefix = function_name.substr(0U, prefix_end);
        const std::size_t last_space = prefix.rfind(' ');
        const std::size_t name_begin =
            last_space == std::string_view::npos ? 0U : last_space + 1U;
        return prefix.substr(name_begin);
    }

    void Print(const std::string_view message) const noexcept {
        const std::string_view function_name =
            ShortFunctionName(location_.function_name());

        std::cout << message
                  << function_name
                  << kBracketOpen
                  << location_.file_name()
                  << kColon
                  << location_.line()
                  << kBracketClose
                  << '\n';
    }

    std::source_location location_;
};

#define TRACE_CONCAT_IMPL(left, right) left##right
#define TRACE_CONCAT(left, right) TRACE_CONCAT_IMPL(left, right)

#ifndef NDEBUG
#define trace() [[maybe_unused]] Tracer TRACE_CONCAT(tracer_, __LINE__)
#else
#define trace() static_cast<void>(0)
#endif

std::string CaptureStdout(void (*function)()) {
    std::ostringstream stream;
    std::streambuf* old_buffer = nullptr;
    std::string output;

    old_buffer = std::cout.rdbuf(stream.rdbuf());
    function();
    std::cout.rdbuf(old_buffer);
    output = stream.str();

    return output;
}

std::size_t CountOccurrences(
    const std::string_view text,
    const std::string_view token) {
    std::size_t count = 0U;
    std::size_t position = 0U;
    std::size_t found = 0U;

    while (true) {
        found = text.find(token, position);
        if (found == std::string_view::npos) {
            break;
        }
        ++count;
        position = found + token.size();
    }

    return count;
}

void ExampleLeaf() {
    trace();
}

void ExampleMiddle() {
    trace();
    ExampleLeaf();
}

void ExampleTop() {
    trace();
    ExampleMiddle();
}

#ifndef NDEBUG

void TestSingleFunctionTrace() {
    constexpr std::size_t kExpectedCount = 1U;

    const std::string output = CaptureStdout(ExampleLeaf);
    const std::size_t enter_count =
        CountOccurrences(output, "Enter: ExampleLeaf");
    const std::size_t exit_count =
        CountOccurrences(output, "Exit:  ExampleLeaf");

    assert(enter_count == kExpectedCount);
    assert(exit_count == kExpectedCount);
}

void TestNestedTraceOrder() {
    constexpr std::size_t kExpectedEnterCount = 3U;
    constexpr std::size_t kExpectedExitCount = 3U;

    const std::string output = CaptureStdout(ExampleTop);

    const std::size_t top_enter = output.find("Enter: ExampleTop");
    const std::size_t middle_enter = output.find("Enter: ExampleMiddle");
    const std::size_t leaf_enter = output.find("Enter: ExampleLeaf");
    const std::size_t leaf_exit = output.find("Exit:  ExampleLeaf");
    const std::size_t middle_exit = output.find("Exit:  ExampleMiddle");
    const std::size_t top_exit = output.find("Exit:  ExampleTop");
    const std::size_t total_enter = CountOccurrences(output, "Enter: ");
    const std::size_t total_exit = CountOccurrences(output, "Exit:  ");

    assert(top_enter != std::string::npos);
    assert(middle_enter != std::string::npos);
    assert(leaf_enter != std::string::npos);
    assert(leaf_exit != std::string::npos);
    assert(middle_exit != std::string::npos);
    assert(top_exit != std::string::npos);

    assert(total_enter == kExpectedEnterCount);
    assert(total_exit == kExpectedExitCount);

    assert(top_enter < middle_enter);
    assert(middle_enter < leaf_enter);
    assert(leaf_enter < leaf_exit);
    assert(leaf_exit < middle_exit);
    assert(middle_exit < top_exit);
}

#else

void TestTracingDisabled() {
    const std::string output = CaptureStdout(ExampleTop);
    assert(output.empty());
}

#endif

void RunTests() {
#ifndef NDEBUG
    TestSingleFunctionTrace();
    TestNestedTraceOrder();
#else
    TestTracingDisabled();
#endif
}

int main() {
    RunTests();

#ifndef NDEBUG
    std::cout << "Demo:\n";
    ExampleTop();
#else
    std::cout << "Tracing is disabled because NDEBUG is defined.\n";
#endif

    return 0;
}