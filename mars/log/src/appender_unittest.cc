#include "mars/comm/xlogger/xloggerbase.h"
#include "gtest/gtest.h"

#include <cstring>
#include <string>

using namespace testing;

static int calc_dump_required_length(int srcbytes){
    //MUST CHANGE THIS IF YOU CHANGE `to_string` function.
    return srcbytes * 6 + 1;
}

TEST(appender, memorydump){
    char srcbuffer[4096];    
    char dummybuf[64];

    const char* dump1 = xlogger_memory_dump(srcbuffer, 1);
    int head_bytes = snprintf(dummybuf, sizeof(dummybuf), "\n%zu bytes:\n",1);
    EXPECT_EQ(strlen(dump1), calc_dump_required_length(1) + head_bytes + 1);    // 1 for '\n'

    dump1 = xlogger_memory_dump(srcbuffer, 121);
    head_bytes = snprintf(dummybuf, sizeof(dummybuf), "\n%zu bytes:\n",121);
    int round = 121/32 + 1; // +1 because not aligned to 32
    EXPECT_EQ(strlen(dump1), calc_dump_required_length(121) + head_bytes + round * 2 - 1);

    dump1 = xlogger_memory_dump(srcbuffer, 128);
    head_bytes = snprintf(dummybuf, sizeof(dummybuf), "\n%zu bytes:\n",128);
    round = 128/32;
    EXPECT_EQ(strlen(dump1), calc_dump_required_length(128) + head_bytes + round * 2 - 1);

    EXPECT_GT(calc_dump_required_length(4096), 4096);

    dump1 = xlogger_memory_dump(srcbuffer, 4096);
    EXPECT_LT(strlen(dump1), 4096);

    std::string sdump1(dump1);
    const char* dump2 = xlogger_memory_dump(srcbuffer, 673);
    EXPECT_EQ(sdump1.length(), strlen(dump2) + 1);
}

EXPORT_GTEST_SYMBOLS(log_export_appender_unittest)

