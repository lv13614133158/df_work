cmake_minimum_required(VERSION 3.4.1)



project(idps C CXX)


# 添加格式溢出检查忽略选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -Wno-error=format-truncation -Wno-error=return-local-addr -Wno-error=format -Wno-error=stringop-overflow -Wno-error=unused-macros -Wno-error=format-overflow -fPIE -fstack-protector-all -fsigned-char")

# 设置编译时缓冲区溢出检查
add_definitions(-D_FORTIFY_SOURCE=2)

# 设置堆栈不可执行、重定位只读和立即绑定
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

# 设置 空间随机化 堆栈保护#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -fPIE -fstack-protector-all -fsigned-char")
# 添加格式溢出检查忽略选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -Wno-error=format-truncation -Wno-error=return-local-addr -Wno-error=format -Wno-error=stringop-overflow -Wno-error=unused-macros -Wno-error=format-overflow -fPIE -fstack-protector-all -fsigned-char")
# 在现有的 CMAKE_C_FLAGS 基础上添加 -Wno-format-truncation
Set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=unused-function -Wno-error=pointer-sign -Wno-stringop-overflow -Wno-format-overflow -Wno-format-truncation -fPIE -fstack-protector-all -fsigned-char")
# 设置 strip 
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_BUILD_TYPE "RELEASE")


#头文件路径
include_directories(include)
include_directories(networkmanager)
include_directories( common/include)
include_directories( common/include/clock)
include_directories( common/include/crypto)
include_directories( common/include/idpslog)
include_directories( common/include/timer)
include_directories( common/include/util)

include_directories( networkmanager/include)
include_directories( networkmanager/include/aes)
include_directories( networkmanager/include/data)
include_directories( networkmanager/include/protobuf)
include_directories( networkmanager/include/util)

include_directories( networkmonitor/include)
include_directories( networkmonitor/include/networkapi)
include_directories( networkmonitor/include/firewall)
include_directories( networkmonitor/include/firewall/libiptc)
include_directories( networkmonitor/include/firewall/linux)
include_directories( networkmonitor/include/firewall/linux/netfilter)
include_directories( networkmonitor/include/firewall/linux/netfilter_ipv4)
include_directories( networkmonitor/include/flow_count)

include_directories( filemonitor/include)
include_directories( filemonitor/include/fileapi)
include_directories( processmonitor/include)
include_directories( resourcemonitor/include)
include_directories( sysconfigmonitor/include)
include_directories( networkfirewall/include)
include_directories( configParse/include)
#websocket 模块
include_directories( websocket/include)
include_directories( websocket/include/util)
#spdlog 模块
include_directories( spdlog)





set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


#依赖库的路径
link_directories(${CMAKE_SOURCE_DIR}/lib)



#auto find src
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/			    				SOURCE_COMMON) 
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/clock 							SOURCE_COMMON_CLOCK)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/crypto 						SOURCE_COMMON_CRYPTO)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/timer 							SOURCE_COMMON_TIMER)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/util 							SOURCE_COMMON_UTIL)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src/idpslog		 				SOURCE_COMMON_IDPSLOG)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/src 	    				SOURCE_NETWORKMANAGER)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/src/aes 					SOURCE_NETWORKMANAGER_AES)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/src/data 					SOURCE_NETWORKMANAGER_DATA)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/src/protobuf 				SOURCE_NETWORKMANAGER_PROBUF)
aux_source_directory(${CMAKE_SOURCE_DIR}/networkmanager/src/util 					SOURCE_NETWORKMANAGER_UTIL)


add_executable(IDPS 
		./networkmanager/https_networkManger.c
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
		)


 #预编译好的库（注释掉缺失的库）
target_link_libraries( 
	IDPS
	-lspdlog
	-lpthread
	-lrt
	-ldl
	-lz
	-lcurl
	-lssl
	-lcrypto
	-lm
	-lstdc++
)

