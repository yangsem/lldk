#ifndef LLDK_LOGGER_CONFIG_H
#define LLDK_LOGGER_CONFIG_H

#include "lldk/common/common.h"

namespace lldk
{
namespace logger
{
namespace config
{
constexpr const char *kLogLevel = "log_level"; // The level of the logger,[debug, info, warn, error, fatal, event]
constexpr const char *kAsyncLog = "async_log"; // Whether to log asynchronously [true, false]
constexpr const char *kLogPrefix = "log_prefix"; // The prefix of the log, e.g. [ /home/log ]
constexpr const char *kLogSuffix = "log_suffix"; // The suffix of the log, e.g. [ log, txt, etc. ]
constexpr const char *kLogFileSizeMB = "log_file_size_mb"; // The size of the log file, e.g. [ 10 ]
constexpr const char *kLogFileCount = "log_file_count"; // The count of the log file, e.g. [ 10 ]
}
namespace defaultvalue
{
constexpr const char *kLogLevel = "info";
constexpr const char *kAsyncLog = "false";
constexpr const char *kLogPrefix = "./";
constexpr const char *kLogSuffix = "log";
constexpr const char *kLogFileSizeMB = "16";
constexpr const char *kLogFileCount = "16";
}
}
}

#endif // LLDK_LOGGER_CONFIG_H
