cmake_minimum_required(VERSION 3.4.1)



project(idps C CXX)

include(ExternalProject)
# 统一设置目标架构
set(TARGET_ARCH "x86_64-linux-gnu")
# 然后在各个库中使用统一的变量：
set(OPENSSL_CONFIG_TARGET "linux-x86_64")
set(CURL_CONFIG_TARGET ${TARGET_ARCH})
set(PCAP_CONFIG_TARGET ${TARGET_ARCH})  
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(OPENSSL_CC_PATH ) # openssl编译使用环境
set(LIB_TOOL_PATH "/home/nvidia/df/lib") # 依赖库源码地址
set(LIBRARY_DIR "/home/nvidia/df/lib/lib") # 设置动态库库文件所在目录
set(BUILD_LIB ON)    # 是否编译静态库
set(USE_LIB ON)      # 是否使用静态库（ON）或系统动态库


#头文件路径
include_directories(include)
include_directories(include/websocket)
include_directories(include/websocket/include)
include_directories(include/spdlog)
include_directories(networkmanager)
include_directories(common/include)
include_directories(common/include/clock)
include_directories(common/include/crypto)
include_directories(common/include/idpslog)
include_directories(common/include/timer)
include_directories(common/include/util)

include_directories(networkmanager/https/include)
include_directories(networkmanager/https/include/aes)
include_directories(networkmanager/https/include/data)
include_directories(networkmanager/https/include/protobuf)
include_directories(networkmanager/https/include/util)


#websocket 模块
include_directories(networkmanager/websocket/include)
include_directories(networkmanager/websocket/include/util)



if(BUILD_LIB)
    # 静态库模式 - 编译并链接生成的静态库
    # 配置 OpenSSL 外部项目
    ExternalProject_Add(openssl
        SOURCE_DIR ${LIB_TOOL_PATH}/openssl-1.1.0j
        CONFIGURE_COMMAND env
            <SOURCE_DIR>/Configure 
            ${OPENSSL_CONFIG_TARGET} 
            --prefix=<INSTALL_DIR> 
            no-asm
            -DL_ENDIAN
            --cross-compile-prefix=${OPENSSL_CC_PATH}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        INSTALL_DIR ${CMAKE_BINARY_DIR}/openssl
    )
    # 获取 OpenSSL 安装路径
    ExternalProject_Get_Property(openssl INSTALL_DIR)
    set(OPENSSL_INSTALL_DIR ${INSTALL_DIR})
  
    include_directories(${OPENSSL_INSTALL_DIR}/include)
 
    ExternalProject_Add(curl
        SOURCE_DIR ${LIB_TOOL_PATH}/curl-7.81.0
        CONFIGURE_COMMAND env
            CC=${CMAKE_C_COMPILER}
            AR=${CMAKE_AR}
            RANLIB=${CMAKE_RANLIB}
            CPPFLAGS=-I${OPENSSL_INSTALL_DIR}/include
            LDFLAGS=-L${OPENSSL_INSTALL_DIR}/lib
            ${LIB_TOOL_PATH}/curl-7.81.0/configure
            --prefix=<INSTALL_DIR>
            --host=${CURL_CONFIG_TARGET}
            --with-ssl=${OPENSSL_INSTALL_DIR}
            --enable-static
            --disable-shared
            --without-zlib
            --disable-threaded-resolver
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        INSTALL_DIR ${CMAKE_BINARY_DIR}/curl
        BUILD_ALWAYS 1 
    )
    # 获取 curl 安装路径
    ExternalProject_Get_Property(curl INSTALL_DIR)
    set(CURL_INSTALL_DIR ${INSTALL_DIR})
    add_dependencies(curl openssl)
    include_directories(${CURL_INSTALL_DIR}/include)

    # 配置 libpcap 外部项目
    ExternalProject_Add(pcap
        SOURCE_DIR ${LIB_TOOL_PATH}/libpcap-1.9.1
        CONFIGURE_COMMAND env
            <SOURCE_DIR>/configure
            --host=${PCAP_CONFIG_TARGET}
            --prefix=<INSTALL_DIR>
            --disable-static   
            --enable-shared
            CC=${CMAKE_C_COMPILER}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        INSTALL_DIR ${CMAKE_BINARY_DIR}/pcap
    )
        # 获取 libpcap 安装路径
    ExternalProject_Get_Property(pcap INSTALL_DIR)
    set(PCAP_INSTALL_DIR ${INSTALL_DIR})
    include_directories(${PCAP_INSTALL_DIR}/include)

    # 配置 libwebsockets 外部项目
    ExternalProject_Add(libwebsockets
        SOURCE_DIR ${LIB_TOOL_PATH}/libwebsockets-4.3.1
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_SYSTEM_NAME=Linux
            -DLWS_WITH_SSL=ON
            -DLWS_WITH_STATIC=ON
            -DLWS_WITH_SHARED=ON
            -DOPENSSL_ROOT_DIR=${OPENSSL_INSTALL_DIR}
            -DOPENSSL_INCLUDE_DIR=${OPENSSL_INSTALL_DIR}/include
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_C_FLAGS=-I${OPENSSL_INSTALL_DIR}/include
            -DCMAKE_EXE_LINKER_FLAGS=-L${OPENSSL_INSTALL_DIR}/lib
        INSTALL_DIR ${CMAKE_BINARY_DIR}/websockets
        BUILD_ALWAYS 1
    )
    ExternalProject_Get_Property(libwebsockets INSTALL_DIR)
    set(WEBSOCKETS_INSTALL_DIR ${INSTALL_DIR})
    add_dependencies(libwebsockets openssl)

   
    include_directories(${WEBSOCKETS_INSTALL_DIR}/include)

  
