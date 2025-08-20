# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
SET(TOOLCHAIN_DIR "/home/toolchain/S73/SGW/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu")
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)
SET(CMAKE_LINKER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-ld)
project(IDPS_SGW C CXX) 
cmake_minimum_required(VERSION 3.4.1)

set(CURRENT_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
#生成gdb的调试信息和不进行代码优化
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -fsigned-char")

SET(CMAKE_SYSTEM_NAME Linux)
SET(TOOLCHAIN_DIR "/home/toolchain/S73/SGW/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)

#set(GNU_FLAGS " -rdynamic -mapcs -Wall -pie -fpie -march=armv7-a -mfpu=neon -mfloat-abi=hard -funwind-tables -ffunction-sections")
#set(CMAKE_CXX_FLAGS "${GNU_FLAGS} ")
#set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")
#SET(CMAKE_FIND_ROOT_PATH  ${SDKTARGETSYSROOT}
#${SDKTARGETSYSROOT}/usr/include
#${SDKTARGETSYSROOT}/lib )

#头文件路径
include_directories(function)
include_directories(function/common/include)
include_directories(function/common/include/clock)
include_directories(function/common/include/crypto)
include_directories(function/common/include/sqlite)
include_directories(function/common/include/timer)
include_directories(function/common/include/util)
include_directories(function/common/include/idpslog)
include_directories(function/common/include/openaes)

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

include_directories(function/canmonitor)
include_directories(function/canmonitor/io/udp)
include_directories(function/canmonitor/io/mcu)
include_directories(function/canmonitor/protocol)
include_directories(function/canmonitor/rule_parses)
include_directories(function/canmonitor/rule_parses/common)
include_directories(function/canmonitor/rule_parses/function)

#第三方库
include_directories(third_part/curl/include)
include_directories(third_part/openssl/include)
include_directories(third_part/pcap/include)
include_directories(third_part/websocket/include)
include_directories(third_part/idps_status/include)

#硬件支持
include_directories(tier1_part/quecte/include)
include_directories(tier1_part/quecte/Qualcomm/qmi)
include_directories(tier1_part/quecte/Qualcomm/qmi-framework)
include_directories(tier1_part/swg_info/include)
include_directories(tier1_part/pki_hsm/include)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib:${CMAKE_SOURCE_DIR}/lib:/vendor/usr/lib/idps)

#依赖库的路径
link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/tier1_part/swg_info/lib)
link_directories(${CMAKE_SOURCE_DIR}/tier1_part/pki_hsm/lib)
link_directories(${CMAKE_SOURCE_DIR}/third_part/curl/lib)
link_directories(${CMAKE_SOURCE_DIR}/third_part/openssl/lib)
link_directories(${CMAKE_SOURCE_DIR}/third_part/pcap/lib)
#link_directories(${CMAKE_SOURCE_DIR}/third_part/websocket/lib)


#auto find src
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/			    				SOURCE_COMMON) 
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/clock 							SOURCE_COMMON_CLOCK)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/crypto 						SOURCE_COMMON_CRYPTO)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/timer 							SOURCE_COMMON_TIMER)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/util 							SOURCE_COMMON_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/sqlite		 					SOURCE_COMMON_SQLITE)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/idpslog		 				SOURCE_COMMON_IDPSLOG)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/common/src/openaes		 				SOURCE_COMMON_OPENAES)
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
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor	 							SOURCE_CAN)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/io/udp 						SOURCE_CAN_IO_UDP)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/io/mcu 						SOURCE_CAN_IO_MCU)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/protocol	 					SOURCE_CAN_PROTOCOL)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/rule_parses		    		SOURCE_CAN)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/rule_parses/common 			SOURCE_CAN_COMMON)
aux_source_directory(${CMAKE_SOURCE_DIR}/function/canmonitor/rule_parses/function   		SOURCE_CAN_FUNCTION)

add_executable(IDPS 
		${CMAKE_SOURCE_DIR}/function/idps_main.c
		${SOURCE_COMMON}
		${SOURCE_COMMON_CLOCK}
		${SOURCE_COMMON_CRYPTO}
		${SOURCE_COMMON_TIMER}
		${SOURCE_COMMON_UTIL}
		${SOURCE_COMMON_SQLITE}
		${SOURCE_COMMON_IDPSLOG}
		${SOURCE_COMMON_OPENAES}
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
		${SOURCE_CAN}
		${SOURCE_CAN_IO_UDP}
		${SOURCE_CAN_PROTOCOL}
		${SOURCE_CAN}
		${SOURCE_CAN_COMMON}
		${SOURCE_CAN_FUNCTION}
		${SOURCE_CAN_IO_MCU}
		)

#预编译好的库
target_link_libraries( # Specifies the target library.
	IDPS
	-lspdlog
	-lpthread
	-lrt
	-ldl
	-lssl
	-lcrypto
	-lcurl
	-lpcap
	-lwebsockets
	-lidpsProxy
	-lidpsReadInfo
	# -lnutils 
	-lidpsProxy
	-lseccrypto
	-lstdc++
	)
