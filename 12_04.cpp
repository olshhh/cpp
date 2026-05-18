#include <cassert>
#include <cstddef>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

struct EmailEntry
{
    std::string address;
    std::string domain;

    bool operator==(EmailEntry const& other) const = default;
};

std::vector<EmailEntry> extract_emails(std::string const& text)
{
    const std::string pattern_text =
        R"(([A-Za-z0-9._%+\-]+@(([A-Za-z0-9\-]+\.)+[A-Za-z]{2,})))";
    const auto flags = std::regex_constants::ECMAScript
        | std::regex_constants::icase;
    const std::regex pattern(pattern_text, flags);
    const std::size_t address_group = 1U;
    const std::size_t domain_group = 2U;
    std::vector<EmailEntry> entries = {};
    std::sregex_iterator iterator(text.begin(), text.end(), pattern);
    const std::sregex_iterator end = {};
    std::smatch match = {};

    for (; iterator != end; ++iterator) {
        match = *iterator;
        entries.push_back(
            EmailEntry{match.str(address_group), match.str(domain_group)});
    }

    return entries;
}

void test_multiple_emails()
{
    const std::string text = R"(Contact alice@example.com and bob.smith@sub.example.org.)";
    const std::vector<EmailEntry> expected = {
        {"alice@example.com", "example.com"},
        {"bob.smith@sub.example.org", "sub.example.org"}
    };
    const std::vector<EmailEntry> result = extract_emails(text);

    assert(result == expected);
}

void test_email_with_plus()
{
    const std::string text = R"(Use first.last+tag@mail.example.net, please.)";
    const std::vector<EmailEntry> expected = {
        {"first.last+tag@mail.example.net", "mail.example.net"}
    };
    const std::vector<EmailEntry> result = extract_emails(text);

    assert(result == expected);
}

void test_no_emails()
{
    const std::string text = R"(There are no email addresses here.)";
    const std::vector<EmailEntry> expected = {};
    const std::vector<EmailEntry> result = extract_emails(text);

    assert(result == expected);
}

void test_case_insensitive_domain()
{
    const std::string text = R"(Write to ADMIN@EXAMPLE.COM now.)";
    const std::vector<EmailEntry> expected = {
        {"ADMIN@EXAMPLE.COM", "EXAMPLE.COM"}
    };
    const std::vector<EmailEntry> result = extract_emails(text);

    assert(result == expected);
}

void test_all()
{
    test_multiple_emails();
    test_email_with_plus();
    test_no_emails();
    test_case_insensitive_domain();
}

void print_entries(std::vector<EmailEntry> const& entries)
{
    std::size_t index = 0U;

    for (index = 0U; index < entries.size(); ++index) {
        std::cout << "Email: " << entries[index].address
                  << ", domain: " << entries[index].domain << '\n';
    }
}

int main()
{
    std::string text = {};
    std::vector<EmailEntry> entries = {};

    test_all();
    std::cout << "Self-check: OK\n";

    std::cout << "Enter text: ";
    if (!std::getline(std::cin, text)) {
        return 0;
    }

    entries = extract_emails(text);
    print_entries(entries);

    return 0;
}