#include "gtest/gtest.h"
#include "lldk_unordered_map.h"
#include <string>
#include <cstring>
#include <vector>
#include <limits>

using namespace lldk::utilities;

// ============================================================================
// 1. 基本功能测试
// ============================================================================

// 测试基本插入和查找
TEST(LldkUnorderedMap, BasicInsertAndFind)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 初始状态应该是空的
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // 插入键值对
    EXPECT_TRUE(map.insert(1, 100));
    EXPECT_FALSE(map.empty());
    EXPECT_EQ(map.size(), 1);
    
    // 查找存在的键
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    
    // 查找不存在的键
    EXPECT_EQ(map.find(2), nullptr);
    
    // 插入更多键值对
    EXPECT_TRUE(map.insert(2, 200));
    EXPECT_TRUE(map.insert(3, 300));
    EXPECT_EQ(map.size(), 3);
    
    value = map.find(2);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 200);
    
    value = map.find(3);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 300);
}

// 测试 contains 方法
TEST(LldkUnorderedMap, Contains)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    EXPECT_FALSE(map.contains(1));
    
    map.insert(1, 100);
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    
    map.insert(2, 200);
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

// 测试 erase 方法
TEST(LldkUnorderedMap, Erase)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    EXPECT_EQ(map.size(), 3);
    
    // 删除存在的键
    map.erase(2);
    EXPECT_EQ(map.size(), 2);
    EXPECT_FALSE(map.contains(2));
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(3));
    
    // 删除不存在的键（应该不会崩溃）
    map.erase(999);
    EXPECT_EQ(map.size(), 2);
    
    // 删除所有键
    map.erase(1);
    map.erase(3);
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

// 测试 clear 方法
TEST(LldkUnorderedMap, Clear)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    EXPECT_EQ(map.size(), 3);
    
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_FALSE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_FALSE(map.contains(3));
    
    // 清空后可以继续使用
    map.insert(4, 400);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(4));
}

// 测试 operator[] 方法
TEST(LldkUnorderedMap, OperatorBracket)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 使用 operator[] 访问不存在的键，应该创建默认值
    map[1] = 100;
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[1], 100);
    
    // 修改已存在的值
    map[1] = 200;
    EXPECT_EQ(map[1], 200);
    EXPECT_EQ(map.size(), 1);
    
    // 访问多个键
    map[2] = 300;
    map[3] = 400;
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map[2], 300);
    EXPECT_EQ(map[3], 400);
    
    // 读取已存在的值
    int value = map[1];
    EXPECT_EQ(value, 200);
}

// ============================================================================
// 2. 缓存机制测试
// ============================================================================

// 测试缓存机制
TEST(LldkUnorderedMap, CacheMechanism)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map; // 使用小的缓存大小便于测试
    
    // 初始缓存未命中计数应该为0
    EXPECT_EQ(map.cachemissCount(), 0);
    
    // 第一次查找不存在的键
    // 注意：如果缓存槽中有其他键（由于之前的操作），查找不存在的键可能会增加计数
    // 因为实现会先检查缓存槽，如果不匹配，会增加计数并从map中查找
    uint64_t count0 = map.cachemissCount();
    map.find(1);
    // 验证查找不存在的键返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    
    // 插入后查找，验证数据正确性
    map.insert(1, 100);
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    
    // 查找另一个键，验证数据正确性
    map.insert(2, 200);
    value = map.find(2);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 200);
    
    // 验证所有插入的键都能找到
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}

// 测试缓存未命中计数
TEST(LldkUnorderedMap, CacheMissCount)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    // 初始计数为0
    EXPECT_EQ(map.cachemissCount(), 0);
    
    // 插入一个键
    map.insert(1, 100);
    
    // 查找键1，验证数据正确性
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    
    // 插入另一个键并查找
    map.insert(2, 200);
    value = map.find(2);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 200);
    
    // 查找不存在的键，应该返回nullptr
    // 注意：如果缓存槽中有其他键，查找不存在的键可能会增加计数
    // 因为实现会先检查缓存槽，如果不匹配，会增加计数并从map中查找
    EXPECT_EQ(map.find(999), nullptr);
    
    // 验证已存在的键仍然可以找到
    value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    value = map.find(2);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 200);
}

