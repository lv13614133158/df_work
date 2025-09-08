#!/bin/bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

OPENSSL_LIB_PATH="${DIR}/openssl-1.1.0j"

GCC=gcc
${GCC} -Wno-deprecated-declarations main.c \
    ${DIR}/src/idpslog.c            \
    ${DIR}/src/openAes.c        \
    ${DIR}/src/base64.c         \
    ${DIR}/src/mysqlite.c       \
    ${DIR}/src/sqlite3.c        \
    ${DIR}/src/util.c           \
    -I${DIR}/include            \
    -I${OPENSSL_LIB_PATH}/include \
    -I${OPENSSL_LIB_PATH}/build/include \
    -L${DIR}/lib \
    -L${OPENSSL_LIB_PATH}/build \
    -lssl \
    -lcrypto \
    -lspdlog \
    -lstdc++ \
    -lm \
    -lpthread \
    -o idpslog.a


${GCC} -Wno-deprecated-declarations read_main.c \
    ${DIR}/src/idpslog.c            \
    ${DIR}/src/openAes.c        \
    ${DIR}/src/base64.c         \
    ${DIR}/src/mysqlite.c       \
    ${DIR}/src/sqlite3.c        \
    ${DIR}/src/util.c           \
    -I${DIR}/include            \
    -I${OPENSSL_LIB_PATH}/include \
    -I${OPENSSL_LIB_PATH}/build/include \
    -L${DIR}/lib \
    -L${OPENSSL_LIB_PATH}/build \
    -lssl \
    -lcrypto \
    -lspdlog \
    -lstdc++ \
    -lm \
    -lpthread \
    -o read.a


LD_LIBRARY_PATH=${OPENSSL_LIB_PATH}/build:${DIR}/:$LD_LIBRARY_PATH
echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"