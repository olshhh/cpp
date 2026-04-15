#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
constexpr std::size_t kAlgorithmCount = 9U;
constexpr std::size_t kDefaultStringCount = 100000U;
constexpr std::size_t kDefaultStep = 5000U;
constexpr std::size_t kMinLength = 8U;
constexpr std::size_t kMaxLength = 20U;
constexpr unsigned int kSeed = 42U;

constexpr std::uint32_t byte_value(const char value)
{
    return static_cast<std::uint32_t>(static_cast<unsigned char>(value));
}

constexpr std::uint32_t rs_hash(const std::string_view text)
{
    const std::uint32_t multiplier = 378551U;
    std::uint32_t a = 63689U;
    std::uint32_t hash = 0U;
    for (const char c : text)
    {
        hash = hash * a + byte_value(c);
        a *= multiplier;
    }

    return hash;
}

constexpr std::uint32_t js_hash(const std::string_view text)
{
    std::uint32_t hash = 1315423911U;
    for (const char c : text)
    {
        hash ^= ((hash << 5U) + byte_value(c) + (hash >> 2U));
    }

    return hash;
}

constexpr std::uint32_t pjw_hash(const std::string_view text)
{
    constexpr std::uint32_t bits = 32U;
    constexpr std::uint32_t quarter = (bits * 3U) / 4U;
    constexpr std::uint32_t eighth = bits / 8U;
    constexpr std::uint32_t high_bits =
        std::numeric_limits<std::uint32_t>::max() << (bits - eighth);
    std::uint32_t hash = 0U;
    std::uint32_t test = 0U;
    for (const char c : text)
    {
        hash = (hash << eighth) + byte_value(c);
        test = hash & high_bits;

        if (test != 0U)
        {
            hash = (hash ^ (test >> quarter)) & (~high_bits);
        }
    }

    return hash;
}

constexpr std::uint32_t elf_hash(const std::string_view text)
{
    std::uint32_t hash = 0U;
    std::uint32_t high = 0U;
    for (const char c : text)
    {
        hash = (hash << 4U) + byte_value(c);
        high = hash & 0xF0000000U;

        if (high != 0U)
        {
            hash ^= (high >> 24U);
        }

        hash &= ~high;
    }

    return hash;
}

constexpr std::uint32_t bkdr_hash(const std::string_view text)
{
    constexpr std::uint32_t seed = 131U;
    std::uint32_t hash = 0U;
    for (const char c : text)
    {
        hash = hash * seed + byte_value(c);
    }

    return hash;
}

constexpr std::uint32_t sdbm_hash(const std::string_view text)
{
    std::uint32_t hash = 0U;
    for (const char c : text)
    {
        hash = byte_value(c) + (hash << 6U) + (hash << 16U) - hash;
    }

    return hash;
}

constexpr std::uint32_t djb_hash(const std::string_view text)
{
    std::uint32_t hash = 5381U;
    for (const char c : text)
    {
        hash = ((hash << 5U) + hash) + byte_value(c);
    }

    return hash;
}

constexpr std::uint32_t dek_hash(const std::string_view text)
{
    std::uint32_t hash = static_cast<std::uint32_t>(text.size());
    for (const char c : text)
    {
        hash = ((hash << 5U) ^ (hash >> 27U)) ^ byte_value(c);
    }

    return hash;
}

constexpr std::uint32_t ap_hash(const std::string_view text)
{
    std::uint32_t hash = 0xAAAAAAAAU;
    std::size_t i = 0U;
    std::uint32_t value = 0U;

    for (i = 0U; i < text.size(); ++i)
    {
        value = byte_value(text[i]);

        if ((i & 1U) == 0U)
        {
            hash ^= ((hash << 7U) ^ (value * (hash >> 3U)));
        }
        else
        {
            hash ^= ~((hash << 11U) + (value ^ (hash >> 5U)));
        }
    }

    return hash;
}

using HashFunction = std::uint32_t (*)(std::string_view);