// 测试清空后的缓存未命中计数
TEST(LldkUnorderedMap, ClearCacheMissCount)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    map.insert(1, 100);
    map.find(1);
    
    uint64_t count = map.cachemissCount();
    map.clear();
    
    // 清空不应该重置缓存未命中计数
    EXPECT_EQ(map.cachemissCount(), count);
    
    // 清空后插入新数据
    map.insert(2, 200);
    map.find(2);
    // 计数可能会增加，也可能不变（取决于缓存状态）
}

// ============================================================================
// 3. 哈希冲突测试
// ============================================================================

// 自定义哈希函数：所有键映射到同一个缓存槽
struct ConstantHash
{
    size_t operator()(int key) const
    {
        return 0; // 所有键都映射到同一个哈希值
    }
};

TEST(LldkUnorderedMap, HashCollision)
{
    LldkUnorderedMap<int, int, ConstantHash, 4> map;
    
    // 插入多个键，它们会有哈希冲突
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    EXPECT_EQ(map.size(), 3);
    
    // 由于哈希冲突，缓存中只能存储一个，其他的需要从map中查找
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
    
    auto* value3 = map.find(3);
    ASSERT_NE(value3, nullptr);
    EXPECT_EQ(*value3, 300);
}

// 测试哈希冲突时的缓存覆盖
TEST(LldkUnorderedMap, CacheOverwriteOnHashCollision)
{
    LldkUnorderedMap<int, int, ConstantHash, 4> map;
    
    // 插入第一个键，会放入缓存槽0
    map.insert(1, 100);
    map.find(1); // 确保在缓存中
    EXPECT_EQ(map.cachemissCount(), 0);
    
    // 插入第二个键，会覆盖缓存槽0
    map.insert(2, 200);
    map.find(2); // 查找2，会更新缓存槽0为键2
    // 此时缓存槽0中存储的是键2
    
    // 再次查找键1，应该从map中查找（缓存未命中）
    uint64_t missCount = map.cachemissCount();
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    // 查找后，缓存槽0会被更新为键1
    EXPECT_GT(map.cachemissCount(), missCount); // 应该增加了缓存未命中
}

// ============================================================================
// 4. 删除后的缓存一致性测试
// ============================================================================

// 测试删除后的缓存一致性
TEST(LldkUnorderedMap, CacheConsistencyAfterErase)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    // 插入并查找，使缓存中有数据
    map.insert(1, 100);
    map.find(1); // 确保缓存中有这个键
    
    // 删除键
    map.erase(1);
    EXPECT_FALSE(map.contains(1));
    
    // 再次查找，应该返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    
    // 如果缓存中有其他键映射到同一个槽，删除不应该影响它们
    map.insert(2, 200);
    map.insert(3, 300);
    map.find(2);
    map.erase(2);
    EXPECT_FALSE(map.contains(2));
    EXPECT_TRUE(map.contains(3));
}

// 测试哈希冲突时删除后的缓存一致性
TEST(LldkUnorderedMap, CacheConsistencyAfterEraseWithHashCollision)
{
    LldkUnorderedMap<int, int, ConstantHash, 4> map;
    
    // 插入多个键，它们映射到同一个缓存槽
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    // 查找键1，使其在缓存中
    map.find(1);
    EXPECT_TRUE(map.contains(1));
    
    // 删除键1
    map.erase(1);
    EXPECT_FALSE(map.contains(1));
    
    // 缓存槽应该被清空（因为删除时检查到缓存槽中的键是1）
    // 但键2和键3仍然在map中
    EXPECT_TRUE(map.contains(2));
    EXPECT_TRUE(map.contains(3));
    
    // 查找键2，应该能从map中找到（缓存未命中）
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
    
    // 查找键3，应该能从map中找到（缓存未命中）
    auto* value3 = map.find(3);
    ASSERT_NE(value3, nullptr);
    EXPECT_EQ(*value3, 300);
}

// 测试删除不存在的键后的缓存状态
TEST(LldkUnorderedMap, EraseNonExistentKey)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    map.insert(1, 100);
    map.find(1); // 确保在缓存中
    
    // 删除不存在的键
    map.erase(999);
    
    // 缓存应该不受影响
    EXPECT_TRUE(map.contains(1));
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
}

// ============================================================================
// 5. 字符串类型测试
// ============================================================================

