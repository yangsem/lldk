#include "gtest/gtest.h"
#include "lldk_thread_local.h"
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <cstring>
#include <algorithm>

using namespace lldk::utilities;

// ============================================================================
// 测试辅助类
// ============================================================================

// 简单的测试类
class TestObject
{
public:
    TestObject() : m_value(0), m_created(true)
    {
        s_constructorCount++;
    }
    
    explicit TestObject(int value) : m_value(value), m_created(true)
    {
        s_constructorCount++;
    }
    
    ~TestObject()
    {
        if (m_created)
        {
            s_destructorCount++;
        }
        m_created = false;
    }
    
    int getValue() const { return m_value; }
    void setValue(int value) { m_value = value; }
    
    static int getConstructorCount() { return s_constructorCount.load(); }
    static int getDestructorCount() { return s_destructorCount.load(); }
    static void resetCounters()
    {
        s_constructorCount = 0;
        s_destructorCount = 0;
    }
    
private:
    int m_value;
    bool m_created;
    static std::atomic<int> s_constructorCount;
    static std::atomic<int> s_destructorCount;
};

std::atomic<int> TestObject::s_constructorCount{0};
std::atomic<int> TestObject::s_destructorCount{0};

// 可默认构造的创建函数对象
struct CreateTestObjectFunc
{
    TestObject* operator()() const
    {
        return new TestObject();
    }
};

struct CreateTestObjectWithValueFunc
{
    int m_value;
    
    CreateTestObjectWithValueFunc(int value = 0) : m_value(value) {}
    
    TestObject* operator()() const
    {
        return new TestObject(m_value);
    }
};

// 可默认构造的销毁函数对象
struct DestroyTestObjectFunc
{
    void operator()(TestObject* obj) const
    {
        delete obj;
    }
};

// 可默认构造的字符串创建函数
struct CreateStringFunc
{
    std::string* operator()() const
    {
        return new std::string("default");
    }
};

// 可默认构造的字符串销毁函数
struct DestroyStringFunc
{
    void operator()(std::string* str) const
    {
        delete str;
    }
};

// ============================================================================
// 1. 基本功能测试
// ============================================================================

// 测试基本创建和获取
TEST(LldkThreadLocal, BasicGet)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 第一次获取，应该创建新对象
    TestObject* obj1 = threadLocal.get();
    ASSERT_NE(obj1, nullptr);
    EXPECT_EQ(TestObject::getConstructorCount(), 1);
    
    // 再次获取，应该返回同一个对象
    TestObject* obj2 = threadLocal.get();
    ASSERT_NE(obj2, nullptr);
    EXPECT_EQ(obj1, obj2); // 应该是同一个对象
    EXPECT_EQ(TestObject::getConstructorCount(), 1); // 不应该再次创建
}

// 测试不同线程获取不同的对象
TEST(LldkThreadLocal, DifferentThreadsDifferentObjects)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    std::vector<TestObject*> objects;
    std::vector<std::thread> threads;
    std::mutex mutex;
    
    // 创建多个线程，每个线程获取对象
    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back([&threadLocal, &objects, &mutex]() {
            TestObject* obj = threadLocal.get();
            ASSERT_NE(obj, nullptr);
            
            std::lock_guard<std::mutex> lock(mutex);
            objects.push_back(obj);
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads)
    {
        t.join();
    }
    
    // 验证每个线程获取的对象都不同
    EXPECT_EQ(objects.size(), 5);
    for (size_t i = 0; i < objects.size(); i++)
    {
        for (size_t j = i + 1; j < objects.size(); j++)
        {
            EXPECT_NE(objects[i], objects[j]) << "Thread " << i << " and " << j << " should have different objects";
        }
    }
    
    // 验证创建了5个对象
    EXPECT_EQ(TestObject::getConstructorCount(), 5);
}

// ============================================================================
// 2. 多实例测试
// ============================================================================

