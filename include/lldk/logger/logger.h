#ifndef LLDK_LOGGER_LOGGER_H
#define LLDK_LOGGER_LOGGER_H

#include "lldk/base/common.h"

namespace lldk
{
namespace logger
{

class ILogger
{
protected:
    virtual ~ILogger() = default;

public:
    enum class Level : uint32_t
    {
        kUnknown = 0,
        kDebug,
        kInfo,
        kWarn,
        kError,
        kFatal,
        kEvent,
    };

    struct Config
    {
        const char *pKey;    // The key of the config
        const char *pValue;  // The value of the config
    };

    /**
     * @brief Initialize the logger
     * @param pConfig The config array
     * @param uConfigCount The count of the config
     * @return 0 if success, -1 if failed
     */
    virtual int32_t init(const Config *pConfig, uint32_t uConfigCount) = 0;

    /**
     * @brief Start the logger thread
     * @return 0 if success, -1 if failed
     */
    virtual int32_t start() = 0;

    /**
     * @brief Stop the logger thread
     */
    virtual void stop() = 0;

    /**
     * @brief Log a message
     * @param eLevel The level of the message
     * @param iErrorCode The error code of the message
     * @param pMessage The message
     * @return written bytes if success, -1 if failed
     */
    virtual int32_t log(Level eLevel, int32_t iErrorCode, const char *pMessage) = 0;

    /**
     * @brief Log a formatted message
     * @param eLevel The level of the message
     * @param iErrorCode The error code of the message
     * @param pFormat The format of the message, string constant, e.g. "this is a {} test, {} {} {} ..."
     * @param ppParams The parameters of the message, e.g. ["test", "test1", "test2", "test3"]
     * @param uParamCount The count of the parameters
     * @return written bytes if success, -1 if failed
     */
    virtual int32_t log(Level eLevel, int32_t iErrorCode, const char *pFormat, const char **ppParams, uint32_t uParamCount) = 0;

    /**
     * @brief Set the level of the logger
     * @param eLevel The level of the logger
     */
    LLDK_INLINE void setLevel(Level eLevel)
    {
        m_eLevel = eLevel;
    }

    /**
     * @brief Get the level of the logger
     * @return The level of the logger
     */
    LLDK_INLINE Level getLevel() const
    {
        return LLDK_ACCESS_ONCE(m_eLevel);
    }

    /**
     * @brief Get the stats of the logger
     * @return The stats of the logger
     */
    virtual const char *getStats() const = 0;

private:
    Level m_eLevel = Level::kUnknown;
};

}
}

/**
 * @brief Create a logger
 * @param pName The name of the logger
 * @return The logger pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::logger::ILogger *lldkCreateLogger(const char *pName);

/**
 * @brief Destroy a logger
 * @param pLogger The logger pointer
 */
LLDK_EXTERN_C void lldkDestroyLogger(lldk::logger::ILogger *pLogger);

/**
 * @brief Get the logger singleton
 * @return The logger pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::logger::ILogger *lldkGetLoggerSingleton();

#define __LLDK_LOG_BASE(pLogger, eLevel, iErrorCode, pFormat, ...)             \
  {                                                                            \
    if ((pLogger) != nullptr && (eLevel) >= (pLogger)->getLevel()) {           \
      const char *ppParams[] = {MODULE_NAME, ##__VA_ARGS__,                    \
                                LLDK_CODE_POSITION};                           \
      (pLogger)->log(eLevel, int32_t(iErrorCode), "[{}] " pFormat "({},{})",   \
                     ppParams, sizeof(ppParams) / sizeof(ppParams[0]));        \
    }                                                                          \
  }

#define LLDK_LOG_DEBUG(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kDebug, iErrorCode, pFormat, ##__VA_ARGS__)
#define LLDK_LOG_INFO(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kInfo, iErrorCode, pFormat, ##__VA_ARGS__)
#define LLDK_LOG_WARN(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kWarn, iErrorCode, pFormat, ##__VA_ARGS__)
#define LLDK_LOG_ERROR(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kError, iErrorCode, pFormat, ##__VA_ARGS__)
#define LLDK_LOG_FATAL(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kFatal, iErrorCode, pFormat, ##__VA_ARGS__)
#define LLDK_LOG_EVENT(pLogger, iErrorCode, pFormat, ...) __LLDK_LOG_BASE(pLogger, lldk::logger::ILogger::Level::kEvent, iErrorCode, pFormat, ##__VA_ARGS__)

#endif // LLDK_LOGGER_LOGGER_H