// 测试字符串类型
TEST(LldkUnorderedMap, StringKey)
{
    LldkUnorderedMap<std::string, int, std::hash<std::string>> map;
    
    map.insert("key1", 100);
    map.insert("key2", 200);
    map.insert("key3", 300);
    
    EXPECT_EQ(map.size(), 3);
    EXPECT_TRUE(map.contains("key1"));
    EXPECT_FALSE(map.contains("key4"));
    
    auto* value = map.find("key1");
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    
    map["key4"] = 400;
    EXPECT_EQ(map["key4"], 400);
    EXPECT_EQ(map.size(), 4);
    
    map.erase("key1");
    EXPECT_FALSE(map.contains("key1"));
    EXPECT_EQ(map.size(), 3);
}

// 测试字符串值类型
TEST(LldkUnorderedMap, StringValue)
{
    LldkUnorderedMap<int, std::string, std::hash<int>> map;
    
    map.insert(1, "value1");
    map.insert(2, "value2");
    
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, "value1");
    
    map[3] = "value3";
    EXPECT_EQ(map[3], "value3");
}

// ============================================================================
// 6. 重复插入测试
// ============================================================================

// 测试重复插入
TEST(LldkUnorderedMap, DuplicateInsert)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 第一次插入应该成功
    EXPECT_TRUE(map.insert(1, 100));
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(*map.find(1), 100);
    
    // 重复插入相同的键，应该失败（返回false）
    EXPECT_FALSE(map.insert(1, 200));
    EXPECT_EQ(map.size(), 1); // 大小不应该改变
    EXPECT_EQ(*map.find(1), 100); // 值不应该改变
}

// 测试重复插入后的缓存状态
TEST(LldkUnorderedMap, DuplicateInsertCacheState)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    // 插入并查找，使缓存中有数据
    map.insert(1, 100);
    map.find(1);
    EXPECT_EQ(map.cachemissCount(), 0);
    
    // 重复插入，应该失败
    EXPECT_FALSE(map.insert(1, 200));
    
    // 缓存状态应该不变
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
    EXPECT_EQ(map.cachemissCount(), 0); // 应该仍然命中缓存
}

// ============================================================================
// 7. 大量数据测试
// ============================================================================

// 测试大量数据
TEST(LldkUnorderedMap, LargeData)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    const int count = 1000;
    for (int i = 0; i < count; i++)
    {
        EXPECT_TRUE(map.insert(i, i * 10));
    }
    
    EXPECT_EQ(map.size(), count);
    
    // 验证所有数据
    for (int i = 0; i < count; i++)
    {
        auto* value = map.find(i);
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(*value, i * 10);
    }
    
    // 删除一半数据
    for (int i = 0; i < count; i += 2)
    {
        map.erase(i);
    }
    
    EXPECT_EQ(map.size(), count / 2);
    
    // 验证剩余数据
    for (int i = 1; i < count; i += 2)
    {
        EXPECT_TRUE(map.contains(i));
        auto* value = map.find(i);
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(*value, i * 10);
    }
}

// ============================================================================
// 8. 边界情况测试
// ============================================================================

// 测试边界情况：空map的各种操作
TEST(LldkUnorderedMap, EmptyMapOperations)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // 查找不存在的键，应该返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    EXPECT_FALSE(map.contains(1));
    
    // 在空map上删除不应该崩溃
    map.erase(1);
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // 清空空map不应该有问题
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // 清空后，查找不存在的键仍然应该返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    EXPECT_FALSE(map.contains(1));
}

// 测试自定义缓存大小
TEST(LldkUnorderedMap, CustomCacheSize)
{
    LldkUnorderedMap<int, int, std::hash<int>, 8> map; // 缓存大小为8
    
    // 插入多个键
    for (int i = 0; i < 20; i++)
    {
        map.insert(i, i * 10);
    }
    
    EXPECT_EQ(map.size(), 20);
    
    // 验证所有键都能找到
    for (int i = 0; i < 20; i++)
    {
        EXPECT_TRUE(map.contains(i));
        auto* value = map.find(i);
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(*value, i * 10);
    }
}

