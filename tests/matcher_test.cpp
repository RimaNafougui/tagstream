#include "tagstream/matcher.hpp"

#include <gtest/gtest.h>
using tagstream::naive_match;
using tagstream::Pattern;

TEST(ToLower, ConvertsUppercase) { EXPECT_EQ(tagstream::to_lower("APPLE Inc"), "apple inc"); }

TEST(ToLower, LeavesEmptyStringEmpty) { EXPECT_EQ(tagstream::to_lower(""), ""); }

TEST(NaiveMatch, FindsOnePattern) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_EQ(naive_match("Apple shares fall", p), (std::set<std::string>{"AAPL"}));
}

TEST(NaiveMatch, FindsSeveralPatterns) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}, {"Microsoft", "MSFT"}};
    EXPECT_EQ(naive_match("Apple and Microsoft shares fall", p),
              (std::set<std::string>{"AAPL", "MSFT"}));
}

TEST(NaiveMatch, ReturnsEmptyWhenNothingMatches) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_TRUE(naive_match("Weather warning in Montreal", p).empty());
}

TEST(NaiveMatch, ReportsRepeatedTagOnce) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}, {"Apple Inc", "AAPL"}};
    EXPECT_EQ(naive_match("Apple Inc says Apple will grow", p), (std::set<std::string>{"AAPL"}));
}

TEST(NaiveMatch, IgnoresEmptyPattern) {
    std::vector<Pattern> p = {{"", "EMPTY"}};
    EXPECT_TRUE(naive_match("anything", p).empty());
}

TEST(NaiveMatch, IgnoresCase) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_EQ(naive_match("APPLE and apple", p), (std::set<std::string>{"AAPL"}));
}

TEST(NaiveMatch, RejectsMatchInsideWord) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_TRUE(naive_match("pineapple juice", p).empty());
}

TEST(NaiveMatch, AcceptsPunctuationAroundWord) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_EQ(naive_match("(Apple's results)", p), (std::set<std::string>{"AAPL"}));
}

TEST(NaiveMatch, FindsLaterWholeWordOccurrence) {
    std::vector<Pattern> p = {{"Apple", "AAPL"}};
    EXPECT_EQ(naive_match("pineapple and Apple", p), (std::set<std::string>{"AAPL"}));
}

TEST(WholeWord, ChecksBothSides) {
    using tagstream::is_whole_word;
    EXPECT_TRUE(is_whole_word("apple", 0, 5));       // whole line
    EXPECT_TRUE(is_whole_word("an apple.", 3, 8));   // space before, dot after
    EXPECT_FALSE(is_whole_word("pineapple", 4, 9));  // letter before
    EXPECT_FALSE(is_whole_word("apples", 0, 5));     // letter after
}