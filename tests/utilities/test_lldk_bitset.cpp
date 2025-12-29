#include "gtest/gtest.h"
#include "lldk_bitset.h"

using namespace lldk::utilities;

// 测试基本的 set、clear、test 操作
TEST(LldkBitset, BasicSetClearTest)
{
    LldkBitset<128> bitset;
    
    // 初始状态所有位都应该是0
    EXPECT_FALSE(bitset.test(0));
    EXPECT_FALSE(bitset.test(63));
    EXPECT_FALSE(bitset.test(64));
    EXPECT_FALSE(bitset.test(127));
    
    // 设置一些位
    bitset.set(0);
    bitset.set(63);
    bitset.set(64);
    bitset.set(127);
    
    // 验证设置的位
    EXPECT_TRUE(bitset.test(0));
    EXPECT_TRUE(bitset.test(63));
    EXPECT_TRUE(bitset.test(64));
    EXPECT_TRUE(bitset.test(127));
    
    // 验证未设置的位
    EXPECT_FALSE(bitset.test(1));
    EXPECT_FALSE(bitset.test(62));
    EXPECT_FALSE(bitset.test(65));
    
    // 清除一些位
    bitset.clear(0);
    bitset.clear(64);
    
    // 验证清除的位
    EXPECT_FALSE(bitset.test(0));
    EXPECT_TRUE(bitset.test(63));
    EXPECT_FALSE(bitset.test(64));
    EXPECT_TRUE(bitset.test(127));
}

// 测试 testAll
TEST(LldkBitset, TestAll)
{
    LldkBitset<64> bitset;
    
    // 初始状态不应该全为1
    EXPECT_FALSE(bitset.testAll());
    
    // 设置所有位
    bitset.setAll();
    EXPECT_TRUE(bitset.testAll());
    
    // 清除一个位
    bitset.clear(0);
    EXPECT_FALSE(bitset.testAll());
}

// 测试 testAny
TEST(LldkBitset, TestAny)
{
    LldkBitset<64> bitset;
    
    // 初始状态不应该有任何位被设置
    EXPECT_FALSE(bitset.testAny());
    
    // 设置一个位
    bitset.set(0);
    EXPECT_TRUE(bitset.testAny());
    
    // 清除所有位
    bitset.clearAll();
    EXPECT_FALSE(bitset.testAny());
}

// 测试 testNone
TEST(LldkBitset, TestNone)
{
    LldkBitset<64> bitset;
    
    // 初始状态应该没有任何位被设置
    EXPECT_TRUE(bitset.testNone());
    
    // 设置一个位
    bitset.set(0);
    EXPECT_FALSE(bitset.testNone());
    
    // 清除所有位
    bitset.clearAll();
    EXPECT_TRUE(bitset.testNone());
}

// 测试 clearAll
TEST(LldkBitset, ClearAll)
{
    LldkBitset<128> bitset;
    
    // 设置一些位
    bitset.set(0);
    bitset.set(63);
    bitset.set(64);
    bitset.set(127);
    
    // 清除所有位
    bitset.clearAll();
    
    // 验证所有位都被清除
    EXPECT_FALSE(bitset.test(0));
    EXPECT_FALSE(bitset.test(63));
    EXPECT_FALSE(bitset.test(64));
    EXPECT_FALSE(bitset.test(127));
    EXPECT_TRUE(bitset.testNone());
}

// 测试 setAll
TEST(LldkBitset, SetAll)
{
    LldkBitset<64> bitset;
    
    // 设置所有位
    bitset.setAll();
    
    // 验证所有位都被设置
    for (uint32_t i = 0; i < 64; i++)
    {
        EXPECT_TRUE(bitset.test(i));
    }
    EXPECT_TRUE(bitset.testAll());
}

// 测试 count
TEST(LldkBitset, Count)
{
    LldkBitset<128> bitset;
    
    // 初始状态应该为0
    EXPECT_EQ(bitset.count(), 0);
    
    // 设置一些位
    bitset.set(0);
    bitset.set(1);
    bitset.set(63);
    bitset.set(64);
    bitset.set(127);
    
    EXPECT_EQ(bitset.count(), 5);
    
    // 设置所有位
    bitset.setAll();
    EXPECT_EQ(bitset.count(), 128);
    
    // 清除所有位
    bitset.clearAll();
    EXPECT_EQ(bitset.count(), 0);
}

