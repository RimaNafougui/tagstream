#include <gtest/gtest.h>

#include <random>
#include <string>
#include <vector>

#include "tagstream/automaton.hpp"
#include "tagstream/matcher.hpp"

using tagstream::Automaton;
using tagstream::Pattern;

namespace {

// Builds a random string of length [min_len, max_len] from the given letters.
std::string random_string(std::mt19937& rng, const std::string& alphabet, int min_len,
                          int max_len) {
    std::uniform_int_distribution<int> length(min_len, max_len);
    std::uniform_int_distribution<int> letter(0, static_cast<int>(alphabet.size()) - 1);
    int n = length(rng);
    std::string s;
    for (int i = 0; i < n; ++i) {
        s += alphabet[letter(rng)];
    }
    return s;
}

// One random case: random patterns, several random lines, compare both matchers.
void check_seed(unsigned seed, const std::string& alphabet, int max_pattern_len, int max_line_len) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pattern_count(1, 10);

    std::vector<Pattern> patterns;
    int n = pattern_count(rng);
    for (int i = 0; i < n; ++i) {
        std::string text = random_string(rng, alphabet, 0, max_pattern_len);
        patterns.push_back({text, text});  // tag = text, so failures are easy to read
    }

    Automaton automaton(patterns);
    for (int t = 0; t < 5; ++t) {
        std::string line = random_string(rng, alphabet, 0, max_line_len);
        ASSERT_EQ(automaton.match(line), tagstream::naive_match(line, patterns))
            << "seed " << seed << ", line \"" << line << "\"";
    }
}

}  // namespace

TEST(Property, AgreesWithNaiveOnTwoLetters) {
    for (unsigned seed = 0; seed < 2000; ++seed) {
        check_seed(seed, "ab", 5, 30);
        if (HasFatalFailure()) return;
    }
}

TEST(Property, AgreesWithNaiveOnThreeLetters) {
    for (unsigned seed = 0; seed < 2000; ++seed) {
        check_seed(seed, "abc", 6, 50);
        if (HasFatalFailure()) return;
    }
}

TEST(Property, AgreesWithNaiveOnRealisticText) {
    for (unsigned seed = 0; seed < 500; ++seed) {
        check_seed(seed, "Apple Inc", 8, 80);
        if (HasFatalFailure()) return;
    }
}