// 测试多个LldkThreadLocal实例的隔离
TEST(LldkThreadLocal, MultipleInstancesIsolation)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    // 创建两个不同的实例
    // 注意：由于CreateFunc必须是可默认构造的，我们无法直接传递不同的值
    // 这里我们使用相同的创建函数，但验证它们是不同的实例
    ThreadLocal threadLocal1;
    ThreadLocal threadLocal2;
    
    // 在同一线程中获取两个实例的对象
    TestObject* obj1 = threadLocal1.get();
    TestObject* obj2 = threadLocal2.get();
    
    ASSERT_NE(obj1, nullptr);
    ASSERT_NE(obj2, nullptr);
    EXPECT_NE(obj1, obj2); // 应该是不同的对象
    
    // 验证创建了2个对象
    EXPECT_EQ(TestObject::getConstructorCount(), 2);
}

// 测试多个实例在不同线程中的隔离
TEST(LldkThreadLocal, MultipleInstancesMultiThread)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal1;
    ThreadLocal threadLocal2;
    
    std::vector<std::pair<TestObject*, TestObject*>> results;
    std::vector<std::thread> threads;
    std::mutex mutex;
    
    // 创建多个线程，每个线程获取两个实例的对象
    for (int i = 0; i < 3; i++)
    {
        threads.emplace_back([&threadLocal1, &threadLocal2, &results, &mutex]() {
            TestObject* obj1 = threadLocal1.get();
            TestObject* obj2 = threadLocal2.get();
            
            ASSERT_NE(obj1, nullptr);
            ASSERT_NE(obj2, nullptr);
            EXPECT_NE(obj1, obj2);
            
            std::lock_guard<std::mutex> lock(mutex);
            results.push_back({obj1, obj2});
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads)
    {
        t.join();
    }
    
    // 验证每个线程都有不同的对象
    EXPECT_EQ(results.size(), 3);
    for (size_t i = 0; i < results.size(); i++)
    {
        for (size_t j = i + 1; j < results.size(); j++)
        {
            // 不同线程的threadLocal1对象应该不同
            EXPECT_NE(results[i].first, results[j].first);
            // 不同线程的threadLocal2对象应该不同
            EXPECT_NE(results[i].second, results[j].second);
        }
    }
    
    // 验证创建了6个对象（3个线程 * 2个实例）
    EXPECT_EQ(TestObject::getConstructorCount(), 6);
}

// ============================================================================
// 3. 生命周期测试
// ============================================================================

// 测试对象在LldkThreadLocal销毁时被销毁
TEST(LldkThreadLocal, LifecycleOnDestruction)
{
    TestObject::resetCounters();
    
    {
        using ThreadLocal = LldkThreadLocal<TestObject>;
        
        ThreadLocal threadLocal;
        
        // 获取对象
        TestObject* obj = threadLocal.get();
        ASSERT_NE(obj, nullptr);
        EXPECT_EQ(TestObject::getConstructorCount(), 1);
        EXPECT_EQ(TestObject::getDestructorCount(), 0);
        
        // threadLocal 销毁时，对象应该被销毁
    }
    
    // 验证对象已被销毁
    EXPECT_EQ(TestObject::getDestructorCount(), 1);
}

// 测试多个线程中的对象在LldkThreadLocal销毁时都被销毁
TEST(LldkThreadLocal, LifecycleMultiThread)
{
    TestObject::resetCounters();
    
    {
        using ThreadLocal = LldkThreadLocal<TestObject>;
        
        ThreadLocal threadLocal;
        
        std::vector<std::thread> threads;
        
        // 创建多个线程，每个线程获取对象
        for (int i = 0; i < 5; i++)
        {
            threads.emplace_back([&threadLocal]() {
                TestObject* obj = threadLocal.get();
                ASSERT_NE(obj, nullptr);
            });
        }
        
        // 等待所有线程完成
        for (auto& t : threads)
        {
            t.join();
        }
        
        EXPECT_EQ(TestObject::getConstructorCount(), 5);
        EXPECT_EQ(TestObject::getDestructorCount(), 0);
        
        // threadLocal 销毁时，所有线程的对象都应该被销毁
    }
    
    // 验证所有对象已被销毁
    EXPECT_EQ(TestObject::getDestructorCount(), 5);
}

// 测试线程局部变量的生命周期与线程一致
TEST(LldkThreadLocal, ThreadLocalLifecycle)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    TestObject* mainThreadObj = nullptr;
    
    {
        std::thread t([&threadLocal, &mainThreadObj]() {
            (void)mainThreadObj;
            // 在子线程中获取对象
            TestObject* obj = threadLocal.get();
            ASSERT_NE(obj, nullptr);
            EXPECT_EQ(TestObject::getConstructorCount(), 1);
        });
        
        t.join();
        
        // 在主线程中获取对象
        mainThreadObj = threadLocal.get();
        ASSERT_NE(mainThreadObj, nullptr);
        EXPECT_EQ(TestObject::getConstructorCount(), 2);
    }
    
    // 子线程结束后，子线程的对象应该被销毁（通过LldkThreadLocal的析构函数）
    // 但主线程的对象仍然存在
    EXPECT_EQ(TestObject::getDestructorCount(), 0); // 对象在threadLocal销毁时才会被销毁
}

