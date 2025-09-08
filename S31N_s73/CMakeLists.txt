# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CURRENT_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
#生成gdb的调试信息和不进行代码优化
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")

# 设置交叉编译工具链

set(COMPILE_TOOL_PATH /home/nvidia/df/S73/ql-ag35-1806e-gcc-8.4.0-glibc-v1-toolchain)
set(SDKTARGETSYSROOT /home/nvidia/df/S73/ql-ol-extsdk-ag35cetcar01a08m2g_ocpu/ql-sysroots)
set(CMAKE_C_COMPILER ${COMPILE_TOOL_PATH}/bin/arm-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER ${COMPILE_TOOL_PATH}/bin/arm-openwrt-linux-g++)

project(idps C CXX)

# 设置包含和链接路径
link_directories(${COMPILE_TOOL_PATH}/lib ${SDKTARGETSYSROOT}/usr/lib)
#include_directories(${COMPILE_TOOL_PATH}/include ${SDKTARGETSYSROOT}/usr/include)
# 设置编译时缓冲区溢出检查
add_definitions(-D_FORTIFY_SOURCE=2)
#add_compile_definitions(-D_FORTIFY_SOURCE=2)

# 设置堆栈不可执行、重定位只读和立即绑定
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

# 设置 空间随机化 堆栈保护
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -fPIE -fstack-protector-all -fsigned-char")


# 设置 strip 
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_BUILD_TYPE "RELEASE")


#头文件路径
include_directories(function)
include_directories(function/common/include)
include_directories(function/common/include/clock)
include_directories(function/common/include/crypto)
include_directories(function/common/include/sqlite)
include_directories(function/common/include/timer)
include_directories(function/common/include/util)

include_directories(function/networkmanager/include)
include_directories(function/networkmanager/include/aes)
include_directories(function/networkmanager/include/data)
include_directories(function/networkmanager/include/protobuf)
include_directories(function/networkmanager/include/util)

include_directories(function/networkmonitor/include)
include_directories(function/networkmonitor/include/networkapi)
include_directories(function/networkmonitor/include/firewall)
include_directories(function/networkmonitor/include/firewall/libiptc)
include_directories(function/networkmonitor/include/firewall/linux)
include_directories(function/networkmonitor/include/firewall/linux/netfilter)
include_directories(function/networkmonitor/include/firewall/linux/netfilter_ipv4)
include_directories(function/networkmonitor/include/flow_count)

include_directories(function/filemonitor/include)
include_directories(function/filemonitor/include/fileapi)
include_directories(function/processmonitor/include)
include_directories(function/resourcemonitor/include)
include_directories(function/sysconfigmonitor/include)
include_directories(function/networkfirewall/include)
include_directories(function/configParse/include)
#websocket 模块
include_directories(function/websocket/include)
include_directories(function/websocket/include/util)
#spdlog 模块
include_directories(function/spdlog)


#第三方库
#include_directories(third_part/curl/include)
#include_directories(third_part/openssl/iude)
#include_directories(third_part/pcap/include)
include_directories(third_part/websocket/include)
include_directories(${SDKTARGETSYSROOT}/usr/include)
#硬件支持
include_directories(tier1_part/quecte/include)
include_directories(tier1_part/quecte/Qualcomm/qmi)
include_directories(tier1_part/quecte/Qualcomm/qmi-framework)
include_directories(tier1_part/tbox_info/include)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#TODO 需要修改lib路径
#set(CMAKE_INSTALL_RPATH $ORIGIN/../lib:${CMAKE_SOURCE_DIR}/lib:/oemapp/idps/lib)

#依赖库的路径
link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/tier1_part/tbox_info/lib)
link_directories(${CMAKE_SOURCE_DIR}/tier1_part/lib)
#link_directories(${CMAKE_SOURCE_DIR}/third_part/curl/lib)
#link_directories(${CMAKE_SOURCE_DIR}/third_part/openssl/lib)
#link_directories(${CMAKE_SOURCE_DIR}/third_part/pcap/lib)
#link_directories(${CMAKE_SOURCE_DIR}/third_part/websocket/lib)


