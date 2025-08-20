cmake_minimum_required(VERSION 3.4.1)
set(TOOLCHAIN_DIR "/home/nvidia/df/HAD/S73a2/toolchain/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu")
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++)
set(CMAKE_LINKER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-ld)


project(Spdloglib)

set(CMAKE_CXX_STANDARD 11)


include_directories(include
        include/adapter)

add_library(spdlog
        STATIC
        #SHARED
        scr/spdloglib.cpp
        scr/adapter/BussinessLogger.cpp
        scr/adapter/LoggerContainer.cpp
        scr/adapter/ManageLogger.cpp
        scr/adapter/ModelLogger.cpp
        scr/adapter/ConsoleLoggerFactory.cpp
        scr/adapter/FileLoggerFactory.cpp
        )

