#ifndef LLDK_UTILITIES_LLDK_THREAD_LOCAL_H
#define LLDK_UTILITIES_LLDK_THREAD_LOCAL_H

#include <mutex>
#include <functional>
#include "lldk/common/common.h"
#include "lldk_bitset.h"

namespace lldk
{
namespace utilities
{

template <typename T, uint32_t kInstanceIdSize = 1024, 
          typename CreateFunc = std::function<T *()>, 
          typename DestroyFunc = std::function<void(T *)>>
class LldkThreadLocal
{
public:
    LldkThreadLocal();
    ~LldkThreadLocal();

    T *get();

    int32_t foreach(std::function<void(T *)> func) const;

private:
    uint32_t m_uInstanceId{0};

    static thread_local T **m_ppInstances;

    static std::mutex m_mutex;
    static LldkBitset<kInstanceIdSize> m_bitset;
    static std::vector<T **> m_vecppInstances;
};

}
}
#endif // LLDK_UTILITIES_LLDK_THREAD_LOCAL_H