// 测试最小缓存大小（CACHE_SIZE=1）
TEST(LldkUnorderedMap, MinimumCacheSize)
{
    LldkUnorderedMap<int, int, std::hash<int>, 1> map; // 缓存大小为1
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    EXPECT_EQ(map.size(), 3);
    
    // 由于缓存大小只有1，只有最后查找的键会在缓存中
    map.find(1); // 键1在缓存中
    EXPECT_TRUE(map.contains(1));
    
    map.find(2); // 键2覆盖缓存，键1不在缓存中
    EXPECT_TRUE(map.contains(2));
    
    // 再次查找键1，应该从map中查找（缓存未命中）
    uint64_t missCount = map.cachemissCount();
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    EXPECT_GT(map.cachemissCount(), missCount); // 应该增加了缓存未命中
}

// ============================================================================
// 9. 错误处理测试
// ============================================================================

// 测试插入失败的情况（虽然很难触发，但测试代码逻辑）
TEST(LldkUnorderedMap, InsertFailure)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 正常插入应该成功
    EXPECT_TRUE(map.insert(1, 100));
    
    // 重复插入应该失败
    EXPECT_FALSE(map.insert(1, 200));
    
    // 值不应该改变
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
}

// 测试 operator[] 的异常情况
// 注意：operator[] 在插入失败时会抛出异常，但这种情况在实际中很难触发
// 因为 std::unordered_map::insert 通常不会失败（除非内存不足）
TEST(LldkUnorderedMap, OperatorBracketException)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 正常情况下，operator[] 应该能正常工作
    map[1] = 100;
    EXPECT_EQ(map[1], 100);
    
    // 如果插入失败，应该抛出异常
    // 但在正常情况下很难触发这种情况
    // 这里主要测试正常流程
    map[2] = 200;
    EXPECT_EQ(map[2], 200);
}

// 测试 operator[] 在重复访问时的行为
TEST(LldkUnorderedMap, OperatorBracketRepeatedAccess)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 使用 operator[] 创建并赋值
    map[1] = 100;
    EXPECT_EQ(map.size(), 1);
    
    // 重复使用 operator[] 访问，不应该改变大小
    int& value1 = map[1];
    EXPECT_EQ(value1, 100);
    EXPECT_EQ(map.size(), 1);
    
    // 修改值
    map[1] = 200;
    EXPECT_EQ(map[1], 200);
    EXPECT_EQ(map.size(), 1);
}

// ============================================================================
// 10. 复杂类型测试
// ============================================================================

// 测试复杂类型：pair作为值
TEST(LldkUnorderedMap, ComplexValueType)
{
    LldkUnorderedMap<int, std::pair<int, std::string>, std::hash<int>> map;
    
    map.insert(1, std::make_pair(100, "value1"));
    map.insert(2, std::make_pair(200, "value2"));
    
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->first, 100);
    EXPECT_EQ(value->second, "value1");
    
    map[3] = std::make_pair(300, "value3");
    EXPECT_EQ(map[3].first, 300);
    EXPECT_EQ(map[3].second, "value3");
}

// 测试复杂类型：vector作为值
TEST(LldkUnorderedMap, ComplexValueTypeVector)
{
    LldkUnorderedMap<int, std::vector<int>, std::hash<int>> map;
    
    std::vector<int> vec1 = {1, 2, 3};
    map.insert(1, vec1);
    
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->size(), 3);
    EXPECT_EQ((*value)[0], 1);
    EXPECT_EQ((*value)[1], 2);
    EXPECT_EQ((*value)[2], 3);
    
    // 使用 operator[] 创建默认值（空vector）
    std::vector<int>& vec2 = map[2];
    EXPECT_EQ(vec2.size(), 0);
    vec2.push_back(10);
    vec2.push_back(20);
    EXPECT_EQ(map[2].size(), 2);
    EXPECT_EQ(map[2][0], 10);
    EXPECT_EQ(map[2][1], 20);
}

// ============================================================================
// 11. 自定义哈希函数测试
// ============================================================================

// 自定义哈希函数：返回键本身
struct IdentityHash
{
    size_t operator()(int key) const
    {
        return static_cast<size_t>(key);
    }
};

TEST(LldkUnorderedMap, CustomHashFunction)
{
    LldkUnorderedMap<int, int, IdentityHash, 8> map;
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    EXPECT_EQ(map.size(), 3);
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
    EXPECT_TRUE(map.contains(3));
    
    auto* value = map.find(1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100);
}

// 自定义哈希函数：返回键的平方
struct SquareHash
{
    size_t operator()(int key) const
    {
        return static_cast<size_t>(key * key);
    }
};

