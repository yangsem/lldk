#ifndef LLDK_THREAD_CONDITION_VARIABLE_H
#define LLDK_THREAD_CONDITION_VARIABLE_H

#include "lldk/base/common.h"

namespace lldk
{
namespace thread
{

class IConditionVariable
{
protected:
    virtual ~IConditionVariable() = default;

public:
    /**
     * @brief The wait function type
     * @param pArg The argument of the wait function
     * @return true if the wait function returns true, false if the wait function returns false or the timeout occurs
     */
    using WaitFunc = bool (*)(void *pArg);

    /**
     * @brief Wait for the condition variable
     */
    virtual void wait() = 0;

    /**
     * @brief Wait for the condition variable with a timeout
     * @param uTimeoutMs The timeout in milliseconds
     * @param pWaitFunc The wait function
     * @param pArg The argument of the wait function
     */
    virtual void waitFor(uint64_t uTimeoutMs, WaitFunc pWaitFunc, void *pArg) = 0;

    /**
     * @brief Notify one thread
     */
    virtual void notifyOne() = 0;

    /**
     * @brief Notify all threads
     */
    virtual void notifyAll() = 0;
};

}
}

/**
 * @brief Create a condition variable
 * @return The condition variable pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::IConditionVariable *lldkCreateConditionVariable();

/**
 * @brief Destroy a condition variable
 * @param pConditionVariable The condition variable pointer
 */
LLDK_EXTERN_C void lldkDestroyConditionVariable(lldk::thread::IConditionVariable *pConditionVariable);

#endif // LLDK_THREAD_CONDITION_VARIABLE_H
