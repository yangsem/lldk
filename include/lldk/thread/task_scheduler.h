#ifndef LLDK_THREAD_TASK_SCHEDULER_H
#define LLDK_THREAD_TASK_SCHEDULER_H

#include "lldk/base/common.h"

namespace lldk
{
namespace thread
{

class ITaskScheduler
{
protected:
    virtual ~ITaskScheduler() = default;

public:
    /**
     * @brief The task function type
     * @param pArg The argument of the task function
     * @return void
     */
    using TaskFunc = void (*)(void *pArg);

    struct TaskInfo
    {
        const char *pName;    // The name of the task
        TaskFunc pFunc;       // The task function
        void *pArg;           // The argument of the task function
        uint64_t uDelayMs;    // The delay time in milliseconds, 0 means immediate execution
        uint64_t uIntervalMs; // The interval time in milliseconds
        uint64_t uExecTimes;  // The number of times the task need to be executed
        uint64_t uExecCount;  // The number of times the task has been executed successfully
    };

    /**
     * @brief Start the task scheduler
     * @return 0 if success, -1 if failed
     */
    virtual int32_t start() = 0;

    /**
     * @brief Stop the task scheduler
     * @return 0 if success, -1 if failed
     */
    virtual int32_t stop() = 0;

    /**
     * @brief Add a task
     * @param pTaskInfo The task info
     * @return The task id, -1 if failed
     */
    virtual int64_t addTask(TaskInfo *pTaskInfo) = 0;

    /**
     * @brief Pause a task
     * @param iTaskId The task id
     * @return 0 if success, -1 if failed
     */
    virtual int32_t pauseTask(int64_t iTaskId) = 0;

    /**
     * @brief Resume a task
     * @param iTaskId The task id
     * @return 0 if success, -1 if failed
     */
    virtual int32_t resumeTask(int64_t iTaskId) = 0;

    /**
     * @brief Remove a task
     * @param iTaskId The task id
     * @return 0 if success, -1 if failed
     */
    virtual int32_t removeTask(int64_t iTaskId) = 0;

    /**
     * @brief Get all task info
     * @param pCount The count of the tasks, output parameter
     * @return The task info array pointer, NULL if failed
     */
    virtual TaskInfo *getAllTaskInfo(uint32_t *pCount) const = 0;
};

}
}

/**
 * @brief Create a task scheduler
 * @param pName The name of the task scheduler
 * @return The task scheduler pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::ITaskScheduler *lldkCreateTaskScheduler(const char *pName);

/**
 * @brief Destroy a task scheduler
 * @param pTaskScheduler The task scheduler pointer
 */
LLDK_EXTERN_C void lldkDestroyTaskScheduler(lldk::thread::ITaskScheduler *pTaskScheduler);

/**
 * @brief Get the task scheduler singleton
 * @return The task scheduler pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::ITaskScheduler *lldkGetTaskSchedulerSingleton();

#endif // LLDK_THREAD_TASK_SCHEDULER_H
