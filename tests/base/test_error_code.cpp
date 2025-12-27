#include "gtest/gtest.h"
#include "lldk/base/error_code.h"

TEST(ErrorCode, SetErrorCode)
{
    EXPECT_EQ(lldkGetErrorCode(), lldk::common::ErrorCode::kSuccess);
    lldkSetErrorCode(lldk::common::ErrorCode::kSuccess);
    EXPECT_EQ(lldkGetErrorCode(), lldk::common::ErrorCode::kSuccess);
}

TEST(ErrorCode, GetErrorStr)
{
    EXPECT_STREQ(lldkGetErrorStr(lldk::common::ErrorCode::kSuccess), "Success");
    EXPECT_STREQ(lldkGetErrorStr(lldk::common::ErrorCode::kUnknown), "");
}

TEST(ErrorCode, SetErrorMsg)
{
    EXPECT_STREQ(lldkGetErrorMsg(), "");
    EXPECT_EQ(lldkSetErrorMsg("test"), 0);
    EXPECT_STREQ(lldkGetErrorMsg(), "test");
}