struct HashSpec
{
    std::string_view name;
    HashFunction function;
};

using Series = std::array<std::vector<std::size_t>, kAlgorithmCount>;

struct Experiment
{
    std::vector<std::size_t> counts;
    Series collisions;
};

constexpr std::array<HashSpec, kAlgorithmCount> kAlgorithms = {{
    {"RSHash", rs_hash},
    {"JSHash", js_hash},
    {"PJWHash", pjw_hash},
    {"ELFHash", elf_hash},
    {"BKDRHash", bkdr_hash},
    {"SDBMHash", sdbm_hash},
    {"DJBHash", djb_hash},
    {"DEKHash", dek_hash},
    {"APHash", ap_hash}
}};

constexpr std::array<std::string_view, kAlgorithmCount> kColors = {{
    "#d62728",
    "#1f77b4",
    "#2ca02c",
    "#9467bd",
    "#ff7f0e",
    "#8c564b",
    "#e377c2",
    "#7f7f7f",
    "#17becf"
}};

constexpr std::array<std::uint32_t, kAlgorithmCount> kExpectedAbc = {{
    2969643692U,
    446371745U,
    26499U,
    26499U,
    1677554U,
    807794786U,
    193485963U,
    2083U,
    633864072U
}};

std::vector<std::string> generate_unique_strings(const std::size_t count)
{
    std::vector<std::string> strings = {};
    std::unordered_set<std::string> seen = {};
    std::mt19937 engine = {};
    std::uniform_int_distribution<int> char_dist = {};
    std::uniform_int_distribution<int> len_dist = {};
    std::size_t length = 0U;
    std::size_t i = 0U;
    std::string text = {};

    engine.seed(kSeed);
    char_dist = std::uniform_int_distribution<int>('a', 'z');
    len_dist = std::uniform_int_distribution<int>(
        static_cast<int>(kMinLength),
        static_cast<int>(kMaxLength));

    strings.reserve(count);
    seen.reserve(count * 2U);

    while (strings.size() < count)
    {
        length = static_cast<std::size_t>(len_dist(engine));
        text.clear();
        text.reserve(length);

        for (i = 0U; i < length; ++i)
        {
            text.push_back(static_cast<char>(char_dist(engine)));
        }

        if (seen.insert(text).second)
        {
            strings.push_back(text);
        }
    }

    return strings;
}

Experiment run_experiment(const std::vector<std::string>& strings, const std::size_t step)
{
    Experiment experiment = {};
    std::unordered_set<std::uint32_t> seen = {};
    std::size_t algorithm_index = 0U;
    std::size_t sample_index = 0U;
    std::size_t i = 0U;
    std::size_t collisions = 0U;
    std::size_t count = step;
    std::uint32_t value = 0U;

    assert(step > 0U);
    experiment.counts.reserve((strings.size() + step - 1U) / step);

    while (count < strings.size())
    {
        experiment.counts.push_back(count);
        count += step;
    }

    experiment.counts.push_back(strings.size());

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        seen.clear();
        seen.reserve(strings.size() * 2U);
        collisions = 0U;
        experiment.collisions[algorithm_index].reserve(experiment.counts.size());
        sample_index = 0U;

        for (i = 0U; i < strings.size(); ++i)
        {
            value = kAlgorithms[algorithm_index].function(strings[i]);

            if (!seen.insert(value).second)
            {
                ++collisions;
            }

            if (i + 1U == experiment.counts[sample_index])
            {
                experiment.collisions[algorithm_index].push_back(collisions);
                ++sample_index;

                if (sample_index == experiment.counts.size())
                {
                    break;
                }
            }
        }
    }

    return experiment;
}

void write_csv(const Experiment& experiment, const std::string_view file_name)
{
    std::ofstream output = {};
    std::size_t row = 0U;
    std::size_t algorithm_index = 0U;

    output.open(std::string(file_name));
    assert(output.good());

    output << "count";

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        output << ',' << kAlgorithms[algorithm_index].name;
    }

    output << '\n';

    for (row = 0U; row < experiment.counts.size(); ++row)
    {
        output << experiment.counts[row];

        for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
        {
            output << ',' << experiment.collisions[algorithm_index][row];
        }

        output << '\n';
    }
}

