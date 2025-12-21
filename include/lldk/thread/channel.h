#ifndef LLDK_THREAD_CHANNEL_H
#define LLDK_THREAD_CHANNEL_H

#include "lldk/common/common.h"

namespace lldk
{
namespace thread
{

class IChannel
{
protected:
    virtual ~IChannel() = default;

public:
    /**
     * @brief Create a new entry
     * @return The entry pointer, NULL if failed
     */
    virtual void *newEntry() = 0;

    /**
     * @brief Post an entry
     * @param pEntry The entry pointer
     */
    virtual void postEntry(void *pEntry) = 0;

    /**
     * @brief Create a new entry with a size
     * @param uSize The size of the entry
     * @return The entry pointer, NULL if failed
     */
    virtual void *newEntry(uint64_t uSize) = 0;

    /**
     * @brief Post an entry with a size
     * @param pEntry The entry pointer
     * @param uSize The size of the entry
     */
    virtual void postEntry(void *pEntry, uint64_t uSize) = 0;

    /**
     * @brief Get an entry
     * @return The entry pointer, NULL if failed
     */
    virtual void *getEntry() = 0;

    /**
     * @brief Free an entry
     * @param pEntry The entry pointer
     */
    virtual void freeEntry(void *pEntry) = 0;

    /**
     * @brief Check if the channel is empty
     * @return true if empty, false if not
     */
    virtual bool isEmpty() = 0;

    /**
     * @brief Check if the channel is full
     * @return true if full, false if not
     */
    virtual bool isFull() = 0;

    /**
     * @brief Get the size of the channel
     * @return The size of the channel
     */
    virtual uint64_t getSize() = 0;

    /**
     * @brief Get the capacity of the channel
     * @return The capacity of the channel
     */
    virtual uint64_t getCapacity() = 0;

    virtual void clear() = 0;
};

enum class ChannelType : uint32_t
{
    kUnknown = 0,
    kSPSCFixedBounded,
    kSPSCFixedUnbounded,
    kSPMCFixedBounded,
    kSPMCFixedUnbounded,
    kMPSCFixedBounded,
    kMPSCFixedUnbounded,
    kMPMCFixedBounded,
    kMPMCFixedUnbounded,
};

struct ChannelHandle
{
    IChannel *pChannel;
    ChannelType eType;
};

}
}

/**
 * @brief Create a channel
 * @param eType The type of the channel
 * @param uElemSize The size of the element
 * @param uElemCount The count of the elements
 * @param uMaxMemSize The maximum memory size of the channel
 * @return The channel handle
 */
LLDK_EXTERN_C lldk::thread::ChannelHandle lldkCreateChannel(lldk::thread::ChannelType eType, uint32_t uElemSize, uint32_t uElemCount, uint64_t uMaxMemSize);

/**
 * @brief Destroy a channel
 * @param hChannel The channel handle
 */
LLDK_EXTERN_C void lldkDestroyChannel(lldk::thread::ChannelHandle hChannel);

#endif // LLDK_THREAD_CHANNEL_H
