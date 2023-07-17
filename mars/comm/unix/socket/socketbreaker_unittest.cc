
#include "socketbreaker.h"

#include "gtest/gtest.h"

using namespace mars::comm;

TEST(socketbreaker, init) {
    SocketBreaker brk;

    EXPECT_TRUE(brk.IsCreateSuc());
    EXPECT_FALSE(brk.IsBreak());
    EXPECT_EQ(brk.BreakReason(), 0);
}

TEST(socketbreaker, create) {
    SocketBreaker brk;

    EXPECT_TRUE(brk.IsCreateSuc());
    brk.ReCreate();
    EXPECT_TRUE(brk.IsCreateSuc());
    brk.Clear();
    EXPECT_TRUE(brk.IsCreateSuc());
}

TEST(socketbreaker, close) {
    SocketBreaker brk;

    EXPECT_TRUE(brk.IsCreateSuc());
    brk.Close();
    EXPECT_EQ(brk.BreakReason(), 0);
    EXPECT_EQ(brk.BreakerFD(), -1);
    EXPECT_FALSE(brk.IsBreak());
    EXPECT_FALSE(brk.Break());
    EXPECT_TRUE(brk.Clear());
}

TEST(socketbreaker, dobreak) {
    SocketBreaker brk;

    EXPECT_TRUE(brk.IsCreateSuc());
    brk.Break();
    EXPECT_TRUE(brk.IsBreak());
    EXPECT_EQ(brk.BreakReason(), 0);

    brk.Break(1);
    EXPECT_TRUE(brk.IsBreak());
    EXPECT_EQ(brk.BreakReason(), 1);

    brk.ReCreate();
    EXPECT_FALSE(brk.IsBreak());
    brk.Break(1);
    brk.Break(2);
    EXPECT_TRUE(brk.IsBreak());
    EXPECT_EQ(brk.BreakReason(), 2);
}

TEST(socketbreaker, clear) {
    SocketBreaker brk;

    EXPECT_TRUE(brk.IsCreateSuc());
    brk.Break(2);
    EXPECT_TRUE(brk.IsBreak());
    brk.Clear();
    EXPECT_FALSE(brk.IsBreak());
    for (int i = 0; i < 5; i++) {
        EXPECT_FALSE(brk.IsBreak());
    }
    brk.Break(1111);
    EXPECT_TRUE(brk.IsBreak());
    EXPECT_EQ(brk.BreakReason(), 1111);
}

TEST(socketbreaker, massclear) {
    SocketBreaker brk;
    EXPECT_TRUE(brk.IsCreateSuc());

    //多次break可以被1次clear清除
    for (int i = 0; i < 10000; i++) {
        brk.Break();
    }
    EXPECT_TRUE(brk.IsBreak());
    brk.Clear();
    EXPECT_FALSE(brk.IsBreak());
}

EXPORT_GTEST_SYMBOLS(socketbreaker_unittest)