// ============================================================================
// 4. foreach 测试
// ============================================================================

// 测试 foreach 方法
TEST(LldkThreadLocal, Foreach)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    std::vector<std::thread> threads;
    std::vector<TestObject*> collectedObjects;
    std::mutex mutex;
    
    // 创建多个线程，每个线程获取对象并设置不同的值
    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back([&threadLocal, i]() {
            TestObject* obj = threadLocal.get();
            ASSERT_NE(obj, nullptr);
            obj->setValue(i * 10);
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads)
    {
        t.join();
    }
    
    // 使用 foreach 收集所有对象
    int result = threadLocal.foreach(std::function<int32_t(TestObject*)>([&collectedObjects, &mutex](TestObject* obj) {
        std::lock_guard<std::mutex> lock(mutex);
        collectedObjects.push_back(obj);
        return 0;
    }));
    
    EXPECT_EQ(result, 0); // 应该成功
    EXPECT_EQ(collectedObjects.size(), 5); // 应该收集到5个对象
    
    // 验证所有对象都被收集到
    std::vector<int> values;
    for (auto* obj : collectedObjects)
    {
        values.push_back(obj->getValue());
    }
    std::sort(values.begin(), values.end());
    std::vector<int> expected = {0, 10, 20, 30, 40};
    EXPECT_EQ(values, expected);
}

// 测试 foreach 使用空函数
TEST(LldkThreadLocal, ForeachEmptyFunction)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 使用空函数，应该成功但不做任何操作
    int result = threadLocal.foreach(std::function<int32_t(TestObject*)>([](TestObject* obj) {
        (void)obj;
        // 空操作
        return 0;
    }));
    
    EXPECT_EQ(result, 0);
}

// 测试 foreach 使用 nullptr
TEST(LldkThreadLocal, ForeachNullptr)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 使用 nullptr，应该返回错误
    int result = threadLocal.foreach(std::function<int32_t(TestObject*)>(nullptr));
    
    EXPECT_EQ(result, -1);
    // 注意：如果无法链接到lldk_base库，lldkGetErrorCode可能不可用
    // 这里只验证返回值
}

// ============================================================================
// 5. 错误处理测试
// ============================================================================

// 测试创建函数返回 nullptr
struct CreateNullFunc
{
    TestObject* operator()() const
    {
        return nullptr;
    }
};

TEST(LldkThreadLocal, CreateFuncReturnsNullptr)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    TestObject* obj = threadLocal.get();
    EXPECT_EQ(obj, nullptr);
    // 注意：如果无法链接到lldk_base库，lldkGetErrorCode可能不可用
    // 这里只验证返回值为nullptr
}

// ============================================================================
// 6. 边界情况测试
// ============================================================================

// 测试实例ID耗尽的情况
TEST(LldkThreadLocal, InstanceIdExhausted)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    std::vector<std::unique_ptr<ThreadLocal>> instances;
    
    // 创建第一个实例，应该成功
    instances.emplace_back(new ThreadLocal());
    EXPECT_NE(instances.back(), nullptr);
    
    // 创建第二个实例，应该成功
    instances.emplace_back(new ThreadLocal());
    EXPECT_NE(instances.back(), nullptr);
    
    // 尝试创建第三个实例，应该抛出异常
    EXPECT_THROW(
        {
            instances.emplace_back(new ThreadLocal());
        },
        std::runtime_error
    );
}

