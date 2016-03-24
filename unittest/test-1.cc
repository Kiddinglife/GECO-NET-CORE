#include "gtest/gtest.h"
#include "geco-globals.h"
TEST(testcase2, testHello2)
{
    char result[32];
    geco::ultils::Itoa(12, result, 10);
    EXPECT_STREQ(result, "12");
    geco::ultils::Itoa(12, result, 2);
    EXPECT_STREQ(result, "1100");
    geco::ultils::Itoa(12, result, 16);
    EXPECT_STREQ(result, "c");
}