// 测试 size
TEST(LldkBitset, Size)
{
    LldkBitset<64> bitset1;
    EXPECT_EQ(bitset1.size(), 64);
    
    LldkBitset<128> bitset2;
    EXPECT_EQ(bitset2.size(), 128);
    
    LldkBitset<200> bitset3;
    EXPECT_EQ(bitset3.size(), 200);
}

// 测试 findFirstSet
TEST(LldkBitset, FindFirstSet)
{
    LldkBitset<128> bitset;
    
    // 初始状态应该返回 size
    EXPECT_EQ(bitset.findFirstSet(), 128);
    
    // 设置第64位
    bitset.set(64);
    EXPECT_EQ(bitset.findFirstSet(), 64);
    
    // 设置第0位
    bitset.set(0);
    EXPECT_EQ(bitset.findFirstSet(), 0);
    
    // 清除第0位
    bitset.clear(0);
    EXPECT_EQ(bitset.findFirstSet(), 64);
    
    // 设置多个位，测试找到第一个
    bitset.clearAll();
    bitset.set(10);
    bitset.set(20);
    bitset.set(30);
    EXPECT_EQ(bitset.findFirstSet(), 10);
}

// 测试 findFirstNone
TEST(LldkBitset, FindFirstNone)
{
    LldkBitset<64> bitset;
    
    // 初始状态应该返回0
    EXPECT_EQ(bitset.findFirstNone(), 0);
    
    // 设置所有位
    bitset.setAll();
    EXPECT_EQ(bitset.findFirstNone(), 64);
    
    // 清除第0位
    bitset.clear(0);
    EXPECT_EQ(bitset.findFirstNone(), 0);
    
    // 清除第10位
    bitset.setAll();
    bitset.clear(10);
    EXPECT_EQ(bitset.findFirstNone(), 10);
}

// 测试边界情况：跨数组元素
TEST(LldkBitset, CrossArrayElement)
{
    LldkBitset<128> bitset;
    
    // 测试边界位：63和64
    bitset.set(63);
    bitset.set(64);
    EXPECT_TRUE(bitset.test(63));
    EXPECT_TRUE(bitset.test(64));
    
    bitset.clear(63);
    EXPECT_FALSE(bitset.test(63));
    EXPECT_TRUE(bitset.test(64));
}

// 测试边界情况：最后一个位
TEST(LldkBitset, LastBit)
{
    LldkBitset<128> bitset;
    
    // 测试最后一个位（第127位）
    bitset.set(127);
    EXPECT_TRUE(bitset.test(127));
    EXPECT_EQ(bitset.count(), 1);
    
    bitset.clear(127);
    EXPECT_FALSE(bitset.test(127));
    EXPECT_EQ(bitset.count(), 0);
}

// 测试多个操作组合
TEST(LldkBitset, CombinedOperations)
{
    LldkBitset<200> bitset;
    
    // 设置一些位
    for (uint32_t i = 0; i < 200; i += 10)
    {
        bitset.set(i);
    }
    
    EXPECT_EQ(bitset.count(), 20);
    EXPECT_TRUE(bitset.testAny());
    EXPECT_FALSE(bitset.testAll());
    EXPECT_FALSE(bitset.testNone());
    
    // 清除所有位
    bitset.clearAll();
    EXPECT_EQ(bitset.count(), 0);
    EXPECT_FALSE(bitset.testAny());
    EXPECT_TRUE(bitset.testNone());
    
    // 设置所有位
    bitset.setAll();
    EXPECT_EQ(bitset.count(), 200);
    EXPECT_TRUE(bitset.testAll());
    
    // 清除一些位
    bitset.clear(0);
    bitset.clear(100);
    bitset.clear(199);
    EXPECT_EQ(bitset.count(), 197);
    EXPECT_FALSE(bitset.testAll());
    EXPECT_EQ(bitset.findFirstNone(), 0);
}

// 测试不同大小的 bitset
TEST(LldkBitset, DifferentSizes)
{
    LldkBitset<8> bitset1;
    bitset1.set(0);
    EXPECT_TRUE(bitset1.test(0));
    EXPECT_EQ(bitset1.size(), 1);
    
    LldkBitset<128> bitset2;
    bitset2.set(0);
    bitset2.set(127);
    EXPECT_TRUE(bitset2.test(0));
    EXPECT_TRUE(bitset2.test(127));
    EXPECT_EQ(bitset2.count(), 2);
}