void write_svg(const Experiment& experiment, const std::string_view file_name)
{
    constexpr int width = 1400;
    constexpr int height = 900;
    constexpr int left = 90;
    constexpr int right = 340;
    constexpr int top = 60;
    constexpr int bottom = 80;
    const int plot_width = width - left - right;
    const int plot_height = height - top - bottom;
    std::ofstream output = {};
    std::size_t max_count = 0U;
    std::size_t max_collision = 0U;
    std::size_t algorithm_index = 0U;
    std::size_t point_index = 0U;
    double x = 0.0;
    double y = 0.0;

    output.open(std::string(file_name));
    assert(output.good());

    max_count = experiment.counts.back();

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        if (!experiment.collisions[algorithm_index].empty())
        {
            max_collision = std::max(
                max_collision,
                experiment.collisions[algorithm_index].back());
        }
    }

    if (max_collision == 0U)
    {
        max_collision = 1U;
    }

    output << "<svg xmlns='http://www.w3.org/2000/svg' width='" << width
           << "' height='" << height << "'>\n";
    output << "<rect width='100%' height='100%' fill='white'/>\n";
    output << "<text x='" << width / 2
           << "' y='30' text-anchor='middle' font-size='24'>"
           << "Hash collisions vs number of hashed strings</text>\n";
    output << "<line x1='" << left << "' y1='" << height - bottom
           << "' x2='" << width - right << "' y2='" << height - bottom
           << "' stroke='black'/>\n";
    output << "<line x1='" << left << "' y1='" << top
           << "' x2='" << left << "' y2='" << height - bottom
           << "' stroke='black'/>\n";

    for (point_index = 0U; point_index <= 10U; ++point_index)
    {
        x = static_cast<double>(left) +
            static_cast<double>(plot_width) * static_cast<double>(point_index) / 10.0;
        y = static_cast<double>(top) +
            static_cast<double>(plot_height) * static_cast<double>(point_index) / 10.0;

        output << "<line x1='" << x << "' y1='" << top
               << "' x2='" << x << "' y2='" << height - bottom
               << "' stroke='#dddddd'/>\n";
        output << "<line x1='" << left << "' y1='" << y
               << "' x2='" << width - right << "' y2='" << y
               << "' stroke='#dddddd'/>\n";
        output << "<text x='" << x << "' y='" << height - bottom + 25
               << "' text-anchor='middle' font-size='12'>"
               << static_cast<std::size_t>(max_count * point_index / 10U)
               << "</text>\n";
        output << "<text x='" << left - 10 << "' y='" << y + 4.0
               << "' text-anchor='end' font-size='12'>"
               << static_cast<std::size_t>(max_collision * (10U - point_index) / 10U)
               << "</text>\n";
    }

    output << "<text x='" << width / 2 << "' y='" << height - 20
           << "' text-anchor='middle' font-size='16'>Number of hashed strings</text>\n";
    output << "<text x='25' y='" << height / 2
           << "' transform='rotate(-90 25," << height / 2
           << ")' text-anchor='middle' font-size='16'>Collisions</text>\n";

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        output << "<polyline fill='none' stroke='" << kColors[algorithm_index]
               << "' stroke-width='2' points='";

        for (point_index = 0U; point_index < experiment.counts.size(); ++point_index)
        {
            x = static_cast<double>(left) +
                static_cast<double>(plot_width) *
                static_cast<double>(experiment.counts[point_index]) /
                static_cast<double>(max_count);
            y = static_cast<double>(height - bottom) -
                static_cast<double>(plot_height) *
                static_cast<double>(experiment.collisions[algorithm_index][point_index]) /
                static_cast<double>(max_collision);

            output << x << ',' << y;

            if (point_index + 1U != experiment.counts.size())
            {
                output << ' ';
            }
        }

        output << "'/>\n";
        output << "<rect x='" << width - right + 20 << "' y='"
               << top + 25 * algorithm_index
               << "' width='14' height='14' fill='" << kColors[algorithm_index]
               << "'/>\n";
        output << "<text x='" << width - right + 42 << "' y='"
               << top + 12 + 25 * algorithm_index
               << "' font-size='14'>" << kAlgorithms[algorithm_index].name
               << "</text>\n";
    }

    output << "</svg>\n";
}

