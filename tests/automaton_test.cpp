#include "tagstream/automaton.hpp"

#include <gtest/gtest.h>

using tagstream::Automaton;
using tagstream::Pattern;

TEST(Trie, EmptyAutomatonHasOnlyRoot) {
    Automaton a({});
    EXPECT_EQ(a.node_count(), 1u);
    EXPECT_EQ(a.walk(""), 0);
}

TEST(Trie, WalkReachesInsertedPattern) {
    Automaton a({{"he", "T1"}});
    int node = a.walk("he");
    ASSERT_NE(node, -1);
    EXPECT_EQ(a.outputs_at(node), std::vector<int>{0});
}

TEST(Trie, PrefixOfPatternHasNoOutput) {
    Automaton a({{"hers", "T1"}});
    int node = a.walk("he");
    ASSERT_NE(node, -1);
    EXPECT_TRUE(a.outputs_at(node).empty());
}

TEST(Trie, MissingPathReturnsMinusOne) {
    Automaton a({{"he", "T1"}});
    EXPECT_EQ(a.walk("x"), -1);
    EXPECT_EQ(a.walk("hex"), -1);
}

TEST(Trie, SharedPrefixesShareNodes) {
    Automaton a({{"he", "A"}, {"hers", "B"}, {"his", "C"}});
    EXPECT_EQ(a.node_count(), 7u);
}

TEST(Trie, DuplicatePatternsBothRecorded) {
    Automaton a({{"he", "A"}, {"he", "B"}});
    EXPECT_EQ(a.outputs_at(a.walk("he")), (std::vector<int>{0, 1}));
}

TEST(Trie, EmptyPatternIsIgnored) {
    Automaton a({{"", "EMPTY"}});
    EXPECT_EQ(a.node_count(), 1u);
    EXPECT_TRUE(a.outputs_at(0).empty());
}
// Classic textbook example. Indices: he=0, she=1, his=2, hers=3.
static std::vector<Pattern> classic() {
    return {{"he", "HE"}, {"she", "SHE"}, {"his", "HIS"}, {"hers", "HERS"}};
}

TEST(FailureLinks, PointToLongestSuffix) {
    Automaton a(classic());
    EXPECT_EQ(a.fail_of(a.walk("she")), a.walk("he"));
    EXPECT_EQ(a.fail_of(a.walk("sh")), a.walk("h"));
    EXPECT_EQ(a.fail_of(a.walk("hers")), a.walk("s"));
    EXPECT_EQ(a.fail_of(a.walk("h")), 0);
}

TEST(FailureLinks, NodesInheritSuffixOutputs) {
    Automaton a(classic());
    EXPECT_EQ(a.outputs_at(a.walk("she")), (std::vector<int>{1, 0}));
}
TEST(Match, ReturnsEmptyWhenNothingMatches) {
    Automaton a(classic());
    EXPECT_TRUE(a.match("xyz").empty());
}

TEST(Match, AgreesWithNaiveMatcher) {
    std::vector<Pattern> p = {
        {"Apple", "AAPL"}, {"Apple Inc", "AAPL"}, {"Microsoft", "MSFT"}, {"Royal Bank", "RY"}};
    Automaton a(p);
    for (const char* line : {"Apple and Microsoft shares fall", "Royal Bank raises dividend",
                             "Weather warning in Montreal", ""}) {
        EXPECT_EQ(a.match(line), tagstream::naive_match(line, p)) << "line: " << line;
    }
}

TEST(Match, IgnoresMatchesInsideWords) {
    Automaton a(classic());
    EXPECT_TRUE(a.match("ushers").empty());
}

TEST(Match, FindsOverlappingWholeWords) {
    Automaton a({{"Bank of Montreal", "BMO"}, {"Montreal", "MTL"}});
    EXPECT_EQ(a.match("Bank of Montreal raises dividend"), (std::set<std::string>{"BMO", "MTL"}));
}

TEST(Match, IgnoresCase) {
    Automaton a(classic());
    EXPECT_EQ(a.match("SHE said HE"), (std::set<std::string>{"SHE", "HE"}));
}