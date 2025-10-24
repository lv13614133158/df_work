//
// Created by wangchunlong1 on 2020/4/30.
//

#include "FileLoggerFactory.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>
#include <adapter/ManageLogger.h>
#include <adapter/Mutex.h>

static basic::Mutex gDBLock;
std::shared_ptr<spdlog::logger> FileLoggerFactory::createLogger(const std::string &modelName){

    basic::AutoMutex l(gDBLock);

    bool isValidFile = FileLoggerFactory::isValidFile(ManageLogger::getInstance().getFileWritePath());
    if(!isValidFile){
        return nullptr;
    }
    auto file_logger = spdlog::get(modelName);
    if (!file_logger) {
        file_logger = spdlog::rotating_logger_mt(modelName, ManageLogger::getInstance().getFileWritePath() + modelName + ".log"
                , ManageLogger::getInstance().getMaxFileSize(), ManageLogger::getInstance().getMaxFileNumbers());
    }
    file_logger->set_pattern(LOG_PRINT_COMMON_FORMATTING);
    file_logger->flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(10));//5s执行一次flush操作
    return file_logger;
}

bool FileLoggerFactory::isValidFile(const std::string &filePath) {
    if (filePath.empty()) {
        return false;
    }
    //文件不存在
    if (access(filePath.c_str(), F_OK) != 0) {
        int result = mkdir(filePath.c_str(), 0777);
        if (result != 0) {
            return false;
        }
    }
    //此路径文件无读权限
    if (access(filePath.c_str(), R_OK) != 0) {
        return false;
    }
    //此路径文件无写权限
    bool result = access(filePath.c_str(), W_OK) == 0;
    return result;
}