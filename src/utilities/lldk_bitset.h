#ifndef LLDK_UTILITIES_LLDK_BITSET_H
#define LLDK_UTILITIES_LLDK_BITSET_H

#include "lldk/common/common.h"

namespace lldk
{
namespace utilities
{

template <uint32_t kSize>
class LldkBitset
{
    using ArrayType = uint64_t;
    static constexpr uint32_t kBitsPerElement = sizeof(ArrayType) * 8;  // 64 bits
    static constexpr uint32_t kBitCount = (kSize == 0) ? 1 : ((kSize + kBitsPerElement - 1) / kBitsPerElement);
    static constexpr uint32_t kBitMask = kBitsPerElement - 1;

    static_assert(kSize > 0 && kSize % kBitsPerElement == 0, "kSize must be greater than 0 and a multiple of kBitsPerElement");

    static constexpr uint32_t getArrayIndex(uint32_t uIndex)
    {
        return uIndex / kBitsPerElement;
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
        return (m_arrBits[getArrayIndex(uIndex)] & ((ArrayType)1 << getBitIndex(uIndex))) != 0;
    }

    void set(uint32_t uIndex)
    {
        m_arrBits[getArrayIndex(uIndex)] |= ((ArrayType)1 << getBitIndex(uIndex));
    }

    void clear(uint32_t uIndex)
    {
        m_arrBits[getArrayIndex(uIndex)] &= ~((ArrayType)1 << getBitIndex(uIndex));
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
        memset(m_arrBits, 0xFF, sizeof(m_arrBits));
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
                return i * kBitsPerElement + __builtin_ctzll(m_arrBits[i]);
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
                return i * kBitsPerElement + __builtin_ctzll(~m_arrBits[i]);
            }
        }
        return kSize;
    }

private:
    ArrayType m_arrBits[kBitCount] {0};
};

}
}
#endif // LLDK_UTILITIES_LLDK_BITSET_H