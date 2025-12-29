# ==============================================================================
# 测试用例的 CMakeLists.txt
# 
# 功能：
# 1. 自动检索所有测试用例（test_*.cpp）
# 2. 根据测试文件所在目录，自动链接对应的库
# 3. 链接 GoogleTest
# ==============================================================================

cmake_minimum_required(VERSION 3.14)
project(lldk_tests VERSION 1.0.0 LANGUAGES CXX)

# ==============================================================================
# 配置选项
# ==============================================================================

# C++ 标准版本
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# ==============================================================================
# 路径设置
# ==============================================================================

# 获取项目根目录（从 tests 目录向上一级）
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(INCLUDE_DIR "${PROJECT_ROOT_DIR}/include")
set(SRC_DIR "${PROJECT_ROOT_DIR}/src")
set(DEPS_DIR "${PROJECT_ROOT_DIR}/deps")

# 验证目录
if(NOT EXISTS "${INCLUDE_DIR}")
    message(FATAL_ERROR "Include directory not found: ${INCLUDE_DIR}")
endif()
if(NOT EXISTS "${SRC_DIR}")
    message(FATAL_ERROR "Source directory not found: ${SRC_DIR}")
endif()

message(STATUS "Project root: ${PROJECT_ROOT_DIR}")
message(STATUS "Include directory: ${INCLUDE_DIR}")
message(STATUS "Source directory: ${SRC_DIR}")

# ==============================================================================
# GoogleTest 配置
# ==============================================================================

# 查找 GoogleTest
set(GTEST_ROOT "${DEPS_DIR}/googletest")

# 检查 GoogleTest 是否存在
if(EXISTS "${GTEST_ROOT}/CMakeLists.txt")
    # 使用 add_subdirectory 方式（推荐，自动处理依赖）
    message(STATUS "Using add_subdirectory for GoogleTest...")
    add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/googletest)
    # gtest_main 已经包含了 gtest，所以只需要链接 gtest_main
    set(GTEST_LIB gtest_main)
    set(GMOCK_LIB gmock_main)
else()
    message(FATAL_ERROR "GoogleTest not found. Please ensure deps/googletest exists or build it using deps/build.sh")
endif()

# ==============================================================================
# 自动搜索测试文件
# ==============================================================================

# 递归搜索所有测试文件
file(GLOB_RECURSE TEST_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*/test_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)

# 排除 test_main.cpp（它是主入口，不是测试用例）
list(FILTER TEST_FILES EXCLUDE REGEX ".*test_main\\.cpp$")

# 对测试文件列表进行排序
list(SORT TEST_FILES)

# 输出找到的测试文件
list(LENGTH TEST_FILES TEST_FILE_COUNT)
message(STATUS "Found ${TEST_FILE_COUNT} test file(s):")
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    message(STATUS "  - ${TEST_NAME}")
endforeach()

# ==============================================================================
# 构建被测试的库
# ==============================================================================

# 自动发现所有库目录
file(GLOB LIB_DIRS "${SRC_DIR}/*")
list(FILTER LIB_DIRS INCLUDE REGEX ".*/[^/]+$")  # 只保留目录

# 为每个库目录添加子目录（如果存在 CMakeLists.txt）
foreach(LIB_DIR ${LIB_DIRS})
    get_filename_component(LIB_NAME ${LIB_DIR} NAME)
    if(EXISTS "${LIB_DIR}/CMakeLists.txt")
        message(STATUS "Adding library: ${LIB_NAME}")
        add_subdirectory(${LIB_DIR} ${CMAKE_BINARY_DIR}/libs/${LIB_NAME})
    endif()
endforeach()

# ==============================================================================
# 创建测试可执行文件
# ==============================================================================

# 为每个测试文件创建可执行文件
foreach(TEST_FILE ${TEST_FILES})
    # 获取测试文件名（不含扩展名）
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    # 获取测试文件所在目录（相对于 tests 目录）
    get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
    get_filename_component(TEST_REL_DIR ${TEST_DIR} NAME)
    
    # 确定需要链接的库
    # 如果测试文件在 tests/base/ 目录，则链接 lldk_base
    set(TARGET_LIB "")
    if(TEST_REL_DIR STREQUAL "base" OR TEST_REL_DIR STREQUAL "")
        # 检查是否是根目录的测试（TEST_REL_DIR 为空表示在 tests/ 根目录）
        if(TEST_REL_DIR STREQUAL "")
            # 根目录测试，可能需要链接所有库或特定库
            # 这里可以根据需要调整
            set(TARGET_LIB "lldk_base")
        else()
            # 子目录测试，链接对应的库
            set(TARGET_LIB "lldk_${TEST_REL_DIR}")
        endif()
    else()
        # 其他目录的测试
        set(TARGET_LIB "lldk_${TEST_REL_DIR}")
    endif()
    
    # 创建测试可执行文件
    add_executable(${TEST_NAME}
        ${TEST_FILE}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    )
    
    # 设置包含目录
    target_include_directories(${TEST_NAME} PRIVATE
        ${INCLUDE_DIR}
        ${SRC_DIR}/utilities
        ${GTEST_INCLUDE_DIR}
    )
    
    # 设置编译特性
    target_compile_features(${TEST_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
    
    # 链接库
    if(TARGET ${TARGET_LIB})
        target_link_libraries(${TEST_NAME} PRIVATE
            ${TARGET_LIB}
            ${GTEST_LIB}  # gtest_main 已经包含了 gtest
        )
        message(STATUS "Test ${TEST_NAME} links to ${TARGET_LIB}")
    else()
        # 如果目标库不存在，只链接 GoogleTest
        target_link_libraries(${TEST_NAME} PRIVATE
            ${GTEST_LIB}  # gtest_main 已经包含了 gtest
        )
        message(WARNING "Library ${TARGET_LIB} not found, test ${TEST_NAME} will only link to GoogleTest")
    endif()
    
    # 添加到 CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # 设置测试属性
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 60
    )
endforeach()

# ==============================================================================
# 启用测试
# ==============================================================================

enable_testing()

# ==============================================================================
# 输出信息
# ==============================================================================

message(STATUS "==========================================")
message(STATUS "Test Configuration")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Test files: ${TEST_FILE_COUNT}")
message(STATUS "==========================================")

