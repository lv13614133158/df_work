//
// Created by wangchunlong1 on 2020/4/30.
//

#include "ConsoleLoggerFactory.h"
#include <adapter/ManageLogger.h>
#include <adapter/Mutex.h>
#include <spdlog/spdlog.h>

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
#endif

#ifndef __ANDROID__
#include "spdlog/sinks/stdout_sinks.h"
#endif
static basic::Mutex gDBLock;
std::shared_ptr<spdlog::logger> ConsoleLoggerFactory::createLogger(const std::string &modelName){
    basic::AutoMutex l(gDBLock);
    auto console_logger = spdlog::get(modelName);
    if (console_logger) {
        return console_logger;
    }
#ifdef __ANDROID__
    console_logger = spdlog::android_logger_mt(modelName, "SPDLOG");
    console_logger->set_pattern(LOG_PRINT_ANDROID_FORMATTING);
#endif

#ifndef __ANDROID__
    console_logger = spdlog::stdout_logger_mt(modelName);
    console_logger->set_pattern(LOG_PRINT_COMMON_FORMATTING);
#endif
    return console_logger;
}
