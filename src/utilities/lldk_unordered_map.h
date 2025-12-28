#ifndef LLDK_UTILITIES_LLDK_UNORDERED_MAP_H
#define LLDK_UTILITIES_LLDK_UNORDERED_MAP_H

#include "lldk/common/common.h"
#include <cstdint>
#include <unordered_map>
#include <functional>

namespace lldk
{
namespace utilities
{

template <typename Key, typename Value, typename HashFunc, uint32_t CACHE_SIZE = 64>
class LldkUnorderedMap
{
public:
    LldkUnorderedMap()
    {
        memset(m_arrEntries, 0, sizeof(m_arrEntries));
    }

    ~LldkUnorderedMap() = default;

    bool insert(const Key& key, const Value& value)
    {
        try
        {
            auto result =m_mapEntries.insert({key, value});
            if (likely(result.second))
            {
                auto uHash = hashFunc(key) % CACHE_SIZE;
                m_arrEntries[uHash] = &(*result.first);
                return true;
            }
        }
        catch (...)
        {
        }
        return false;
    }

    void erase(const Key& key)
    {
        m_mapEntries.erase(key);
        auto uHash = hashFunc(key) % CACHE_SIZE;
        if (m_arrEntries[uHash] != nullptr && (m_arrEntries[uHash])->first == key)
        {
            m_arrEntries[uHash] = nullptr;
        }
    }

    Value* find(const Key& key)
    {
        auto uHash = hashFunc(key) % CACHE_SIZE;
        if (likely(m_arrEntries[uHash] != nullptr && (m_arrEntries[uHash])->first == key))
        {
            return &m_arrEntries[uHash]->second;
        }

        m_uCachemissCount++;
        auto iter = m_mapEntries.find(key);
        if (likely(iter != m_mapEntries.end()))
        {
            m_arrEntries[uHash] = &(*iter);
            return &(m_arrEntries[uHash])->second;
        }

        return nullptr;
    }

    bool contains(const Key& key)
    {
        return find(key) != nullptr;
    }

    void clear()
    {
        memset(m_arrEntries, 0, sizeof(m_arrEntries));
        m_mapEntries.clear();
    }

    uint32_t size() const
    {
        return (uint32_t)m_mapEntries.size();
    }

    bool empty() const
    {
        return m_mapEntries.empty();
    }

    Value& operator[](const Key& key)
    {
        auto pValue = find(key);
        if (likely(pValue != nullptr))
        {
            return *pValue;
        }

        auto result = m_mapEntries.insert({key, Value()});
        if (likely(result.second))
        {
            auto uHash = hashFunc(key) % CACHE_SIZE;
            m_arrEntries[uHash] = &(*result.first);
            return result.first->second;
        }

        throw std::runtime_error("Failed to insert key-value pair");
        return Value();
    }

private:
    const std::pair<Key, Value> *m_arrEntries[CACHE_SIZE];
    uint64_t m_uCachemissCount{0};
    std::unordered_map<Key, Value, HashFunc> m_mapEntries;
};

}
}
#endif // LLDK_UTILITIES_LLDK_UNORDERED_MAP_H
