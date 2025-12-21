#ifndef LLDK_COMMON_ERROR_CODE_H
#define LLDK_COMMON_ERROR_CODE_H

#include "lldk/common/common.h"

namespace lldk
{
namespace common
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
};

/**
 * @brief Set the error code
 * @param eErrorCode The error code
 */
LLDK_EXTERN_C void lldkSetErrorCode(ErrorCode eErrorCode);

/**
 * @brief Get the error code
 * @return The error code
 */
LLDK_EXTERN_C ErrorCode lldkGetErrorCode();

/**
 * @brief Get the error code string
 * @param eErrorCode The error code
 * @return The error code string
 */
LLDK_EXTERN_C const char *lldkGetErrorCodeString(ErrorCode eErrorCode);

}
}
#endif // LLDK_COMMON_ERROR_CODE_H
