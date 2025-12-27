# ==============================================================================
# base 库的 CMakeLists.txt 示例
# 基于 CMakeLists.txt.template 模板
# ==============================================================================

cmake_minimum_required(VERSION 3.14)
project(lldk_base VERSION 1.0.0 LANGUAGES CXX)

# ==============================================================================
# 配置选项
# ==============================================================================

# C++ 标准版本 (默认: 11, 可通过 -DCMAKE_CXX_STANDARD=17 覆盖)
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 构建类型 (默认: Release, 可通过 -DCMAKE_BUILD_TYPE=Debug 覆盖)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# 代码覆盖率选项 (默认: OFF, 可通过 -DENABLE_COVERAGE=ON 启用)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# 其他调试选项
option(ENABLE_SANITIZER "Enable address sanitizer" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# ==============================================================================
# 编译选项
# ==============================================================================

# 基础编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Debug 模式选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Release 模式选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# RelWithDebInfo 模式选项
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")

# ==============================================================================
# 代码覆盖率配置
# ==============================================================================

if(ENABLE_COVERAGE)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # 添加覆盖率编译选项
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        
        # 查找覆盖率工具
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(LCOV_PATH AND GENHTML_PATH)
            message(STATUS "Code coverage enabled: lcov and genhtml found")
            
            # 创建覆盖率目标
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' '*/deps/*' --output-file coverage.info
                COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage_html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
        else()
            message(WARNING "Code coverage enabled but lcov/genhtml not found. Install with: apt-get install lcov")
        endif()
    else()
        message(WARNING "Code coverage requires Debug build type. Current: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

# ==============================================================================
# Sanitizer 配置
# ==============================================================================

if(ENABLE_SANITIZER OR ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

if(ENABLE_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

if(ENABLE_UBSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()

# ==============================================================================
# 库配置
# ==============================================================================

# 库名称
set(LIBRARY_NAME "base")

# 自动搜索当前目录下的源文件
# 选项：是否递归搜索子目录（默认: OFF，只搜索当前目录）
option(AUTO_SOURCE_RECURSE "Recursively search for source files in subdirectories" OFF)

if(AUTO_SOURCE_RECURSE)
    # 递归搜索当前目录及所有子目录下的 .cpp 文件
    file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )
    message(STATUS "Auto-detecting source files (recursive): ${CMAKE_CURRENT_SOURCE_DIR}")
else()
    # 只搜索当前目录下的 .cpp 文件（不递归）
    file(GLOB SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )
    message(STATUS "Auto-detecting source files: ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# 排除不需要编译的文件（如果有）
# 例如：排除测试文件、示例文件等
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*test.*\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*example.*\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*demo.*\\.cpp$")

# 如果自动搜索没有找到文件，使用手动指定的文件列表作为后备
if(NOT SOURCE_FILES)
    message(WARNING "No source files found automatically. Using fallback list.")
    set(SOURCE_FILES
        "error_code.cpp"
        # 如果自动搜索失败，可以在这里手动添加源文件
    )
endif()

# 对源文件列表进行排序，确保构建的确定性
list(SORT SOURCE_FILES)

# 输出找到的源文件
list(LENGTH SOURCE_FILES SOURCE_FILE_COUNT)
message(STATUS "Found ${SOURCE_FILE_COUNT} source file(s):")
foreach(SOURCE_FILE ${SOURCE_FILES})
    get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME)
    message(STATUS "  - ${SOURCE_NAME}")
endforeach()

# 设置头文件目录
# 从 src/base 目录向上两级找到项目根目录
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(INCLUDE_DIR "${PROJECT_ROOT_DIR}/include")

# 验证头文件目录是否存在
if(NOT EXISTS "${INCLUDE_DIR}")
    message(FATAL_ERROR "Include directory not found: ${INCLUDE_DIR}")
endif()
message(STATUS "Include directory: ${INCLUDE_DIR}")

# 创建库目标
add_library(lldk_${LIBRARY_NAME} STATIC
    ${SOURCE_FILES}
)

# 设置包含目录
target_include_directories(lldk_${LIBRARY_NAME}
    PUBLIC
        ${INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置编译特性
target_compile_features(lldk_${LIBRARY_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

# base 库没有依赖其他 lldk 库
# 如果有依赖，可以这样添加：
# target_link_libraries(lldk_${LIBRARY_NAME}
#     PUBLIC
#         # 公共依赖
#     PRIVATE
#         # 私有依赖
# )

# ==============================================================================
# 输出信息
# ==============================================================================

message(STATUS "==========================================")
message(STATUS "Building library: lldk_${LIBRARY_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Coverage: ${ENABLE_COVERAGE}")
message(STATUS "Sanitizer: ${ENABLE_SANITIZER}")
list(LENGTH SOURCE_FILES SOURCE_FILE_COUNT)
message(STATUS "Source files: ${SOURCE_FILE_COUNT} file(s) found")
message(STATUS "==========================================")

