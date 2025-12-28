#include "lldk/common/common.h"

#ifdef LLDK_OS_LINUX
#include <sys/syscall.h>
#include <unistd.h>
int64_t lldkGetTid()
{
    return (int64_t)syscall(SYS_gettid);
}
int64_t lldkGetPid()
{
    return (int64_t)getpid();
}
#elif defined(LLDK_OS_APPLE)
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
int64_t lldkGetTid()
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (int64_t)tid;
}
int64_t lldkGetPid()
{
    return (int64_t)getpid();
}
#elif defined(LLDK_OS_WINDOWS)
#include <windows.h>
int64_t lldkGetTid()
{
    return (int64_t)GetCurrentThreadId();
}
int64_t lldkGetPid()
{
    return (int64_t)GetProcessId(GetCurrentProcess());
}
#endif
