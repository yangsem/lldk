#ifndef LLDK_BASE_ALLOCATOR_IMPL_H
#define LLDK_BASE_ALLOCATOR_IMPL_H

#include "lldk/base/allocator.h"
#include <mutex>
#include <string>
#include "../utilities/lldk_thread_local.h"

namespace lldk
{
namespace base
{

class AllocatorImpl : public IAllocator
{
public:
    AllocatorImpl(const char *pName);
    ~AllocatorImpl() override;

    void *allocate(uint64_t uSize) override;
    void free(void *pMemory) override;
    void *reAllocate(void *pMemory, uint64_t uSize) override;
    const char *getName() const override;
    int32_t getAllocateStats(IAllocator::AllocateStats *pAllocateStats, uint32_t *pThreadCount) const override;

    int32_t init(uint64_t uMaxSizeMB);

private:
    static AllocateStats *createAllocateStats();
    static void deleteAllocateStats(AllocateStats *pAllocateStats);
    
    // 函数对象类型，用于 LldkThreadLocal 模板
    struct CreateAllocateStatsFunc
    {
        AllocateStats *operator()() const
        {
            return AllocatorImpl::createAllocateStats();
        }
    };
    
    struct DeleteAllocateStatsFunc
    {
        void operator()(AllocateStats *pStats) const
        {
            AllocatorImpl::deleteAllocateStats(pStats);
        }
    };
    
    using AllocatorThreadLocal = utilities::LldkThreadLocal<AllocateStats>;

private:
    std::string m_sName;
    uint64_t m_uMaxSizeMB{0};
    mutable std::mutex m_mutex;
    AllocatorThreadLocal m_allocatorThreadLocal;
};

}
}

#endif // LLDK_BASE_ALLOCATOR_IMPL_H
