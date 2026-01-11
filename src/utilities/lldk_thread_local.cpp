#include "lldk_thread_local.h"
#include "../utilities/lldk_bitset.h"
#include "lldk/common/error_code.h"
#include <vector>
#include <mutex>

namespace lldk
{
namespace utilities
{

static thread_local void **s_ppInstances = nullptr;

static std::mutex s_mutex;
static LldkBitset<LldkThreadLocalBase::kMaxInstanceId> s_bitset;
static std::vector<void **> s_vecpppInstances;

uint32_t LldkThreadLocalBase::newInstanceId()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto uInstanceId = s_bitset.findFirstNone();
    if (unlikely(uInstanceId == kMaxInstanceId))
    {
        return kInvalidInstanceId;
    }
    s_bitset.set(uInstanceId);
    return uInstanceId;
}

void LldkThreadLocalBase::deleteInstanceId(uint32_t uInstanceId)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_bitset.clear(uInstanceId);
}

int32_t LldkThreadLocalBase::setThreadLocalStorage(uint32_t uInstanceId, void *pStorage)
{
    if (unlikely(uInstanceId == kInvalidInstanceId || pStorage == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return -1;
    }
    
    if (unlikely(s_ppInstances == nullptr))
    {
        s_ppInstances = (void **)LldkThreadLocalBase::lldkAllocate(sizeof(void *) * kMaxInstanceId);
        if (unlikely(s_ppInstances == nullptr))
        {
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return -1;
        }
        memset(s_ppInstances, 0, sizeof(void *) * kMaxInstanceId);

        try
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_vecpppInstances.push_back(s_ppInstances);
        }
        catch (...)
        {
            LldkThreadLocalBase::lldkFree(s_ppInstances);
            s_ppInstances = nullptr;
            lldkSetErrorCode(lldk::ErrorCode::kThrowException);
            return -1;
        }
    }

    s_ppInstances[uInstanceId] = pStorage;
    return 0;
}

void *LldkThreadLocalBase::getThreadLocalStorage(uint32_t uInstanceId)
{
    return s_ppInstances[uInstanceId];
}

int32_t LldkThreadLocalBase::foreach(uint32_t uInstanceId, std::function<int32_t(void *)> func)
{
    if (unlikely(func == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return -1;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto ppInstance : s_vecpppInstances)
    {
        if (ppInstance != nullptr && ppInstance[uInstanceId] != nullptr)
        {
            if (unlikely(func(ppInstance[uInstanceId]) != 0))
            {
                lldkSetErrorCode(lldk::ErrorCode::kCallFailed);
                return -1;
            }
        }
    }
    return 0;
}

LLDK_EXTERN_C void *__lldkAllocate(uint64_t uSize);
LLDK_EXTERN_C void __lldkFree(void *pMemory);

void *LldkThreadLocalBase::lldkAllocate(uint64_t uSize)
{
    return __lldkAllocate(uSize);
}

void LldkThreadLocalBase::lldkFree(void *pMemory)
{
    __lldkFree(pMemory);
}

}
}