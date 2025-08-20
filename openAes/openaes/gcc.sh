#!/bin/bash

# 配置的openssl 1.1.0j  路径变量    3.0版本不可用
OPENSSL_LIB_PATH="/home/nvidia/work/openAes/openssl-1.1.0j/build"

GCC=gcc
# 编译命令
${GCC} main.c base64.c \
    -I ./ \
    -L./ \
    -L${OPENSSL_LIB_PATH} \
    -lopenaes \
    -lssl \
    -lcrypto \
    -o aes_test