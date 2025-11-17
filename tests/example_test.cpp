#include <gtest/gtest.h>

// Example test to verify googletest is working
TEST(ExampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

TEST(ExampleTest, StringComparison) {
    std::string str = "Ant Colony";
    EXPECT_EQ(str, "Ant Colony");
    EXPECT_NE(str, "Other");
}
