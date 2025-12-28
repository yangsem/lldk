#include "gtest/gtest.h"
#include "lldk/common/error_code.h"
#include <thread>

TEST(ErrorCode, SetErrorCode)
{
    EXPECT_EQ(lldkGetErrorCode(), lldk::ErrorCode::kSuccess);
    lldkSetErrorCode(lldk::ErrorCode::kSuccess);
    EXPECT_EQ(lldkGetErrorCode(), lldk::ErrorCode::kSuccess);
}

TEST(ErrorCode, SetErrorCodeMultipleThreads)
{
    auto func = [](lldk::ErrorCode eErrorCode) {
        timespec begin, end;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        end = begin;
        while (end.tv_sec - begin.tv_sec < 10)
        {
            lldkSetErrorCode(eErrorCode);
            EXPECT_EQ(lldkGetErrorCode(), eErrorCode);

            clock_gettime(CLOCK_MONOTONIC, &end);
        }
    };

    std::thread thread1(func, lldk::ErrorCode::kWarn);
    func(lldk::ErrorCode::kError);

    thread1.join();
}

TEST(ErrorCode, GetErrorStr)
{
    EXPECT_STREQ(lldkGetErrorStr((lldk::ErrorCode)-2), "");
    EXPECT_STREQ(lldkGetErrorStr(lldk::ErrorCode::kSuccess), "Success");
    EXPECT_STREQ(lldkGetErrorStr(lldk::ErrorCode::kUnknown), "Unknown");
}

TEST(ErrorCode, SetErrorMsg)
{
    EXPECT_STREQ(lldkGetErrorMsg(), "");
    EXPECT_EQ(lldkSetErrorMsg("test"), 0);
    EXPECT_STREQ(lldkGetErrorMsg(), "test");
}

TEST(ErrorCode, SetErrorMsgMultipleThreads)
{
    auto func = [](const char *pMsg) {
        timespec begin, end;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        end = begin;
        while (end.tv_sec - begin.tv_sec < 10)
        {
            lldkSetErrorMsg(pMsg);
            EXPECT_STREQ(lldkGetErrorMsg(), pMsg);

            clock_gettime(CLOCK_MONOTONIC, &end);
        }
    };

    std::thread thread1(func, "test1");
    func("test2");

    thread1.join();
}