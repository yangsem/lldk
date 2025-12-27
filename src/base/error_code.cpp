#include "lldk/base/error_code.h"
#include "lldk/base/common.h"
#include <string>
#include <unordered_map>

static thread_local lldk::common::ErrorCode s_eErrorCode = lldk::common::ErrorCode::kSuccess;
static thread_local std::string *s_pErrorMsg = nullptr;

static std::unordered_map<lldk::common::ErrorCode, const char *> s_pErrorStrMap = {
    {lldk::common::ErrorCode::kUnknown, "Unknown"},
    {lldk::common::ErrorCode::kSuccess, "Success"},
    {lldk::common::ErrorCode::kDebug, "Debug"},
    {lldk::common::ErrorCode::kInfo, "Info"},
    {lldk::common::ErrorCode::kWarn, "Warn"},
    {lldk::common::ErrorCode::kError, "Error"},
    {lldk::common::ErrorCode::kEvent, "Event"},

    {lldk::common::ErrorCode::kSystemCallError, "System call error"},
    {lldk::common::ErrorCode::kThrowException, "Throw exception"},
    {lldk::common::ErrorCode::kNoMemory, "No memory"},
    {lldk::common::ErrorCode::kInvalidParam, "Invalid parameter"},
    {lldk::common::ErrorCode::kInvalidState, "Invalid state"},
};

void lldkSetErrorCode(lldk::common::ErrorCode eErrorCode)
{
    s_eErrorCode = eErrorCode;
}

lldk::common::ErrorCode lldkGetErrorCode()
{
    return s_eErrorCode;
}

const char *lldkGetErrorStr(lldk::common::ErrorCode eErrorCode)
{
    auto it = s_pErrorStrMap.find(eErrorCode);
    if (likely(it != s_pErrorStrMap.end()))
    {
        return it->second;
    }

    return "";
}

int32_t lldkSetErrorMsg(const char *pMsg)
{
    if (unlikely(pMsg == nullptr))
    {
        return (int32_t)lldk::common::ErrorCode::kInvalidParam;
    }

    if (unlikely(s_pErrorMsg == nullptr))
    {
        s_pErrorMsg = LLDK_NEW std::string();
        if (unlikely(s_pErrorMsg == nullptr))
        {
            LLDK_PRINT_ERROR("Failed to allocate memory for error message");
            return (int32_t)lldk::common::ErrorCode::kNoMemory;
        }
    }

    try
    {
        *s_pErrorMsg = pMsg;
    }
    catch (...)
    {
        LLDK_PRINT_ERROR("Failed to set error message: %s", s_pErrorMsg->c_str());
        return (int32_t)lldk::common::ErrorCode::kThrowException;
    }

    return 0;
}

const char *lldkGetErrorMsg()
{
    if (unlikely(s_pErrorMsg == nullptr))
    {
        return "";
    }

    return s_pErrorMsg->c_str();
}