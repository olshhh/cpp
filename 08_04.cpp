#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <utility>

namespace
{
    constexpr char kFirstLetter = 'a';
    constexpr char kLastLetter = 'z';
    constexpr double kMutationProbability = 0.05;
    constexpr std::size_t kCopiesPerGeneration = 100;
    constexpr std::size_t kTargetSize = 23u;
    constexpr char kIterationSeparator[] = " : ";

    const std::string kTarget = "methinksitislikeaweasel";

    auto random_letter(std::default_random_engine & engine) -> char
    {
        std::uniform_int_distribution<int> letter_dist(static_cast<int>(kFirstLetter),
                                                       static_cast<int>(kLastLetter));
        int value = 0;

        value = letter_dist(engine);

        return static_cast<char>(value);
    }

    auto random_string(std::default_random_engine & engine, std::size_t size) -> std::string
    {
        std::string result(size, kFirstLetter);
        std::size_t i = 0;

        for (i = 0; i < size; ++i)
        {
            result[i] = random_letter(engine);
        }

        return result;
    }

    auto distance_to_target(const std::string & lhs, const std::string & rhs) -> std::size_t
    {
        std::size_t distance = 0;
        std::size_t i = 0;

        assert(lhs.size() == rhs.size());

        for (i = 0; i < lhs.size(); ++i)
        {
            distance += static_cast<std::size_t>(lhs[i] != rhs[i]);
        }

        return distance;
    }

    auto mutate(const std::string & source, std::default_random_engine & engine) -> std::string
    {
        std::uniform_real_distribution<double> probability_dist(0.0, 1.0);
        std::string result(source);
        std::size_t i = 0;

        for (i = 0; i < result.size(); ++i)
        {
            if (probability_dist(engine) < kMutationProbability)
            {
                result[i] = random_letter(engine);
            }
        }

        return result;
    }

    auto next_generation(const std::string & parent,
                         const std::string & target,
                         std::default_random_engine & engine) -> std::pair<std::string, std::size_t>
    {
        std::string best = parent;
        std::string candidate;
        std::size_t best_distance = target.size();
        std::size_t candidate_distance = 0;
        std::size_t i = 0;

        for (i = 0; i < kCopiesPerGeneration; ++i)
        {
            candidate = mutate(parent, engine);
            candidate_distance = distance_to_target(candidate, target);

            if (candidate_distance < best_distance)
            {
                best = candidate;
                best_distance = candidate_distance;
            }
        }

        return {best, best_distance};
    }

    void run_tests()
    {
        std::default_random_engine engine(12345u);
        std::string source = "aaaaaaaaaaaaaaaaaaaaaaa";
        std::string mutated;
        std::string random;
        std::pair<std::string, std::size_t> best;

        assert(kTarget.size() == kTargetSize);

        assert(distance_to_target(kTarget, kTarget) == 0u);
        assert(distance_to_target("abc", "abc") == 0u);
        assert(distance_to_target("abc", "axc") == 1u);
        assert(distance_to_target("abc", "xyz") == 3u);

        random = random_string(engine, kTargetSize);
        assert(random.size() == kTargetSize);
        assert(std::all_of(random.begin(), random.end(), [](char ch)
        {
            return ch >= kFirstLetter && ch <= kLastLetter;
        }));

        mutated = mutate(source, engine);
        assert(mutated.size() == source.size());
        assert(std::all_of(mutated.begin(), mutated.end(), [](char ch)
        {
            return ch >= kFirstLetter && ch <= kLastLetter;
        }));

        best = next_generation(source, kTarget, engine);
        assert(best.first.size() == kTargetSize);
        assert(best.second <= kTargetSize);
        assert(best.second == distance_to_target(best.first, kTarget));
    }
}

int main()
{
    std::random_device device;
    std::default_random_engine engine(device());
    std::string current;
    std::size_t generation = 0;
    std::size_t distance = 0;
    std::pair<std::string, std::size_t> best;

    run_tests();

    current = random_string(engine, kTargetSize);
    distance = distance_to_target(current, kTarget);

    std::cout << generation << kIterationSeparator << current
              << kIterationSeparator << distance << '\n';

    while (distance != 0u)
    {
        ++generation;
        best = next_generation(current, kTarget, engine);
        current = best.first;
        distance = best.second;

        std::cout << generation << kIterationSeparator << current
                  << kIterationSeparator << distance << '\n';
    }

    return 0;
}