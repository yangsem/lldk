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

    auto pAllocateStats = m_allocatorThreadLocal.get();
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

    void *pOriginalMemory = (void *)((uint8_t *)pMemory - sizeof(uint64_t));
    uint64_t uSize = *((uint64_t *)pOriginalMemory);

    auto pAllocateStats = m_allocatorThreadLocal.get();
    if (likely(pAllocateStats != nullptr))
    {
        pAllocateStats->uFreedSize += uSize;
        pAllocateStats->uFreedCount++;
    }

    ::free(pOriginalMemory);
}

void *AllocatorImpl::reAllocate(void *pMemory, uint64_t uSize)
{
    if (unlikely(pMemory == nullptr))
    {
        return allocate(uSize);
    }

    void *pOriginalMemory = (void *)((uint8_t *)pMemory - sizeof(uint64_t));
    uint64_t uOldSize = *((uint64_t *)pOriginalMemory);

    void *pNewMemory = realloc(pOriginalMemory, uSize + sizeof(uint64_t));
    if (unlikely(pNewMemory == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
        return nullptr;
    }

    *((uint64_t *)pNewMemory) = uSize;
    void *pResult = (void *)((uint8_t *)pNewMemory + sizeof(uint64_t));

    auto pAllocateStats = m_allocatorThreadLocal.get();
    if (likely(pAllocateStats != nullptr))
    {
        pAllocateStats->uFreedSize += uOldSize;
        pAllocateStats->uFreedCount++;
        pAllocateStats->uAllocatedSize += uSize;
        pAllocateStats->uAllocatedCount++;
    }

    return pResult;
}

const char *AllocatorImpl::getName() const
{
    return m_sName.c_str();
}

int32_t AllocatorImpl::init(uint64_t uMaxSizeMB)
{
    m_uMaxSizeMB = uMaxSizeMB;
    return 0;
}

AllocatorImpl::AllocateStats *AllocatorImpl::createAllocateStats()
{
    auto pStats = new AllocateStats();
    if (unlikely(pStats == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
        return nullptr;
    }
    memset(pStats, 0, sizeof(AllocateStats));
    pStats->uTid = lldkGetTid();
    return pStats;
}

void AllocatorImpl::deleteAllocateStats(AllocateStats *pAllocateStats)
{
    if (likely(pAllocateStats != nullptr))
    {
        delete pAllocateStats;
    }
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
        return 0;
    };

    m_allocatorThreadLocal.foreach(func);
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

using AllocatorUniqueptr = std::unique_ptr<lldk::base::AllocatorImpl, decltype(&deleteAllocatorImpl)>;
static std::mutex s_mutex;
static std::unordered_map<std::string, AllocatorUniqueptr> s_pAllocatorMap;
static lldk::base::AllocatorImpl::AllocateStats s_allocateStats {0, 0, 0, 0, 0};

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
        AllocatorUniqueptr allocatorUniqueptr(pAllocator, deleteAllocatorImpl);
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
            s_pAllocatorMap.emplace(pName, std::move(allocatorUniqueptr));
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
        if (likely(iter != s_pAllocatorMap.end() && pAllocatorImpl == iter->second.get()))
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

#ifdef __cplusplus
extern "C" {
#endif
LLDK_EXPORT void *__lldkAllocate(uint64_t uSize)
{
    auto pData = ::malloc(uSize + sizeof(uint64_t));
    if (unlikely(pData == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
        return nullptr;
    }

    *((uint64_t *)pData) = uSize;
    pData = (void *)((uint8_t *)pData + sizeof(uint64_t));

    std::lock_guard<std::mutex> lock(s_mutex);
    s_allocateStats.uAllocatedSize += uSize;
    s_allocateStats.uAllocatedCount++;

    return pData;
}

LLDK_EXPORT void __lldkFree(void *pMemory)
{
    if (unlikely(pMemory == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return;
    }

    void *pOriginalMemory = (void *)((uint8_t *)pMemory - sizeof(uint64_t));
    uint64_t uSize = *((uint64_t *)pOriginalMemory);

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_allocateStats.uFreedSize += uSize;
        s_allocateStats.uFreedCount++;
    }

    ::free(pOriginalMemory);
}

LLDK_EXPORT int32_t __lldkGetAllocateStats(lldk::base::IAllocator::AllocateStats *pAllocateStats)
{
    if (unlikely(pAllocateStats == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return -1;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    *pAllocateStats = s_allocateStats;
    return 0;
}
#ifdef __cplusplus
}
#endif
