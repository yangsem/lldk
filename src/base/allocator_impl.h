#ifndef LLDK_BASE_ALLOCATOR_IMPL_H
#define LLDK_BASE_ALLOCATOR_IMPL_H

#include "lldk/base/allocator.h"
#include <mutex>
#include <string>
#include <vector>
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
    using CreateAllocateStatsFunc = std::function<AllocateStats *()>;
    using DeleteAllocateStatsFunc = std::function<void(AllocateStats *)>;
    using ThreadLocalAllocateStats = utilities::LldkThreadLocal<AllocateStats, 1024, CreateAllocateStatsFunc, DeleteAllocateStatsFunc>;

private:
    std::string m_sName;
    uint64_t m_uMaxSizeMB{0};
    mutable std::mutex m_mutex;
    ThreadLocalAllocateStats m_threadLocalAllocateStats;
};

}
}

#endif // LLDK_BASE_ALLOCATOR_IMPL_H
