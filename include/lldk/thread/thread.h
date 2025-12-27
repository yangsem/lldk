#ifndef LLDK_THREAD_THREAD_H
#define LLDK_THREAD_THREAD_H

#include "lldk/base/common.h"

namespace lldk
{
namespace thread
{

class IThread
{
protected:
    virtual ~IThread() = default;

public:
    enum class ThreadState
    {
        kUnknown = 0,
        kInit,
        kRunning,
        kStopped,
        kBlocking,
    };

    /**
     * @brief The thread function type
     * @param pArg The argument of the thread function
     * @return void
     */
    using ThreadFunc = void (*)(void *pArg);

    /**
     * @brief Start the thread
     * @param pFunc The thread function
     * @param pArg The argument of the thread function
     * @return 0 if success, -1 if failed
     */
    virtual int32_t start(ThreadFunc pFunc, void *pArg) = 0;

    /**
     * @brief Stop the thread
     * @return 0 if success, -1 if failed
     */
    virtual int32_t stop() = 0;

    /**
     * @brief Get the last execution time of the thread
     * @return The last execution time of the thread
     */
    virtual uint64_t getLastExecTime() const = 0;

    /**
     * @brief Get the name of the thread
     * @return The name of the thread
     */
    virtual const char *getName() const = 0;

    /**
     * @brief Check if the thread is running
     * @return true if running, false if not
     */
    bool isRunning() const 
    {
        return LLDK_ACCESS_ONCE(m_eState) == ThreadState::kRunning 
                || LLDK_ACCESS_ONCE(m_eState) == ThreadState::kBlocking;
    }

    /**
     * @brief Get the state of the thread
     * @return The state of the thread
     */
    ThreadState getState() const 
    {
        return LLDK_ACCESS_ONCE(m_eState); 
    }

private:
    ThreadState m_eState = ThreadState::kUnknown;
};

class IThreadManager
{
protected:
    virtual ~IThreadManager() = default;

public:
    struct ThreadInfo
    {
        const char *pName;
        IThread::ThreadState eState;
        uint64_t uLastExecTime;
    };

    /**
     * @brief Create a thread
     * @param name The thread name
     * @return The thread pointer, NULL if failed
     */
    virtual IThread *createThread(const char *name) = 0;

    /**
     * @brief Destroy a thread
     * @param pThread The thread pointer
     */
    virtual void destroyThread(IThread *pThread) = 0;

    /**
     * @brief Set the thread local storage
     * @param pStorage The thread local storage
     * @return The thread local storage id, -1 if failed
     */
    virtual int64_t setThreadLocalStorage(void *pStorage) = 0;

    /**
     * @brief Clear the thread local storage
     * @param iThreadLocalStorageId The thread local storage id
     */
    virtual void clearThreadLocalStorage(int64_t iThreadLocalStorageId) = 0;

    /**
     * @brief Get the thread local storage
     * @param iThreadLocalStorageId The thread local storage id
     * @return The thread local storage, NULL if failed
     */
    virtual void *getThreadLocalStorage(int64_t iThreadLocalStorageId) const = 0;

    /**
     * @brief Get all threads info
     * @param pCount The count of the threads, output parameter
     * @return The threads info array pointer, NULL if failed
     */
    virtual ThreadInfo *getAllThreadInfo(uint32_t *pCount) const = 0;
};

}
}

/**
 * @brief Get the process id
 * @return The process id, -1 if failed
 */
LLDK_EXTERN_C int64_t lldkGetPid();

/**
 * @brief Get the thread id
 * @return The thread id, -1 if failed
 */
LLDK_EXTERN_C int64_t lldkGetTid();

/**
 * @brief Set the thread name
 * @param pName The thread name
 * @return 0 if success, -1 if failed
 */
LLDK_EXTERN_C int32_t lldkSetThreadName(const char *pName);

/**
 * @brief Bind the thread to the cpu
 * @param iCpuId The cpu id
 * @return 0 if success, -1 if failed
 */
LLDK_EXTERN_C int32_t lldkBindCpu(uint32_t iCpuId);

/**
 * @brief Create a thread
 * @param pName The thread name
 * @return The thread pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::IThread *lldkCreateThread(const char *pName);

/**
 * @brief Destroy a thread
 * @param pThread The thread pointer
 */
LLDK_EXTERN_C void lldkDestroyThread(lldk::thread::IThread *pThread);

/**
 * @brief Create a thread
 * @param pName The thread name
 * @return The thread pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::IThread *lldkCreateThread(const char *pName);

/**
 * @brief Destroy the thread manager singleton
 * @param pThreadManager The thread manager pointer
 */
LLDK_EXTERN_C void lldkDestroyThreadManager(lldk::thread::IThreadManager *pThreadManager);

/**
 * @brief Get the thread manager singleton
 * @return The thread manager pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::IThreadManager *lldkGetThreadManagerSingleton();

#endif // LLDK_THREAD_THREAD_H