#auto find src
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/			    				SOURCE_COMMON) 
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/clock 							SOURCE_COMMON_CLOCK)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/crypto 						SOURCE_COMMON_CRYPTO)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/timer 							SOURCE_COMMON_TIMER)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/util 							SOURCE_COMMON_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/sqlite		 					SOURCE_COMMON_SQLITE)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmanager/src 	    				SOURCE_NETWORKMANAGER)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmanager/src/aes 					SOURCE_NETWORKMANAGER_AES)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmanager/src/data 					SOURCE_NETWORKMANAGER_DATA)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmanager/src/protobuf 				SOURCE_NETWORKMANAGER_PROBUF)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmanager/src/util 					SOURCE_NETWORKMANAGER_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src 	    				SOURCE_NETWORKMONITOR)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/networkapi 			SOURCE_NETWORKMONITOR_API)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/firewall/iptables 	    SOURCE_NETWORKMONITOR_IPTABLE)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/firewall/libiptc 	    SOURCE_NETWORKMONITOR_IPTC)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/firewall/libxtables 	SOURCE_NETWORKMONITOR_XTABLE)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/firewall/modul 	    SOURCE_NETWORKMONITOR_MODUL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkmonitor/src/flow_count 	        SOURCE_NETWORKMONITOR_FLOW)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/filemonitor/src 							SOURCE_FILEMONITOR)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/filemonitor/src/fileapi 					SOURCE_FILEMONITOR_API)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/processmonitor/src 						SOURCE_PROCESSMONITOR)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/resourcemonitor/src 						SOURCE_RESOURCEMONITOR)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/sysconfigmonitor/src 						SOURCE_SYSCONFIGMONITOR)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/websocket/src 							SOURCE_WEBSOCKET)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/websocket/src/util 						SOURCE_WEBSOCKET_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/configParse/src 							SOURCE_CONFIGPARSE)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/networkfirewall/src 						SOURCE_NETWORKFIREWALL)

add_executable(IDPS 
		${CMAKE_SOURCE_DIR}/function/idps_main.c
		${SOURCE_COMMON}
		${SOURCE_COMMON_CLOCK}
		${SOURCE_COMMON_CRYPTO}
		${SOURCE_COMMON_TIMER}
		${SOURCE_COMMON_UTIL}
		${SOURCE_COMMON_SQLITE}
		${SOURCE_NETWORKMANAGER}
		${SOURCE_NETWORKMANAGER_AES}
		${SOURCE_NETWORKMANAGER_DATA}
		${SOURCE_NETWORKMANAGER_PROBUF}
		${SOURCE_NETWORKMANAGER_SQLITE}
		${SOURCE_NETWORKMANAGER_UTIL}
		${SOURCE_NETWORKMONITOR}
		${SOURCE_NETWORKMONITOR_API}
		${SOURCE_NETWORKMONITOR_IPTABLE}
		${SOURCE_NETWORKMONITOR_IPTC}
		${SOURCE_NETWORKMONITOR_XTABLE}
		${SOURCE_NETWORKMONITOR_MODUL}
		${SOURCE_NETWORKMONITOR_FLOW}
		${SOURCE_FILEMONITOR}
		${SOURCE_FILEMONITOR_API}
		${SOURCE_PROCESSMONITOR}
		${SOURCE_RESOURCEMONITOR}
		${SOURCE_SYSCONFIGMONITOR}
		${SOURCE_WEBSOCKET}
		${SOURCE_WEBSOCKET_UTIL}
		${SOURCE_CONFIGPARSE}
		${SOURCE_NETWORKFIREWALL}
		)

#预编译好的库
target_link_libraries( # Specifies the target library.
	IDPS
	-lspdlog
	-lpthread
	-lrt
	-ldl
	-lz
	-lcurl
	-lssl
	-lcrypto
	-lpcap
	-lwebsockets
	-lnghttp2
	-lssptbox
	-ldfsdkapi
	-lfdbus
	-ldfsdkproto
	-lprotobuf
	-labsl_log_internal_check_op
	-labsl_log_internal_message
	-llog
	-lprop2uci
	-luci
	-lubox
	-ldf_utils
	-ldfsdkproto
	-ldf_log
	-lql_sdk
	-lql_sys_log
	-lql_lib_ipc
	-lql_lib_utils
	-lteec
	-lql-tee-service
	-lm
	-lstdc++

	)


