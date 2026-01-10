#include "lldk/common/error_code.h"
#include <string>
#include <unordered_map>

static thread_local lldk::ErrorCode s_eErrorCode = lldk::ErrorCode::kSuccess;
static thread_local std::string s_strErrorMsg = "";

static std::unordered_map<lldk::ErrorCode, const char *> s_pErrorStrMap = {
    {lldk::ErrorCode::kUnknown, "Unknown"},
    {lldk::ErrorCode::kSuccess, "Success"},
    {lldk::ErrorCode::kDebug, "Debug"},
    {lldk::ErrorCode::kInfo, "Info"},
    {lldk::ErrorCode::kWarn, "Warn"},
    {lldk::ErrorCode::kError, "Error"},
    {lldk::ErrorCode::kEvent, "Event"},

    {lldk::ErrorCode::kSystemCallError, "System call error"},
    {lldk::ErrorCode::kThrowException, "Throw exception"},
    {lldk::ErrorCode::kNoMemory, "No memory"},
    {lldk::ErrorCode::kInvalidParam, "Invalid parameter"},
    {lldk::ErrorCode::kInvalidState, "Invalid state"},
    {lldk::ErrorCode::kInvalidCall, "Invalid call"},
    {lldk::ErrorCode::kCallFailed, "Call failed"},
};

extern "C" {

void lldkSetErrorCode(lldk::ErrorCode eErrorCode)
{
    s_eErrorCode = eErrorCode;
}

lldk::ErrorCode lldkGetErrorCode()
{
    return s_eErrorCode;
}

const char *lldkGetErrorStr(lldk::ErrorCode eErrorCode)
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
        return (int32_t)lldk::ErrorCode::kInvalidParam;
    }

    try
    {
        s_strErrorMsg = pMsg;
    }
    catch (...)
    {
        LLDK_PRINT_ERROR("Failed to set error message: %s", pMsg);
        return (int32_t)lldk::ErrorCode::kThrowException;
    }

    return 0;
}

const char *lldkGetErrorMsg()
{
    return s_strErrorMsg.c_str();
}

} // extern "C"