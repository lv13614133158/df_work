cmake_minimum_required(VERSION 3.4.1)



project(https C CXX)


include_directories(include)

include_directories(./)

#依赖库的路径
link_directories(lib)

include_directories(common/include)
include_directories(common/include/clock)
include_directories(common/include/crypto)
include_directories(common/include/timer)
include_directories(common/include/util)

include_directories(networkmanager/include)
include_directories(networkmanager/include/aes)
include_directories(networkmanager/include/data)
include_directories(networkmanager/include/protobuf)
include_directories(networkmanager/include/util)

aux_source_directory(common/src/			    				SOURCE_COMMON) 
aux_source_directory(common/src/clock 							SOURCE_COMMON_CLOCK)
aux_source_directory(common/src/crypto 						SOURCE_COMMON_CRYPTO)
aux_source_directory(common/src/timer 							SOURCE_COMMON_TIMER)
aux_source_directory(common/src/util 							SOURCE_COMMON_UTIL)
aux_source_directory(networkmanager/src 	    				SOURCE_NETWORKMANAGER)
aux_source_directory(networkmanager/src/aes 					SOURCE_NETWORKMANAGER_AES)
aux_source_directory(networkmanager/src/data 					SOURCE_NETWORKMANAGER_DATA)
aux_source_directory(networkmanager/src/protobuf 				SOURCE_NETWORKMANAGER_PROBUF)
aux_source_directory(networkmanager/src/util 					SOURCE_NETWORKMANAGER_UTIL)

#auto find src



add_executable(https 
		./main.c
		./idshttps.c
		${SOURCE_COMMON}
		${SOURCE_COMMON_CLOCK}
		${SOURCE_COMMON_CRYPTO}
		${SOURCE_COMMON_TIMER}
		${SOURCE_COMMON_UTIL}
		${SOURCE_NETWORKMANAGER}
		${SOURCE_NETWORKMANAGER_AES}
		${SOURCE_NETWORKMANAGER_DATA}
		${SOURCE_NETWORKMANAGER_PROBUF}
		${SOURCE_NETWORKMANAGER_SQLITE}
		${SOURCE_NETWORKMANAGER_UTIL}
		)


 #预编译好的库（注释掉缺失的库）
target_link_libraries( 
	https
	-lpthread
	-lrt
	-ldl
	-lz
	-lcurl
	-lssl
	-lcrypto
)

