cmake_minimum_required(VERSION 3.10)
project(MyCPrograms C)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 查找 pkg-config
find_package(PkgConfig REQUIRED)

# 查找 SQLite3 库（或其他通过 pkg-config 支持的库）
pkg_check_modules(SQLITE3 REQUIRED sqlite3)


# 包含通过 pkg-config 找到的库的头文件路径
include_directories(${SQLITE3_INCLUDE_DIRS})

# 获取当前目录下所有 .c 文件
file(GLOB_RECURSIVE ALL_C_FILES "${CMAKE_SOURCE_DIR}/*.c")

# 排除 lib 目录下的 .c 文件
list(FILTER ALL_C_FILES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/lib/.*")

# 为每个 .c 文件创建一个可执行文件
foreach(C_FILE ${ALL_C_FILES})
    # 获取文件名（不含路径和扩展名）
    get_filename_component(EXEC_NAME ${C_FILE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${EXEC_NAME} ${C_FILE})
    
    # 链接 pkg-config 找到的库
    target_link_libraries(${EXEC_NAME} ${SQLITE3_LIBRARIES})
    
    # 添加编译选项
    target_compile_options(${EXEC_NAME} PRIVATE ${SQLITE3_CFLAGS_OTHER})
endforeach()