endif()




#依赖库的路径
link_directories(${CMAKE_SOURCE_DIR}/lib)



#auto find src
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/			    				SOURCE_COMMON) 
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/clock 							SOURCE_COMMON_CLOCK)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/crypto 						SOURCE_COMMON_CRYPTO)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/timer 							SOURCE_COMMON_TIMER)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/util 							SOURCE_COMMON_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/idpslog		 				SOURCE_COMMON_IDPSLOG)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/https/src 	    				SOURCE_NETWORKMANAGER)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/https/src/aes 					SOURCE_NETWORKMANAGER_AES)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/https/src/data 					SOURCE_NETWORKMANAGER_DATA)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/https/src/protobuf 				SOURCE_NETWORKMANAGER_PROBUF)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/https/src/util 					SOURCE_NETWORKMANAGER_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/websocket/src 							SOURCE_WEBSOCKET)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/websocket/src/util 						SOURCE_WEBSOCKET_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager SOURCE_NETWORKMANAGER_BASE)


add_executable(IDPS 
		./main.c
		${SOURCE_COMMON}
		${SOURCE_COMMON_CLOCK}
		${SOURCE_COMMON_CRYPTO}
		${SOURCE_COMMON_TIMER}
		${SOURCE_COMMON_UTIL}
		${SOURCE_COMMON_IDPSLOG}
		${SOURCE_NETWORKMANAGER}
		${SOURCE_NETWORKMANAGER_AES}
		${SOURCE_NETWORKMANAGER_DATA}
		${SOURCE_NETWORKMANAGER_PROBUF}
		${SOURCE_NETWORKMANAGER_SQLITE}
		${SOURCE_NETWORKMANAGER_UTIL}
		${SOURCE_NETWORKMANAGER_BASE}
		${SOURCE_WEBSOCKET}
		${SOURCE_WEBSOCKET_UTIL}
		)

if(USE_LIB)
    set(PCAP ${PCAP_INSTALL_DIR}/lib/libpcap.a)
    set(CURL ${CURL_INSTALL_DIR}/lib/libcurl.a)
    set(SSL ${OPENSSL_INSTALL_DIR}/lib/libssl.a)
    set(CRYPTO ${OPENSSL_INSTALL_DIR}/lib/libcrypto.a)
    set(WEBSOCKETS ${WEBSOCKETS_INSTALL_DIR}/lib/libwebsockets.a)
else()
    set(PCAP -lpcap)
    set(CURL -lcurl)
    set(SSL -lssl)
    set(CRYPTO -lcrypto)
    set(WEBSOCKETS -lwebsockets) 
endif()
 #预编译好的库（注释掉缺失的库）
target_link_libraries( 
	IDPS
	-lspdlog
	-lpthread
	-lrt
	-ldl
	-lz
	-lm
	-lstdc++
	${WEBSOCKETS}
    ${CURL}
    ${SSL}
    ${CRYPTO}
    ${PCAP}
)

