#include "lldk_thread_local.h"
#include "lldk/common/error_code.h"

namespace lldk
{
namespace utilities
{

template <typename T, uint32_t kInstanceIdSize, typename CreateFunc, typename DestroyFunc>
thread_local T **LldkThreadLocal<T, kInstanceIdSize, CreateFunc, DestroyFunc>::m_ppInstances = nullptr;

template <typename T, uint32_t kInstanceIdSize, typename CreateFunc, typename DestroyFunc>
LldkThreadLocal<T, kInstanceIdSize, CreateFunc, DestroyFunc>::LldkThreadLocal()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto uInstanceId = m_bitset.findFirstNone();
    if (unlikely(uInstanceId == kInstanceIdSize))
    {
        throw std::runtime_error("Failed to create instance id");
    }
    m_bitset.set(uInstanceId);
    m_uInstanceId = uInstanceId;
}

template <typename T, uint32_t kInstanceIdSize, typename CreateFunc, typename DestroyFunc>
LldkThreadLocal<T, kInstanceIdSize, CreateFunc, DestroyFunc>::~LldkThreadLocal()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto ppInstance : m_vecppInstances)
    {
        if (ppInstance != nullptr && ppInstance[m_uInstanceId] != nullptr)
        {
            DestroyFunc()(ppInstance[m_uInstanceId]);
            ppInstance[m_uInstanceId] = nullptr;
        }
    }
    m_bitset.clear(m_uInstanceId);
}

template <typename T, uint32_t kInstanceIdSize, typename CreateFunc, typename DestroyFunc>
T *LldkThreadLocal<T, kInstanceIdSize, CreateFunc, DestroyFunc>::get()
{
    if (unlikely(m_ppInstances == nullptr))
    {
        m_ppInstances = LLDK_NEW T *[kInstanceIdSize];
        if (unlikely(m_ppInstances == nullptr))
        {
            lldkSetErrorCode(lldk::ErrorCode::kNoMemory);
            return nullptr;
        }
    
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_vecppInstances.push_back(&m_ppInstances);
        }
        catch (...)
        {
            delete[] m_ppInstances;
            m_ppInstances = nullptr;
            lldkSetErrorCode(lldk::ErrorCode::kThrowException);
            return nullptr;
        }
    }

    auto &pInstance = m_ppInstances[m_uInstanceId];
    if (unlikely(pInstance == nullptr))
    {
        pInstance = CreateFunc()();
        if (unlikely(pInstance == nullptr))
        {
            lldkSetErrorCode(lldk::ErrorCode::kCallFailed);
            return nullptr;
        }
    }

    return pInstance;
}

template <typename T, uint32_t kInstanceIdSize, typename CreateFunc, typename DestroyFunc>
int32_t LldkThreadLocal<T, kInstanceIdSize, CreateFunc, DestroyFunc>::foreach(std::function<void(T *)> func) const
{
    if (unlikely(func == nullptr))
    {
        lldkSetErrorCode(lldk::ErrorCode::kInvalidParam);
        return -1;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto ppInstance : m_vecppInstances)
    {
        if (ppInstance != nullptr && ppInstance[m_uInstanceId] != nullptr)
        {
            func(ppInstance[m_uInstanceId]);
        }
    }

    return 0;
}

}
}