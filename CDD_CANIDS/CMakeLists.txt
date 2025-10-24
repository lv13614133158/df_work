cmake_minimum_required(VERSION 3.10)
project(UDS_CAN C)

set(CMAKE_C_STANDARD 99)

# 明确指定源文件搜索路径
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/function/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/system/*.c"
)

# 移除不需要的文件（如果有）
list(FILTER SOURCES EXCLUDE REGEX "test_.*\\.c")

add_executable(${PROJECT_NAME} ${SOURCES})
find_package(Threads REQUIRED)
# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/function
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system
)

# 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

