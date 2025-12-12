#!/bin/bash
mkdir -p lib

TOOLCHAIN_PATH=/home/nvidia/df/S73/ql-ag35-1806e-gcc-8.4.0-glibc-v1-toolchain
CC=${TOOLCHAIN_PATH}/bin/arm-openwrt-linux-gcc
CC=gcc
# 编译共享库
$CC -shared -fPIC -o lib/libidslog.so src/libidslog.c -I ./include -lpthread
