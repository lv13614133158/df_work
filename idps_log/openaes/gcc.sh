#!/bin/bash

# 配置的openssl 1.1.0j  路径变量    3.0版本不可用
OPENSSL_ROOT="/home/nvidia/df/df_work/idps_log/openssl-1.1.0j"

GCC=gcc
# 编译命令
${GCC} main.c base64.c \
    -I ./ \
    -I /home/nvidia/df/df_work/idps_log/openssl-1.1.0j/include \
    -L ./ \
    -L ${OPENSSL_ROOT}/build \
    -lssl \
    -lcrypto \
    -o aes_test