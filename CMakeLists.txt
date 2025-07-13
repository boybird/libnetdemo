cmake_minimum_required(VERSION 3.28)
project(MyCPrograms C)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找 pkg-config
find_package(PkgConfig REQUIRED)
find_library(LIBEV_LIBRARY
    NAMES ev # The base name of the library (libev.so)
    HINTS /usr/lib/x86_64-linux-gnu # Your specified path
    PATH_SUFFIXES lib # Common subdirectory for libraries
)

if(NOT LIBEV_LIBRARY)
    message(FATAL_ERROR "libev library not found. Please ensure libev-dev is installed and its path is correctly specified or discoverable.")
endif()

add_library(libev::libev UNKNOWN IMPORTED)

set_target_properties(libev::libev PROPERTIES
    IMPORTED_LOCATION "${LIBEV_LIBRARY}" # The full path to the library file
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include" # Add the include directory if headers are standard
    # If headers are in a non-standard location, you might need to find them separately
    # like: find_path(LIBEV_INCLUDE_DIR ev.h HINTS /path/to/libev/include)
    # and then: INTERFACE_INCLUDE_DIRECTORIES "${LIBEV_INCLUDE_DIR}"
)


# 包含通过 pkg-config 找到的库的头文件路径
include_directories(${SQLITE3_INCLUDE_DIRS})


function(recurse_glob result dir pattern)
    file(GLOB files "${CMAKE_SOURCE_DIR}/${pattern}")
    set(${result} ${files} PARENT_SCOPE)
endfunction()

# 使用该函数
recurse_glob(ALL_C_FILES "${CMAKE_SOURCE_DIR}" "*.c")

# 排除 lib 目录下的 .c 文件
list(FILTER ALL_C_FILES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/lib/.*")
message("found c files ${ALL_C_FILES}")

# 为每个 .c 文件创建一个可执行文件
foreach(C_FILE ${ALL_C_FILES})
    # 获取文件名（不含路径和扩展名）
    get_filename_component(EXEC_NAME ${C_FILE} NAME_WE)
    
    # 创建可执行文件
    message("add execute ${EXEC_NAME} ${C_FILE}")
    add_executable(${EXEC_NAME} ${C_FILE})
    
    target_link_libraries(${EXEC_NAME} PRIVATE libev::libev)
    
endforeach()
