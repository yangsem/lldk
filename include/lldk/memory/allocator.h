#ifndef LLDK_MEMORY_ALLOCATOR_H
#define LLDK_MEMORY_ALLOCATOR_H

#include "lldk/common/common.h"

namespace lldk
{
namespace memory
{

class IAllocator
{
protected:
    virtual ~IAllocator() = default;

public:
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
 * @return The allocator pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::memory::IAllocator *lldkCreateAllocator(const char *pName);

/**
 * @brief Destroy a allocator
 * @param pAllocator The allocator pointer
 */
LLDK_EXTERN_C void lldkDestroyAllocator(lldk::memory::IAllocator *pAllocator);

/**
 * @brief Get the allocator singleton
 * @return The allocator pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::memory::IAllocator *lldkGetAllocatorSingleton();

#endif // LLDK_MEMORY_ALLOCATOR_H
