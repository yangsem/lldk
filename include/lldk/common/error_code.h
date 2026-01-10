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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the error code in the current thread
 * @param eErrorCode The error code
 */
LLDK_EXPORT void lldkSetErrorCode(lldk::ErrorCode eErrorCode);

/**
 * @brief Get the error code in the current thread
 * @return The error code
 */
LLDK_EXPORT lldk::ErrorCode lldkGetErrorCode();

/**
 * @brief Get the error message in the current thread
 * @param eErrorCode The error code
 * @return The error message string, "" if failed
 */
LLDK_EXPORT const char *lldkGetErrorStr(lldk::ErrorCode eErrorCode);

/**
 * @brief Set the error message in the current thread
 * @param pMsg The error message
 * @return 0 if success, -1 if failed
 */
LLDK_EXPORT int32_t lldkSetErrorMsg(const char *pMsg);

/**
 * @brief Get the error message in the current thread
 * @return The error message
 */
LLDK_EXPORT const char *lldkGetErrorMsg();

#ifdef __cplusplus
}
#endif

#endif // LLDK_COMMON_ERROR_CODE_H
