cmake_minimum_required(VERSION 2.6)
project(CMake_tutorial)
set(CMAKE_CXX_STANDARD 11)

# 设置有关版本号的两个变量，版本号 1.0
# 这两个变量将会在后面的操作中被插入到源代码中
set(Tutorial_VERSION_MAJOR 1)
set(Tutorial_VERSION_MINOR 0)

# 提供一个选项：是否使用提供的数学函数？
option(USE_MYMATH "Use tutorial provided math implementation" ON)
include(CMakeDependentOption)
# 提供一个依赖选项，当上一项开启时提供本项，默认开启：是否检查系统提供了 log 和 exp 函数？
CMAKE_DEPENDENT_OPTION(Check_Function "Check whether this system provide the log and exp functions"
        ON "USE_MYMATH" OFF)
# 检查并设置相关变量
if (Check_Function)
    include(CheckFunctionExists)
    check_function_exists(log HAVE_LOG)
    check_function_exists(exp HAVE_EXP)
endif (Check_Function)

# 以 TutorialConfig.h.in 为模版
# 以上设置的变量将会影响此处的处理
# 替换相关变量并输出到 TutorialConfig.h
configure_file(
        "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
        "${PROJECT_BINARY_DIR}/TutorialConfig.h"
)
# 将构建目录添加到 include 的搜索路径中以便找到 TutorialConfig.h 文件
include_directories("${PROJECT_BINARY_DIR}")

if (USE_MYMATH)
    include_directories("${PROJECT_SOURCE_DIR}/MathFunctions")
    add_subdirectory(MathFunctions)
    set(EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
endif (USE_MYMATH)

set(SOURCE_FILES tutorial.cpp)
add_executable(CMake_tutorial ${SOURCE_FILES})
target_link_libraries(CMake_tutorial ${EXTRA_LIBS})

# 开启测试
# include(CTest)
enable_testing()

# 程序是否能够运行？
add_test(TutorialRuns CMake_tutorial 25)

# 使用信息是否正常？
add_test(TutorialUsage CMake_tutorial)
set_tests_properties(TutorialUsage
        PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*number")

# 定义一个宏以简化测试的添加
# 使用 PASS_REGULAR_EXPRESSION 测试属性来验证测试的输出是否包含某些字符串
macro(do_test arg result)
    add_test(TutorialComp${arg} CMake_tutorial ${arg})
    set_tests_properties(TutorialComp${arg}
            PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endmacro(do_test)

# 一系列基础测试
do_test(4 "4 is 2")
do_test(9 "9 is 3")
do_test(5 "5 is 2.236")
do_test(7 "7 is 2.645")
do_test(25 "25 is 5")
do_test(-25 "-25 is 0")
do_test(0.0001 "0.0001 is 0.01")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include (CPack)