// 测试销毁后重新创建
TEST(LldkThreadLocal, DestroyAndRecreate)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    {
        ThreadLocal threadLocal;
        
        TestObject* obj = threadLocal.get();
        ASSERT_NE(obj, nullptr);
        EXPECT_EQ(TestObject::getConstructorCount(), 1);
    }
    
    // 销毁后，对象应该被销毁
    EXPECT_EQ(TestObject::getDestructorCount(), 1);
    
    // 重新创建
    {
        ThreadLocal threadLocal;
        
        TestObject* obj = threadLocal.get();
        ASSERT_NE(obj, nullptr);
        EXPECT_EQ(TestObject::getConstructorCount(), 2);
    }
    
    EXPECT_EQ(TestObject::getDestructorCount(), 2);
}

// ============================================================================
// 7. 复杂类型测试
// ============================================================================

// 测试字符串类型
TEST(LldkThreadLocal, StringType)
{
    using StringThreadLocal = LldkThreadLocal<std::string>;
    
    StringThreadLocal threadLocal;
    
    std::string* str = threadLocal.get();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(*str, "default");
    
    *str = "modified";
    EXPECT_EQ(*str, "modified");
    
    // 再次获取，应该是同一个对象
    std::string* str2 = threadLocal.get();
    EXPECT_EQ(str, str2);
    EXPECT_EQ(*str2, "modified");
}

// 测试自定义类型
struct CustomData
{
    int id;
    std::string name;
    std::vector<int> values;
    
    CustomData() : id(0), name("default")
    {
    }
    
    CustomData(int i, const std::string& n) : id(i), name(n)
    {
    }
};

struct CreateCustomDataFunc
{
    CustomData* operator()() const
    {
        return new CustomData(100, "test");
    }
};

struct DestroyCustomDataFunc
{
    void operator()(CustomData* data) const
    {
        delete data;
    }
};

TEST(LldkThreadLocal, CustomType)
{
    using CustomThreadLocal = LldkThreadLocal<CustomData>;
    
    CustomThreadLocal threadLocal;
    
    CustomData* data = threadLocal.get();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->id, 100);
    EXPECT_EQ(data->name, "test");
    
    data->values.push_back(1);
    data->values.push_back(2);
    EXPECT_EQ(data->values.size(), 2);
}

// ============================================================================
// 8. 并发测试
// ============================================================================

