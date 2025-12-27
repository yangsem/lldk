#ifndef LLDK_UTILITY_TIME_H
#define LLDK_UTILITY_TIME_H

#include "lldk/base/common.h"
#include <time.h>

namespace lldk
{
namespace utility
{

struct TimeSpec
{
    uint32_t uYear;  // e.g. 2025
    uint32_t uMonth; // 1-12
    uint32_t uDay;   // 1-31
    uint32_t uHour;  // 0-23
    uint32_t uMinute; // 0-59
    uint32_t uSecond; // 0-59
    uint64_t uMicrosecond; // 0-999999
};

class ITime
{
protected:
    virtual ~ITime() = default;

public:
    /**
     * @brief Get the time
     * @param pTimeSpec The time spec pointer
     * @return 0 if success, -1 if failed
     */
    virtual int32_t getTime(TimeSpec *pTimeSpec) = 0;

    /**
     * @brief Get the time
     * @param pTimeSpec The time spec pointer
     * @return The timestamp in microseconds
     */
    virtual uint64_t getTimeStamp(TimeSpec *pTimeSpec) = 0;

    /**
     * @brief Get the Unix timestamp
     * @param pTimeSpec The time spec pointer
     * @return The Unix timestamp in microseconds
     */
    virtual uint64_t getUnixTimeStamp(TimeSpec *pTimeSpec) = 0;

    /**
     * @brief Add time
     * @param pTimeSpec The time spec pointer
     * @param uMicroseconds The microseconds to add
     * @return 0 if success, -1 if failed
     */
    virtual int32_t addTime(TimeSpec *pTimeSpec, uint64_t uMicroseconds) = 0;

    /**
     * @brief Subtract time
     * @param pTimeSpec The time spec pointer
     * @param uMicroseconds The microseconds to subtract
     * @return 0 if success, -1 if failed
     */
    virtual int32_t subTime(TimeSpec *pTimeSpec, uint64_t uMicroseconds) = 0;

    /**
     * @brief Compare time
     * @param pTimeSpec1 The first time spec pointer
     * @param pTimeSpec2 The second time spec pointer
     * @return 0 if equal, -1 if less, 1 if greater
     */
    virtual int32_t cmpTime(TimeSpec *pTimeSpec1, TimeSpec *pTimeSpec2) = 0;

    /**
     * @brief Compare time
     * @param pTimeSpec The time spec pointer
     * @param uMicroseconds The microseconds to compare
     * @return 0 if equal, -1 if less, 1 if greater
     */
    virtual int32_t cmpTime(TimeSpec *pTimeSpec, uint64_t uMicroseconds) = 0;

    /**
     * @brief Get the time string
     * @param pTimeSpec The time spec pointer
     * @return The time string
     * @note the time string is in the format of "YYYYMMDD HH:MM:SS.microseconds"
     *       the time string is valid until the next call of getTimeStr in the same thread.
     */
    virtual const char *getTimeStr(TimeSpec *pTimeSpec) = 0;
};

}
}

/**
 * @brief Get the local time
 * @param pTime The time pointer
 * @return 0 if success, -1 if failed
 */
LLDK_EXTERN_C int32_t lldkGetTime(lldk::utility::TimeSpec *pTimeSpec);

/**
 * @brief Get the time singleton
 * @return The time singleton pointer, NULL if failed
 * @note the time singleton is a singleton that provides the time functionality, all functions are thread safe.
 */
LLDK_EXTERN_C lldk::utility::ITime *lldkGetTimeSingleton();

/**
 * @brief Get the clock monotonic nanoseconds
 * @return The clock monotonic nanoseconds
 */
LLDK_INLINE uint64_t lldkGetClockMonotonicNs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/**
 * @brief Get the clock monotonic microseconds
 * @return The clock monotonic microseconds
 */
LLDK_INLINE uint64_t lldkGetClockMonotonicUs()
{
    return lldkGetClockMonotonicNs() / 1000ULL;
}

/**
 * @brief Get the clock monotonic milliseconds
 * @return The clock monotonic milliseconds
 */
LLDK_INLINE uint64_t lldkGetClockMonotonicMs()
{
    return lldkGetClockMonotonicNs() / (1000ULL * 1000ULL);
}

/**
 * @brief Get the clock monotonic seconds
 * @return The clock monotonic seconds
 */
LLDK_INLINE uint64_t lldkGetClockMonotonicSec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

#endif // LLDK_UTILITY_TIME_H
