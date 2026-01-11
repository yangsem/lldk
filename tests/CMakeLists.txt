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

# 设置 utilities 路径
set(UTILITIES_DIR "${SRC_DIR}/utilities")

# 为每个测试文件创建可执行文件
foreach(TEST_FILE ${TEST_FILES})
    # 获取测试文件名（不含扩展名）
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    # 获取测试文件所在目录（相对于 tests 目录）
    get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
    get_filename_component(TEST_REL_DIR ${TEST_DIR} NAME)
    
    # 判断是否是 utilities 目录下的测试
    set(IS_UTILITIES_TEST FALSE)
    if(TEST_REL_DIR STREQUAL "utilities")
        set(IS_UTILITIES_TEST TRUE)
    endif()
    
    # 收集源文件列表
    set(SOURCE_FILES
        ${TEST_FILE}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    )
    
    # 如果是 utilities 目录下的测试，添加对应的源文件实现
    if(IS_UTILITIES_TEST)
        # 根据测试文件名映射到对应的源文件
        # test_lldk_thread_local -> lldk_thread_local.cpp
        # test_lldk_bitset -> (只有头文件，模板类)
        # test_lldk_unordered_map -> (只有头文件，模板类)
        
        if(TEST_NAME STREQUAL "test_lldk_thread_local")
            set(UTILITY_SOURCE "${UTILITIES_DIR}/lldk_thread_local.cpp")
            if(EXISTS "${UTILITY_SOURCE}")
                list(APPEND SOURCE_FILES "${UTILITY_SOURCE}")
                message(STATUS "Test ${TEST_NAME} includes source: ${UTILITY_SOURCE}")
            endif()
        endif()
        
        # 如果还有其他 utilities 的源文件需要添加，可以在这里扩展
        # 例如：
        # if(TEST_NAME STREQUAL "test_xxx")
        #     set(UTILITY_SOURCE "${UTILITIES_DIR}/xxx.cpp")
        #     if(EXISTS "${UTILITY_SOURCE}")
        #         list(APPEND SOURCE_FILES "${UTILITY_SOURCE}")
        #     endif()
        # endif()
    endif()
    
    # 创建测试可执行文件
    add_executable(${TEST_NAME} ${SOURCE_FILES})
    
    # 设置包含目录
    target_include_directories(${TEST_NAME} PRIVATE
        ${INCLUDE_DIR}
        ${SRC_DIR}/utilities
        ${GTEST_INCLUDE_DIR}
    )
    
    # 设置编译特性
    target_compile_features(${TEST_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
    
    # 链接库（utilities 测试不链接库，直接编译源码；其他测试链接对应库）
    if(IS_UTILITIES_TEST)
        # utilities 测试直接编译源码，不链接库
        # 但可能需要链接 base 库（如果 utilities 的源文件依赖 base 库）
        # 检查是否需要链接 base 库（例如 lldk_thread_local 需要 allocator）
        if(TEST_NAME STREQUAL "test_lldk_thread_local")
            if(TARGET lldk_base)
                target_link_libraries(${TEST_NAME} PRIVATE
                    lldk_base
                    ${GTEST_LIB}
                )
                message(STATUS "Test ${TEST_NAME} links to lldk_base (for allocator dependency)")
            else()
                target_link_libraries(${TEST_NAME} PRIVATE
                    ${GTEST_LIB}
                )
                message(WARNING "Test ${TEST_NAME} needs lldk_base but it's not found")
            endif()
        else()
            # 其他 utilities 测试可能不需要链接库（纯模板类）
            target_link_libraries(${TEST_NAME} PRIVATE
                ${GTEST_LIB}
            )
            message(STATUS "Test ${TEST_NAME} uses header-only utilities, no library link needed")
        endif()
    else()
        # 非 utilities 测试，链接对应的库
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