// 测试并发访问
TEST(LldkThreadLocal, ConcurrentAccess)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    const int numThreads = 10;
    const int numOperations = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    // 创建多个线程，每个线程进行多次操作
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([&threadLocal, &successCount, numOperations]() {
            (void)numOperations;
            for (int j = 0; j < numOperations; j++)
            {
                TestObject* obj = threadLocal.get();
                if (obj != nullptr)
                {
                    obj->setValue(j);
                    if (obj->getValue() == j)
                    {
                        successCount++;
                    }
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads)
    {
        t.join();
    }
    
    // 验证所有操作都成功
    EXPECT_EQ(successCount.load(), numThreads * numOperations);
}

// ============================================================================
// 9. 综合场景测试
// ============================================================================

// 测试综合场景：多实例、多线程、foreach
TEST(LldkThreadLocal, ComprehensiveScenario)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal1;
    ThreadLocal threadLocal2;
    
    std::vector<std::thread> threads;
    std::atomic<int> threadCount{0};
    
    // 创建多个线程
    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back([&threadLocal1, &threadLocal2, &threadCount, i]() {
            TestObject* obj1 = threadLocal1.get();
            TestObject* obj2 = threadLocal2.get();
            
            ASSERT_NE(obj1, nullptr);
            ASSERT_NE(obj2, nullptr);
            EXPECT_NE(obj1, obj2);
            
            obj1->setValue(i * 10);
            obj2->setValue(i * 20);
            
            threadCount++;
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads)
    {
        t.join();
    }
    
    EXPECT_EQ(threadCount.load(), 5);
    
    // 使用 foreach 验证所有对象
    std::atomic<int> count1{0};
    std::atomic<int> count2{0};
    
    threadLocal1.foreach(std::function<int32_t(TestObject*)>([&count1](TestObject* obj) {
        count1++;
        EXPECT_NE(obj, nullptr);
        return 0;
    }));
    
    threadLocal2.foreach(std::function<int32_t(TestObject*)>([&count2](TestObject* obj) {
        count2++;
        EXPECT_NE(obj, nullptr);
        return 0;
    }));
    
    EXPECT_EQ(count1.load(), 5);
    EXPECT_EQ(count2.load(), 5);
}

// ============================================================================
// 10. 错误用例分析和测试
// ============================================================================

// 测试在对象已销毁后访问（通过foreach）
TEST(LldkThreadLocal, AccessAfterDestruction)
{
    TestObject::resetCounters();
    
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    std::vector<TestObject*> objects;
    
    {
        ThreadLocal threadLocal;
        
        // 在多个线程中获取对象
        std::vector<std::thread> threads;
        std::mutex mutex;
        
        for (int i = 0; i < 3; i++)
        {
            threads.emplace_back([&threadLocal, &objects, &mutex]() {
                TestObject* obj = threadLocal.get();
                std::lock_guard<std::mutex> lock(mutex);
                objects.push_back(obj);
            });
        }
        
        for (auto& t : threads)
        {
            t.join();
        }
        
        // 使用 foreach 收集对象
        threadLocal.foreach(std::function<int32_t(TestObject*)>([&objects, &mutex](TestObject* obj) {
            (void)objects;
            std::lock_guard<std::mutex> lock(mutex);
            // 对象应该有效
            EXPECT_NE(obj, nullptr);
            return 0;
        }));
        
        // threadLocal 销毁时，所有对象应该被销毁
    }
    
    // 验证所有对象已被销毁
    EXPECT_EQ(TestObject::getDestructorCount(), 3);
}

// 测试多次调用 get 返回同一个对象
TEST(LldkThreadLocal, MultipleGetSameObject)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 多次调用 get，应该返回同一个对象
    TestObject* obj1 = threadLocal.get();
    TestObject* obj2 = threadLocal.get();
    TestObject* obj3 = threadLocal.get();
    
    EXPECT_EQ(obj1, obj2);
    EXPECT_EQ(obj2, obj3);
    
    // 修改值
    obj1->setValue(100);
    EXPECT_EQ(obj2->getValue(), 100);
    EXPECT_EQ(obj3->getValue(), 100);
}

// 测试 foreach 在空情况下
TEST(LldkThreadLocal, ForeachEmpty)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 在没有获取任何对象的情况下调用 foreach
    int count = 0;
    int result = threadLocal.foreach(std::function<int32_t(TestObject*)>([&count](TestObject* obj) {
        (void)obj;
        count++;
        return 0;
    }));
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(count, 0); // 应该没有对象
}

// 测试 foreach 在部分线程有对象的情况下
TEST(LldkThreadLocal, ForeachPartial)
{
    using ThreadLocal = LldkThreadLocal<TestObject>;
    
    ThreadLocal threadLocal;
    
    // 只在部分线程中获取对象
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back([&threadLocal, i]() {
            if (i % 2 == 0) // 只在偶数索引的线程中获取对象
            {
                TestObject* obj = threadLocal.get();
                ASSERT_NE(obj, nullptr);
                obj->setValue(i);
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // 使用 foreach 收集对象
    std::vector<int> values;
    std::mutex mutex;
    threadLocal.foreach(std::function<int32_t(TestObject*)>([&values, &mutex](TestObject* obj) {
        std::lock_guard<std::mutex> lock(mutex);
        values.push_back(obj->getValue());
        return 0;
    }));
    
    // 应该收集到3个对象（索引0, 2, 4）
    EXPECT_EQ(values.size(), 3);
    std::sort(values.begin(), values.end());
    std::vector<int> expected = {0, 2, 4};
    EXPECT_EQ(values, expected);
}
