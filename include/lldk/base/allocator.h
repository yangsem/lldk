#ifndef LLDK_MEMORY_ALLOCATOR_H
#define LLDK_MEMORY_ALLOCATOR_H

#include "lldk/common/common.h"

namespace lldk
{
namespace base
{

class IAllocator
{
protected:
    virtual ~IAllocator() = default;

public:
    struct AllocateStats
    {
        uint64_t uTid;            // The thread id
        uint64_t uAllocatedSize;  // The total bytes size of the allocated memory
        uint64_t uAllocatedCount; // The total count of the allocated memory
        uint64_t uFreedSize;      // The total bytes size of the freed memory
        uint64_t uFreedCount;     // The total count of the freed memory
    };

    /**
     * @brief Allocate memory
     * @param uSize The size of the memory to allocate
     * @return The pointer to the allocated memory, NULL if failed
     */
    virtual void *allocate(uint64_t uSize) = 0;

    /**
     * @brief Free memory
     * @param pMemory The pointer to the memory to free
     */
    virtual void free(void *pMemory) = 0;

    /**
     * @brief Reallocate memory
     * @param pMemory The pointer to the memory to reallocate
     * @param uSize The size of the memory to reallocate
     * @return The pointer to the reallocated memory, NULL if failed
     */
    virtual void *reAllocate(void *pMemory, uint64_t uSize) = 0;

    /**
     * @brief Get the name of the allocator
     * @return The name of the allocator
     */
    virtual const char *getName() const = 0;

    /**
     * @brief Get the allocate stats of the allocator
     * @param pAllocateStats The allocate stats of the allocator, output parameter
     * @param pThreadCount The count of the threads, output parameter
     * @return The allocate stats of the allocator, NULL if failed
     */
    virtual int32_t getAllocateStats(IAllocator::AllocateStats *pAllocateStats, uint32_t *pThreadCount) const = 0;

    /**
     * @brief Create a new object
     * @tparam T The type of the object
     * @tparam Args The types of the arguments
     * @param args The arguments
     * @return The pointer to the new object, NULL if failed
     */
    template <typename T, typename... Args>
    T *newObject(Args&&... args)
    {
        void *pMemory = allocate(sizeof(T));
        if (pMemory == NULL)
        {
            return NULL;
        }

        try
        {
            T *pObject = new(pMemory) T(std::forward<Args&&>(args)...);
            return pObject;
        }
        catch (...)
        {
            free(pMemory);
            throw;
        }

        return NULL;
    }

    /**
     * @brief Delete an object
     * @tparam T The type of the object
     * @param pObject The pointer to the object
     */
    template <typename T>
    void deleteObject(T *pObject)
    {
        pObject->~T();
        free(pObject);
    }
};

}
}

/**
 * @brief Create a allocator
 * @param pName The name of the allocator
 * @param uMaxSizeMB The maximum size of the allocator, in MB, 0 means no limit
 * @return The allocator pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::base::IAllocator *lldkCreateAllocator(const char *pName, uint64_t uMaxSizeMB);

/**
 * @brief Destroy a allocator
 * @param pAllocator The allocator pointer
 */
LLDK_EXTERN_C void lldkDestroyAllocator(lldk::base::IAllocator *pAllocator);

/**
 * @brief Get the allocator singleton
 * @return The allocator pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::base::IAllocator *lldkGetAllocatorSingleton();

#endif // LLDK_MEMORY_ALLOCATOR_H
