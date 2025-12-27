# CMakeLists.txt 模板使用说明

## 概述

`CMakeLists.txt.template` 是一个通用的 CMake 模板，适用于编译 lldk 项目中的每个库。该模板支持：

1. ✅ 指定 C++ 标准版本
2. ✅ 指定 Release/Debug 构建类型
3. ✅ 代码覆盖率生成
4. ✅ 各种 Sanitizer 工具支持

## 快速开始

### 1. 为库创建 CMakeLists.txt

```bash
# 复制模板到库目录
cp CMakeLists.txt.template src/base/CMakeLists.txt

# 编辑文件，修改以下变量：
# - LIBRARY_NAME: 库名称（如 "base"）
# - SOURCE_FILES: 源文件列表
```

### 2. 基本配置

在 CMakeLists.txt 中设置库特定信息：

```cmake
# 设置库名称
set(LIBRARY_NAME "base")

# 设置源文件
set(SOURCE_FILES
    "error_code.cpp"
    "time.cpp"
    # 添加更多源文件...
)
```

## 配置选项

### C++ 标准版本

**默认值**: C++11

**设置方法**:

```bash
# 方法1: 通过 CMake 命令行参数
cmake -DCMAKE_CXX_STANDARD=17 ..

# 方法2: 在 CMakeLists.txt 中修改
set(CMAKE_CXX_STANDARD 17)
```

**支持的版本**: 11, 14, 17, 20, 23

### 构建类型 (Build Type)

**默认值**: Release

**设置方法**:

```bash
# 方法1: 通过 CMake 命令行参数
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# 方法2: 在 CMakeLists.txt 中修改
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
```

**支持的构建类型**:

- **Debug**: 包含调试信息，无优化 (`-g -O0`)
- **Release**: 优化构建，无调试信息 (`-O3 -DNDEBUG`)
- **RelWithDebInfo**: 优化构建，包含调试信息 (`-O2 -g -DNDEBUG`)
- **MinSizeRel**: 最小体积优化 (`-Os -DNDEBUG`)

### 代码覆盖率

**默认值**: OFF

**启用方法**:

```bash
cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
make
make coverage  # 生成覆盖率报告
```

**输出**:
- `coverage.info`: 覆盖率数据文件
- `coverage_html/`: HTML 格式的覆盖率报告

**前提条件**:
- 需要安装 `lcov` 和 `genhtml`:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install lcov
  
  # macOS
  brew install lcov
  ```

**注意**: 覆盖率功能仅在 Debug 模式下有效。

### Sanitizer 工具

#### AddressSanitizer (ASAN)

检测内存错误（如缓冲区溢出、使用已释放内存等）：

```bash
cmake -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug ..
make
```

#### ThreadSanitizer (TSAN)

检测线程竞争条件：

```bash
cmake -DENABLE_TSAN=ON -DCMAKE_BUILD_TYPE=Debug ..
make
```

**注意**: ASAN 和 TSAN 不能同时启用。

#### UndefinedBehaviorSanitizer (UBSAN)

检测未定义行为：

```bash
cmake -DENABLE_UBSAN=ON -DCMAKE_BUILD_TYPE=Debug ..
make
```

#### 通用 Sanitizer 选项

```bash
cmake -DENABLE_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug ..
# 等同于 -DENABLE_ASAN=ON
```

## 使用示例

### 示例 1: 基本编译（Release 模式，C++17）

```bash
mkdir build && cd build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release ..
make
```

### 示例 2: Debug 模式编译

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### 示例 3: 启用代码覆盖率

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
make coverage  # 生成覆盖率报告
# 查看报告: open coverage_html/index.html
```

### 示例 4: 启用 AddressSanitizer

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON ..
make
```

### 示例 5: 组合使用多个选项

```bash
mkdir build && cd build
cmake \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON \
    -DENABLE_UBSAN=ON \
    ..
make
make coverage
```

## 依赖库配置

如果库依赖其他 lldk 库，在 CMakeLists.txt 中添加：

```cmake
# 例如：logger 库依赖 base 库
target_link_libraries(lldk_logger
    PUBLIC
        lldk_base  # 公共依赖，使用此库的用户也会链接 base
    PRIVATE
        # 私有依赖，仅此库内部使用
)
```

## 安装

模板已包含安装配置，使用标准 CMake 安装：

```bash
cmake --build . --target install
# 或
make install
```

安装路径可以通过 `CMAKE_INSTALL_PREFIX` 指定：

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

## 项目结构示例

```
lldk/
├── CMakeLists.txt.template          # 模板文件
├── src/
│   ├── base/
│   │   ├── CMakeLists.txt          # 基于模板创建
│   │   └── error_code.cpp
│   ├── logger/
│   │   ├── CMakeLists.txt          # 基于模板创建
│   │   └── logger.cpp
│   └── thread/
│       ├── CMakeLists.txt          # 基于模板创建
│       └── thread.cpp
└── include/
    └── lldk/
        ├── base/
        ├── logger/
        └── thread/
```

## 常见问题

### Q: 如何查看当前配置？

A: CMake 配置时会输出配置信息：

```
==========================================
Building library: lldk_base
C++ Standard: 17
Build Type: Debug
Coverage: ON
Sanitizer: OFF
==========================================
```

### Q: 覆盖率报告在哪里？

A: 在构建目录下的 `coverage_html/` 文件夹中，打开 `index.html` 查看。

### Q: 为什么启用覆盖率后编译失败？

A: 确保：
1. 使用 Debug 模式：`-DCMAKE_BUILD_TYPE=Debug`
2. 已安装 lcov：`apt-get install lcov` 或 `brew install lcov`

### Q: 如何清理构建？

A: 
```bash
rm -rf build/
# 或
cd build && make clean
```

## 高级配置

### 自定义编译选项

在 CMakeLists.txt 中添加：

```cmake
# 添加自定义编译选项
target_compile_options(lldk_${LIBRARY_NAME} PRIVATE
    -Werror  # 将警告视为错误
    -Wextra  # 启用额外警告
)
```

### 条件编译

```cmake
# 根据平台设置不同选项
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(lldk_${LIBRARY_NAME} PRIVATE LINUX_BUILD)
endif()
```

## 参考

- [CMake 官方文档](https://cmake.org/documentation/)
- [lcov 文档](https://github.com/linux-test-project/lcov)
- [Sanitizer 文档](https://github.com/google/sanitizers)