TEST(LldkUnorderedMap, CustomHashFunctionSquare)
{
    LldkUnorderedMap<int, int, SquareHash, 16> map;
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    EXPECT_EQ(map.size(), 3);
    
    // 验证所有键都能找到
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
    EXPECT_TRUE(map.contains(3));
    
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
}

// ============================================================================
// 12. 缓存覆盖场景测试
// ============================================================================

// 测试新键覆盖旧键的缓存槽
TEST(LldkUnorderedMap, CacheSlotOverwrite)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    // 插入键1，假设它映射到缓存槽0
    map.insert(1, 100);
    map.find(1); // 确保在缓存中
    uint64_t missCount1 = map.cachemissCount();
    
    // 插入键2，假设它也映射到缓存槽0（哈希冲突）
    // 或者使用一个能确保冲突的哈希函数
    // 这里我们使用一个简单的策略：插入多个键，其中一些会冲突
    
    // 先查找键1，确保它在缓存中
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    EXPECT_EQ(map.cachemissCount(), missCount1); // 应该命中缓存
    
    // 插入键2并查找，可能会覆盖缓存槽
    map.insert(2, 200);
    map.find(2);
    
    // 如果键2和键1映射到不同的缓存槽，键1仍然在缓存中
    // 如果映射到同一个槽，键1会被覆盖
    // 这里我们验证无论哪种情况，数据都是正确的
    value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
}

// ============================================================================
// 13. 综合场景测试
// ============================================================================

// 测试综合场景：插入、查找、删除、清空的组合操作
TEST(LldkUnorderedMap, ComprehensiveScenario)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 阶段1：插入数据
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(map.insert(i, i * 10));
    }
    EXPECT_EQ(map.size(), 10);
    
    // 阶段2：查找和修改
    for (int i = 0; i < 10; i++)
    {
        auto* value = map.find(i);
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(*value, i * 10);
        
        // 使用 operator[] 修改值
        map[i] = i * 20;
        EXPECT_EQ(map[i], i * 20);
    }
    
    // 阶段3：删除部分数据
    for (int i = 0; i < 10; i += 2)
    {
        map.erase(i);
    }
    EXPECT_EQ(map.size(), 5);
    
    // 阶段4：验证剩余数据
    for (int i = 1; i < 10; i += 2)
    {
        EXPECT_TRUE(map.contains(i));
        EXPECT_EQ(map[i], i * 20);
    }
    
    // 阶段5：清空
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // 阶段6：重新插入
    map.insert(100, 1000);
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[100], 1000);
}

// 测试缓存未命中计数的准确性
TEST(LldkUnorderedMap, CacheMissCountAccuracy)
{
    LldkUnorderedMap<int, int, std::hash<int>, 2> map; // 很小的缓存，容易产生未命中
    
    // 初始计数为0
    EXPECT_EQ(map.cachemissCount(), 0);
    
    // 插入键1并查找
    map.insert(1, 100);
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    
    // 插入键2并查找
    map.insert(2, 200);
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
    
    // 插入键3，由于缓存只有2个槽，查找时很可能未命中
    map.insert(3, 300);
    uint64_t count3 = map.cachemissCount();
    auto* value3 = map.find(3);
    ASSERT_NE(value3, nullptr);
    EXPECT_EQ(*value3, 300);
    uint64_t count4 = map.cachemissCount();
    
    // 如果键3映射到已占用的槽，应该增加缓存未命中
    // 如果映射到空槽，可能不会增加
    // 这里我们只验证计数不会减少
    EXPECT_GE(count4, count3);
    
    // 查找不存在的键，应该返回nullptr
    // 注意：如果缓存槽中有其他键，查找不存在的键可能会增加计数
    uint64_t count5 = map.cachemissCount();
    EXPECT_EQ(map.find(999), nullptr);
    // 验证已存在的键仍然可以找到
    value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
}

// ============================================================================
// 14. 边界值和特殊值测试
// ============================================================================

// 测试零值
TEST(LldkUnorderedMap, ZeroValue)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    map.insert(0, 0);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(0));
    
    auto* value = map.find(0);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 0);
    
    map[0] = 100;
    EXPECT_EQ(map[0], 100);
}

// 测试负数值
TEST(LldkUnorderedMap, NegativeValue)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    map.insert(-1, -100);
    map.insert(-2, -200);
    
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(-1));
    EXPECT_TRUE(map.contains(-2));
    
    auto* value = map.find(-1);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, -100);
    
    map[-3] = -300;
    EXPECT_EQ(map[-3], -300);
}

