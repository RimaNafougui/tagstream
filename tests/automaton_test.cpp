#include "tagstream/automaton.hpp"

#include <gtest/gtest.h>

using tagstream::Automaton;

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