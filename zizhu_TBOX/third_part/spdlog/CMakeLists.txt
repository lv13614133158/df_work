cmake_minimum_required(VERSION 3.4.1)
project(Spdloglib)

set(CMAKE_CXX_STANDARD 11)

include_directories(include
        include/adapter)

add_library(spdlog
        STATIC
        #SHARED
        scr/spdloglib.cpp
        scr/adapter/BussinessLogger.cpp
        scr/adapter/LoggerContainer.cpp
        scr/adapter/ManageLogger.cpp
        scr/adapter/ModelLogger.cpp
        scr/adapter/ConsoleLoggerFactory.cpp
        scr/adapter/FileLoggerFactory.cpp
        )

