#ifndef LLDK_COMMON_ERROR_CODE_H
#define LLDK_COMMON_ERROR_CODE_H

#include "lldk/common/common.h"

namespace lldk
{

enum class ErrorCode : int32_t
{
    kUnknown = -1,
    kSuccess = 0,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kEvent,

    kSystemCallError = 100,
    kThrowException,
    kNoMemory,
    kInvalidParam,
    kInvalidState,
    kInvalidCall,
    kCallFailed,
};

}

/**
 * @brief Set the error code in the current thread
 * @param eErrorCode The error code
 */
LLDK_EXTERN_C void lldkSetErrorCode(lldk::ErrorCode eErrorCode);

/**
 * @brief Get the error code in the current thread
 * @return The error code
 */
LLDK_EXTERN_C lldk::ErrorCode lldkGetErrorCode();

/**
 * @brief Get the error message in the current thread
 * @param eErrorCode The error code
 * @return The error message string, "" if failed
 */
LLDK_EXTERN_C const char *lldkGetErrorStr(lldk::ErrorCode eErrorCode);

/**
 * @brief Set the error message in the current thread
 * @param pMsg The error message
 * @return 0 if success, -1 if failed
 */
LLDK_EXTERN_C int32_t lldkSetErrorMsg(const char *pMsg);

/**
 * @brief Get the error message in the current thread
 * @return The error message
 */
LLDK_EXTERN_C const char *lldkGetErrorMsg();

#endif // LLDK_COMMON_ERROR_CODE_H
