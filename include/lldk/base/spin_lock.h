#ifndef LLDK_THREAD_SPIN_LOCK_H
#define LLDK_THREAD_SPIN_LOCK_H

#include "lldk/common/common.h"

namespace lldk
{
namespace thread
{

class ISpinLock
{
protected:
    virtual ~ISpinLock() = default;

public:
    /**
     * @brief Lock the spin lock
     */
    virtual void lock() = 0;

    /**
     * @brief Unlock the spin lock
     */
    virtual void unlock() = 0;

    /**
     * @brief Try to lock the spin lock
     * @return true if success, false if failed
     */
    virtual bool tryLock() = 0;
};

class SpinLockGuard
{
public:
    SpinLockGuard(ISpinLock &spinLock) : m_spinLock(spinLock) { m_spinLock.lock(); }
    ~SpinLockGuard() { m_spinLock.unlock(); }

private:
    ISpinLock &m_spinLock;
};

}
}

/**
 * @brief Create a spin lock
 * @return The spin lock pointer, NULL if failed
 */
LLDK_EXTERN_C lldk::thread::ISpinLock *lldkCreateSpinLock();

/**
 * @brief Destroy a spin lock
 * @param pSpinLock The spin lock pointer
 */
LLDK_EXTERN_C void lldkDestroySpinLock(lldk::thread::ISpinLock *pSpinLock);

#endif // LLDK_THREAD_SPIN_LOCK_H
