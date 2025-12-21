#ifndef LLDK_THREAD_TIMER_H
#define LLDK_THREAD_TIMER_H

#include "lldk/common/common.h"

namespace lldk
{
namespace thread
{

class ITimer
{
protected:
    virtual ~ITimer() = default;

public:
    enum class TimerState
    {
        kUnknown = 0,
        kInit,
        kWaiting,
        kRunning,
        kStopped,
    };

    /**
     * @brief The timer function type
     * @param pArg The argument of the timer function
     * @return void
     */
    using TimerFunc = void (*)(void *pArg);

    /**
     * @brief Create thread and start the timer
     * @param pFunc The timer function
     * @param pArg The argument of the timer function
     * @param uDelayMs The delay time in milliseconds, 0 means immediate execution
     * @param uIntervalMs The interval time in milliseconds, 0 means one-time timer
     * @return 0 if success, -1 if failed
     */
    virtual int32_t start(TimerFunc pFunc, void *pArg, uint64_t uDelayMs, uint64_t uIntervalMs) = 0;

    /**
     * @brief Stop the timer
     * @param bCall If true, call the timer function before stopping
     * @return 0 if success, -1 if failed
     */
    virtual int32_t stop(bool bCall = false) = 0;

    /**
     * @brief Get the state of the timer
     * @return The state of the timer
     */
    virtual TimerState getState() const { return m_eState; }

private:
    TimerState m_eState = TimerState::kUnknown;
};

}
}

/**
 * @brief Create a timer
 * @param pName The name of the timer
 * @return The timer pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::ITimer *lldkCreateTimer(const char *pName);

/**
 * @brief Destroy a timer
 * @param pTimer The timer pointer
 */
LLDK_EXTERN_C void lldkDestroyTimer(lldk::thread::ITimer *pTimer);

#endif // LLDK_THREAD_TIMER_H