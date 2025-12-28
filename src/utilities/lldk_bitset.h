#ifndef LLDK_UTILITIES_LLDK_BITSET_H
#define LLDK_UTILITIES_LLDK_BITSET_H

#include "lldk/common/common.h"
#include <cstdint>

namespace lldk
{
namespace utilities
{

template <uint32_t kSize>
class LldkBitset
{
    using ArrayType = uint64_t;
    static constexpr uint32_t kBitCount = (kSize / sizeof(ArrayType)) + 1;
    static constexpr uint32_t kBitMask = sizeof(ArrayType) * 8 - 1;

    static constexpr uint32_t getArrayIndex(uint32_t uIndex)
    {
        return uIndex / sizeof(ArrayType);
    }

    static constexpr uint32_t getBitIndex(uint32_t uIndex)
    {
        return uIndex & kBitMask;
    }

public:
    LldkBitset() = default;
    ~LldkBitset() = default;

    bool test(uint32_t uIndex)
    {
        return m_arrBits[getArrayIndex(uIndex)] & (1 << getBitIndex(uIndex));
    }

    void set(uint32_t uIndex)
    {
        m_arrBits[getArrayIndex(uIndex)] |= (1 << getBitIndex(uIndex));
    }

    void clear(uint32_t uIndex)
    {
        m_arrBits[getArrayIndex(uIndex)] &= ~(1 << getBitIndex(uIndex));
    }

    bool testAll()
    {
        for (uint32_t i = 0; i < kBitCount; i++)
        {
            if (m_arrBits[i] != (ArrayType)-1)
            {
                return false;
            }
        }
        return true;
    }

    bool testAny()
    {
        for (uint32_t i = 0; i < kBitCount; i++)
        {
            if (m_arrBits[i] != (ArrayType)0)
            {
                return true;
            }
        }
        return false;
    }

    void clearAll()
    {
        memset(m_arrBits, 0, sizeof(m_arrBits));
    }

    bool testNone()
    {
        return !testAny();
    }

    void setAll()
    {
        memset(m_arrBits, (ArrayType)-1, sizeof(m_arrBits));
    }

    uint32_t count()
    {
        uint32_t uCount = 0;
        for (uint32_t i = 0; i < kBitCount; i++)
        {
            uCount += __builtin_popcountll(m_arrBits[i]);
        }
        return uCount;
    }

    uint32_t size()
    {
        return kSize;
    }

    uint32_t findFirstSet()
    {
        for (uint32_t i = 0; i < kBitCount; i++)
        {
            if (m_arrBits[i] != (ArrayType)0)
            {
                return i * sizeof(ArrayType) + __builtin_ctzll(m_arrBits[i]);
            }
        }
        return kSize;
    }
    
    uint32_t findFirstNone()
    {
        for (uint32_t i = 0; i < kBitCount; i++)
        {
            if (m_arrBits[i] != (ArrayType)-1)
            {
                return i * sizeof(ArrayType) + __builtin_ctzll(~m_arrBits[i]);
            }
        }
        return kSize;
    }

private:
    ArrayType m_arrBits[kBitCount];
};

}
}
#endif // LLDK_UTILITIES_LLDK_BITSET_H