#include "allocator_impl.h"
#include "lldk/base/allocator.h"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "lldk/common/common.h"
#include "lldk/common/error_code.h"

namespace lldk
{
namespace base
{

AllocatorImpl::AllocatorImpl(const char *pName) : m_sName(pName) {}

AllocatorImpl::~AllocatorImpl() = default;

void *AllocatorImpl::allocate(uint64_t uSize)
{
    auto pData = malloc(uSize + sizeof(uint64_t));
    if (unlikely(pData == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
        return nullptr;
    }

    *((uint64_t *)pData) = uSize;
    pData = (void *)((uint8_t *)pData + sizeof(uint64_t));

    auto pAllocateStats = m_threadLocalAllocateStats.get();
    if (likely(pAllocateStats != nullptr))
    {
        pAllocateStats->uAllocatedSize += uSize;
        pAllocateStats->uAllocatedCount++;
    }

    return pData;
}

void AllocatorImpl::free(void *pMemory)
{
    if (unlikely(pMemory == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return;
    }

    auto pAllocateStats = m_threadLocalAllocateStats.get();
    if (likely(pAllocateStats != nullptr))
    {
        pAllocateStats->uFreedSize += *((uint64_t *)pMemory);
        pAllocateStats->uFreedCount++;
    }

    free(pMemory);
}

int32_t AllocatorImpl::getAllocateStats(IAllocator::AllocateStats *pAllocateStats, uint32_t *pThreadCount) const
{
    if (unlikely( pAllocateStats == nullptr || pThreadCount == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return -1;
    }

    uint32_t uThreadCount = 0;
    uint32_t uThreadMaxSize = *pThreadCount;
    auto func = [&uThreadCount, uThreadMaxSize, pAllocateStats](AllocateStats *pStats) {
        if (likely(uThreadCount < uThreadMaxSize))
        {
            pAllocateStats[uThreadCount++] = *pStats;
        }
    };

    m_threadLocalAllocateStats.foreach(func);
    *pThreadCount = uThreadCount;
    return 0;
}

}
}

static void deleteAllocatorImpl(lldk::base::AllocatorImpl *pAllocator)
{
    auto pAllocatorSingleton = lldkGetAllocatorSingleton();
    if (likely(pAllocatorSingleton != nullptr))
    {
        pAllocatorSingleton->deleteObject(pAllocator);
    }
    else
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidState);
    }
}

static std::mutex s_mutex;
static std::unordered_map<std::string, std::unique_ptr<lldk::base::AllocatorImpl, decltype(&deleteAllocatorImpl)>> s_pAllocatorMap;

lldk::base::IAllocator *lldkCreateAllocator(const char *pName, uint64_t uMaxSizeMB)
{
    if (unlikely(pName == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto iter = s_pAllocatorMap.find(pName);
        if (likely(iter != s_pAllocatorMap.end()))
        {
            lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
            return nullptr;
        }
    }

    auto pAllocatorSingleton = lldkGetAllocatorSingleton();
    if (likely(pAllocatorSingleton != nullptr))
    {
        auto pAllocator = pAllocatorSingleton->newObject<lldk::base::AllocatorImpl>(pName);
        std::unique_ptr<lldk::base::AllocatorImpl, decltype(&deleteAllocatorImpl)> upAllocator(pAllocator, deleteAllocatorImpl);
        if (unlikely(pAllocator == nullptr))
        {
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return nullptr;
        }

        if (unlikely(pAllocator->init(uMaxSizeMB) != 0))
        {
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return nullptr;
        }

        try
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_pAllocatorMap[pName] = std::move(upAllocator);
        }
        catch (...)
        {
            lldkSetErrorCode(lldk::ErrorCode::kThrowException);
            return nullptr;
        }

        return pAllocator;
    }

    lldkSetErrorCode(lldk::ErrorCode::kInvalidState);
    return nullptr;
}

void lldkDestroyAllocator(lldk::base::IAllocator *pAllocator)
{
    auto pAllocatorImpl = dynamic_cast<lldk::base::AllocatorImpl *>(pAllocator);
    if (likely(pAllocatorImpl != nullptr))
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto pName = pAllocatorImpl->getName();
        auto iter = s_pAllocatorMap.find(pName);
        if (likely(iter != s_pAllocatorMap.end() || pAllocatorImpl == iter->second.get()))
        {
            s_pAllocatorMap.erase(iter);
        }
    }

    lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
    return;
}

// global allocator singleton can print in gdb
static lldk::base::AllocatorImpl *s_pAllocator = nullptr;

lldk::base::IAllocator *lldkGetAllocatorSingleton()
{
    static std::mutex s_mutex;

    if (likely(s_pAllocator != nullptr))
    {
        return s_pAllocator;
    }
    
    std::lock_guard<std::mutex> lock(s_mutex);
    if (unlikely(s_pAllocator == nullptr))
    {
        s_pAllocator = new lldk::base::AllocatorImpl("lldk.global.allocator");
        if (unlikely(s_pAllocator == nullptr))
        {
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return nullptr;
        }

        if (unlikely(s_pAllocator->init(0) != 0))
        {
            delete s_pAllocator;
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return nullptr;
        }
    }

    return s_pAllocator;
}