std::vector<std::pair<std::string_view, std::size_t>> build_ranking(const Experiment& experiment)
{
    std::vector<std::pair<std::string_view, std::size_t>> ranking = {};
    std::size_t algorithm_index = 0U;

    ranking.reserve(kAlgorithmCount);

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        ranking.push_back({
            kAlgorithms[algorithm_index].name,
            experiment.collisions[algorithm_index].back()});
    }

    std::sort(
        ranking.begin(),
        ranking.end(),
        [](const auto& left, const auto& right)
        {
            if (left.second != right.second)
            {
                return left.second < right.second;
            }

            return left.first < right.first;
        });

    return ranking;
}

void print_summary(const std::vector<std::pair<std::string_view, std::size_t>>& ranking)
{
    std::size_t i = 0U;

    std::cout << "Final collisions:\n";

    for (i = 0U; i < ranking.size(); ++i)
    {
        std::cout << ranking[i].first << ": " << ranking[i].second << '\n';
    }

    std::cout << "Best: " << ranking.front().first
              << " (" << ranking.front().second << ")\n";
    std::cout << "Worst: " << ranking.back().first
              << " (" << ranking.back().second << ")\n";
}

void run_tests()
{
    const std::string_view sample = "abc";
    std::vector<std::string> strings = {};
    Experiment experiment = {};
    std::unordered_set<std::string> unique_strings = {};
    std::size_t algorithm_index = 0U;
    std::size_t point_index = 0U;

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        assert(kAlgorithms[algorithm_index].function(sample) == kExpectedAbc[algorithm_index]);
    }

    assert(pjw_hash(sample) == elf_hash(sample));

    strings = generate_unique_strings(2048U);
    unique_strings.insert(strings.begin(), strings.end());
    assert(unique_strings.size() == strings.size());

    experiment = run_experiment(strings, 128U);
    assert(!experiment.counts.empty());

    for (algorithm_index = 0U; algorithm_index < kAlgorithmCount; ++algorithm_index)
    {
        for (point_index = 1U; point_index < experiment.counts.size(); ++point_index)
        {
            assert(
                experiment.collisions[algorithm_index][point_index - 1U] <=
                experiment.collisions[algorithm_index][point_index]);
        }
    }
}
} 

int main(int argc, char** argv)
{
    std::size_t string_count = kDefaultStringCount;
    std::size_t step = kDefaultStep;
    std::vector<std::string> strings = {};
    Experiment experiment = {};
    std::vector<std::pair<std::string_view, std::size_t>> ranking = {};

    run_tests();

    if (argc > 1)
    {
        string_count = static_cast<std::size_t>(std::stoull(argv[1]));
    }

    if (argc > 2)
    {
        step = static_cast<std::size_t>(std::stoull(argv[2]));
    }

    assert(string_count > 0U);
    assert(step > 0U);

    strings = generate_unique_strings(string_count);
    experiment = run_experiment(strings, step);
    ranking = build_ranking(experiment);

    write_csv(experiment, "hash_collisions.csv");
    write_svg(experiment, "hash_collisions.svg");
    print_summary(ranking);

    std::cout << "CSV: hash_collisions.csv\n";
    std::cout << "SVG: hash_collisions.svg\n";
}

//g++-11 -O3 -m32 -Wall -Wextra -Wpedantic -std=c++20 10_05_1.cpp -o 10_05_1
