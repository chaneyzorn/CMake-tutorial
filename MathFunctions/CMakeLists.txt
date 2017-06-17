# 添加生成表的执行程序的构建
add_executable(MakeTable MakeTable.cpp)
# 添加一个使用上面程序输出代码的命令
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
        COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
        DEPENDS MakeTable
)

# 将当前构建目录添加到包含路径以发现 Table.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 添加主库
add_library(MathFunctions mysqrt.cpp ${CMAKE_CURRENT_BINARY_DIR}/Table.h)