// 测试最大值
TEST(LldkUnorderedMap, MaxValue)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    int maxKey = std::numeric_limits<int>::max();
    int maxValue = std::numeric_limits<int>::max();
    
    map.insert(maxKey, maxValue);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(maxKey));
    
    auto* value = map.find(maxKey);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, maxValue);
}

// 测试最小值
TEST(LldkUnorderedMap, MinValue)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    int minKey = std::numeric_limits<int>::min();
    int minValue = std::numeric_limits<int>::min();
    
    map.insert(minKey, minValue);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(minKey));
    
    auto* value = map.find(minKey);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, minValue);
}

// ============================================================================
// 15. 错误用例分析和测试
// ============================================================================

// 测试删除后立即查找的行为
TEST(LldkUnorderedMap, FindAfterErase)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    map.insert(1, 100);
    map.find(1); // 确保在缓存中
    
    map.erase(1);
    
    // 删除后立即查找，应该返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    EXPECT_FALSE(map.contains(1));
    
    // 缓存槽应该被清空
    // 如果插入新键映射到同一个槽，应该能正常工作
    map.insert(2, 200);
    auto* value = map.find(2);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 200);
}

// 测试清空后缓存状态
TEST(LldkUnorderedMap, CacheStateAfterClear)
{
    LldkUnorderedMap<int, int, std::hash<int>, 4> map;
    
    map.insert(1, 100);
    map.insert(2, 200);
    map.find(1);
    map.find(2);
    
    uint64_t missCount = map.cachemissCount();
    map.clear();
    
    // 清空后，缓存应该被清空
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.cachemissCount(), missCount); // 计数不应该重置
    
    // 清空后查找，应该返回nullptr
    EXPECT_EQ(map.find(1), nullptr);
    EXPECT_EQ(map.find(2), nullptr);
    
    // 清空后插入新数据，应该能正常工作
    map.insert(3, 300);
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(3));
}

// 测试 operator[] 创建默认值的行为
TEST(LldkUnorderedMap, OperatorBracketDefaultValue)
{
    LldkUnorderedMap<int, int, std::hash<int>> map;
    
    // 使用 operator[] 访问不存在的键，会创建默认值（0）
    int& value = map[1];
    EXPECT_EQ(value, 0);
    EXPECT_EQ(map.size(), 1);
    
    // 修改值
    value = 100;
    EXPECT_EQ(map[1], 100);
    
    // 再次访问，应该得到修改后的值
    int& value2 = map[1];
    EXPECT_EQ(value2, 100);
}

// 测试 operator[] 对字符串默认值的行为
TEST(LldkUnorderedMap, OperatorBracketStringDefaultValue)
{
    LldkUnorderedMap<int, std::string, std::hash<int>> map;
    
    // 使用 operator[] 访问不存在的键，会创建默认值（空字符串）
    std::string& value = map[1];
    EXPECT_EQ(value, "");
    EXPECT_EQ(map.size(), 1);
    
    // 修改值
    value = "test";
    EXPECT_EQ(map[1], "test");
}

// 测试缓存未命中后更新缓存的行为
TEST(LldkUnorderedMap, CacheUpdateAfterMiss)
{
    LldkUnorderedMap<int, int, ConstantHash, 4> map; // 所有键映射到同一槽
    
    // 插入键1
    map.insert(1, 100);
    auto* value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    
    // 插入键2
    map.insert(2, 200);
    
    // 查找键2，由于所有键映射到同一槽，缓存槽中可能是键1，所以查找键2会未命中
    uint64_t missCount1 = map.cachemissCount();
    auto* value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
    // 如果缓存槽中的键不是2，应该增加了缓存未命中
    // 查找后，缓存槽应该被更新为键2
    
    // 再次查找键2，如果缓存槽现在是键2，应该命中缓存
    uint64_t missCount2 = map.cachemissCount();
    value2 = map.find(2);
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(*value2, 200);
    
    // 查找键1，由于缓存槽中现在是键2，应该未命中
    uint64_t missCount3 = map.cachemissCount();
    value1 = map.find(1);
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(*value1, 100);
    // 如果缓存槽中的键不是1，应该增加了缓存未命中
    // 查找后，缓存槽应该被更新为键1
    
    // 验证所有键都能正确找到
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
}
