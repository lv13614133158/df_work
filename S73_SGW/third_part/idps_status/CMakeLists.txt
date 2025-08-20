# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

SET(CMAKE_BUILD_TYPE "Release")
set(CURRENT_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -s -Os")

SET(CMAKE_SYSTEM_NAME Linux)
SET(SDKTARGETSYSROOT "/home/toolchain/HZ_EP36/5G/ql-ol-extsdk-ag551qcnaar03a02m4g_ocpu/ql-sysroots")
SET(TOOLCHAIN_DIR "/home/toolchain/S73/SGW/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)

include_directories(${CURRENT_DIR}/include)

add_library(idps_status
        SHARED
        ${CURRENT_DIR}/src/idps_status.c
        )
