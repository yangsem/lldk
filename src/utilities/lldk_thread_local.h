#ifndef LLDK_UTILITIES_LLDK_THREAD_LOCAL_H
#define LLDK_UTILITIES_LLDK_THREAD_LOCAL_H

#include "lldk/common/common.h"
#include <functional>
#include <stdexcept>

namespace lldk
{
namespace utilities
{

class LldkThreadLocalBase
{
    LldkThreadLocalBase() = default;
    ~LldkThreadLocalBase() = default;

public:
    /**
     * @brief The maximum instance id
     */
    static constexpr uint32_t kMaxInstanceId = 2048;

    static constexpr uint32_t kInvalidInstanceId = kMaxInstanceId;

    /**
     * @brief Create a new instance id
     * @return The new instance id
     */
    static uint32_t newInstanceId();

    /**
     * @brief Delete an instance id
     * @param uInstanceId The instance id to delete
     */
    static void deleteInstanceId(uint32_t uInstanceId);

    /**
     * @brief Set the thread local storage
     * @param uInstanceId The instance id
     * @param pStorage The thread local storage
     * @return 0 if success, -1 if failed
     */
    static int32_t setThreadLocalStorage(uint32_t uInstanceId, void *pStorage);

    /**
     * @brief Get the thread local storage
     * @param uInstanceId The instance id
     * @return The thread local storage
     */
    static void *getThreadLocalStorage(uint32_t uInstanceId);

    /**
     * @brief Foreach the all thread local storages of the instance id
     * @param uInstanceId The instance id
     * @param func The function to be called for each thread local storage, return 0 continue, otherwise stop
     * @return 0 if success, nonzero if failed, if func return nonzero stop the foreach and foreach return 0
     */
    static int32_t foreach(uint32_t uInstanceId, std::function<int32_t(void *)> func);

    /**
     * @brief Allocate memory
     * @param uSize The size of the memory to allocate
     * @return The pointer to the allocated memory, NULL if failed
     */
    static void *lldkAllocate(uint64_t uSize);

    /**
     * @brief Free memory
     * @param pMemory The pointer to the memory to free
     */
    static void lldkFree(void *pMemory);
};

template <typename T>
class LldkThreadLocal
{
public:
    LldkThreadLocal(const LldkThreadLocal &) = delete;
    LldkThreadLocal &operator=(const LldkThreadLocal &) = delete;

    /**
     * @brief Constructor
     */
    LldkThreadLocal()
    {
        m_uInstanceId = LldkThreadLocalBase::newInstanceId();
        if (m_uInstanceId == LldkThreadLocalBase::kInvalidInstanceId)
        {
            throw std::runtime_error("Failed to create instance id");
        }

        auto pStorage = LldkThreadLocalBase::lldkAllocate(sizeof(T));
        if (unlikely(pStorage == nullptr))
        {
            throw std::runtime_error("Failed to create storage");
        }

        new(pStorage) T();

        if (unlikely(LldkThreadLocalBase::setThreadLocalStorage(m_uInstanceId, pStorage) != 0))
        {
            LldkThreadLocalBase::lldkFree(pStorage);
            LldkThreadLocalBase::deleteInstanceId(m_uInstanceId);
            m_uInstanceId = LldkThreadLocalBase::kInvalidInstanceId;
            throw std::runtime_error("Failed to set thread local storage");
        }
    }

    LldkThreadLocal(LldkThreadLocal &&that)
    {
        if (likely(this != &that))
        {
            m_uInstanceId = that.m_uInstanceId;
            that.m_uInstanceId = LldkThreadLocalBase::kInvalidInstanceId;
        }
    }

    LldkThreadLocal &operator=(LldkThreadLocal &&that)
    {
        if (likely(this != &that))
        {
            m_uInstanceId = that.m_uInstanceId;
            that.m_uInstanceId = LldkThreadLocalBase::kInvalidInstanceId;
        }

        return *this;
    }

    /**
     * @brief Destructor
     */
    ~LldkThreadLocal()
    {
        auto pStorage = LldkThreadLocalBase::getThreadLocalStorage(m_uInstanceId);
        if (likely(pStorage != nullptr))
        {
            static_cast<T *>(pStorage)->~T();
            LldkThreadLocalBase::lldkFree(pStorage);
            LldkThreadLocalBase::setThreadLocalStorage(m_uInstanceId, nullptr);
        }
        LldkThreadLocalBase::deleteInstanceId(m_uInstanceId);
        m_uInstanceId = LldkThreadLocalBase::kInvalidInstanceId;
    }

    /**
     * @brief Get the thread local storage
     * @return The thread local storage
     */
    T *get() const
    {
        return reinterpret_cast<T *>(LldkThreadLocalBase::getThreadLocalStorage(m_uInstanceId));
    }

    /**
     * @brief Foreach the all thread local storages of the instance id
     * @param uInstanceId The instance id
     * @param func The function to be called for each thread local storage, return 0 continue, otherwise stop
     * @return 0 if success, nonzero if failed, if func return nonzero stop the foreach and foreach return 0
     */
    int32_t foreach(std::function<int32_t(T *)> func) const
    {
        if (likely(m_uInstanceId != LldkThreadLocalBase::kInvalidInstanceId && func != nullptr))
        {
            return LldkThreadLocalBase::foreach(m_uInstanceId, [func](void *pStorage) {
                return func(static_cast<T *>(pStorage));
            });
        }

        return -1;
    }

private:
    uint32_t m_uInstanceId{LldkThreadLocalBase::kInvalidInstanceId};
};

}
}
#endif // LLDK_UTILITIES_LLDK_THREAD_LOCAL_H
