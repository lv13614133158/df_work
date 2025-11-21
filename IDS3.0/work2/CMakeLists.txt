cmake_minimum_required(VERSION 3.4.1)



project(idps C CXX)




#头文件路径
include_directories()
include_directories(include)
include_directories(common/include)
include_directories(common/include/clock)
include_directories(common/include/crypto)
include_directories(common/include/idpslog)
include_directories(common/include/timer)
include_directories(common/include/util)

include_directories(networkmanager/include)
include_directories(networkmanager/include/aes)
include_directories(networkmanager/include/data)
include_directories(networkmanager/include/protobuf)
include_directories(networkmanager/include/util)


#websocket 模块
include_directories(websocket/include)
include_directories(websocket/include/util)
#spdlog 模块




include_directories(websocket/include)
include_directories(openssl/include)
include_directories(curl/include)

#硬件支持


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
		${CMAKE_SOURCE_DIR}/main.c
		${CMAKE_SOURCE_DIR}/idswebsocket.c
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
	-lpthread
	-lrt
	-ldl
	-lz
	-lcurl
	-lssl
	-lcrypto
	-lwebsockets
	-lm
	-lstdc++
)

