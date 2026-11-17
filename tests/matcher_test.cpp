#include <gtest/gtest.h>
#include "tagstream/matcher.hpp"

TEST(ToLower, ConvertsUppercase) {
    EXPECT_EQ(tagstream::to_lower("APPLE Inc"), "apple inc");
}

TEST(ToLower, LeavesEmptyStringEmpty) {
    EXPECT_EQ(tagstream::to_lower(""), "");
}