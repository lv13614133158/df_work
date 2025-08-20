//
// Created by wangchunlong1 on 2020/4/30.
//
#include "ModelLogger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <adapter/ConsoleLoggerFactory.h>
#include <adapter/FileLoggerFactory.h>
#include <adapter/ManageLogger.h>
#include <spdlog/spdlog.h>


ModelLogger::ModelLogger(const std::string &modelName) :
            consoleLoggerStatus(ManageLogger::getInstance().getConsoleLogger()),
            fileLoggerStatus(ManageLogger::getInstance().getFileWriteLogger()),
            loggerStatus(true),
            modleName(modelName){

    consoleLogger = ConsoleLoggerFactory::createLogger(modelName + "_C");
    fileLogger = FileLoggerFactory::createLogger(modelName + "_F");
    setLevel(ManageLogger::getInstance().getLevel());
}

void ModelLogger::setLevel(int level) {
    ModelLogger::level = level;
    consoleLogger->flush();
    consoleLogger->set_level(getEnumLevelFromInteger(level));
    if (fileLogger) {
        fileLogger->flush();
        fileLogger->set_level(getEnumLevelFromInteger(level));
    }
}

void ModelLogger::setConsoleLogger(bool consoleFlag){
    consoleLoggerStatus = consoleFlag;
}
void ModelLogger::setFileWriteLogger(bool fileFlag){
    fileLoggerStatus = fileFlag;
    if (fileLoggerStatus && !fileLogger) {
        fileLogger = FileLoggerFactory::createLogger(modleName + "_F");
        if (fileLogger) {
            fileLogger->set_level(getEnumLevelFromInteger(level));
        }
    }
}

void ModelLogger::setModelLogger(bool status){
    loggerStatus = status;
}
void ModelLogger::setFileWriteLoggerPath(){
    if (fileLogger) {
        fileLogger->flush();
        spdlog::drop(modleName + "_F");
    }
    fileLogger = FileLoggerFactory::createLogger(modleName + "_F");
    if (fileLogger) {
        fileLogger->set_level(getEnumLevelFromInteger(level));
    }
}


/**
    * verbose级别日志打印
    * @param verbose 级别日志信息
    */
void ModelLogger::v(const std::string &tag, const std::string &verbose){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->trace(formatMsg(tag, verbose));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->trace(formatMsg(tag, verbose));
    }
}
/**
 * debug级别日志打印
 * @param debug 级别日志信息
 */
void ModelLogger::d(const std::string &tag, const std::string &debug){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->debug(formatMsg(tag, debug));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->debug(formatMsg(tag, debug));
    }
}
/**
 * info级别日志打印
 * @param info 级别日志信息
 */
void ModelLogger::i(const std::string &tag, const std::string &info){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->info(formatMsg(tag, info));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->info(formatMsg(tag, info));
    }
}
/**
 * warn级别日志打印
 * @param warn 级别日志信息
 */
void ModelLogger::w(const std::string &tag, const std::string &warn){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->warn(formatMsg(tag, warn));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->warn(formatMsg(tag, warn));
    }
}
/**
 * error级别日志打印
 * @param error 级别日志信息
 */
void ModelLogger::e(const std::string &tag, const std::string &error){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->error(formatMsg(tag, error));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->error(formatMsg(tag, error));
    }
}
/**
 * asset级别日志打印
 * @param assert 级别日志信息
 */
void ModelLogger::a(const std::string &tag, const std::string &assert){
    if (!loggerStatus) {
        return;
    }
    if (consoleLoggerStatus) {
        consoleLogger->critical(formatMsg(tag, assert));
    }
    if (fileLoggerStatus && fileLogger) {
        fileLogger->critical(formatMsg(tag, assert));
    }
}


std::string ModelLogger::formatMsg(const std::string &tag, const std::string &msg){
    return "[" + tag + "]:" + msg;
}

spdlog::level::level_enum ModelLogger::getEnumLevelFromInteger(int level){
    switch(level) {
        case SPDLOG_LEVEL_TRACE:
            return spdlog::level::trace;
        case SPDLOG_LEVEL_DEBUG:
            return spdlog::level::debug;
        case SPDLOG_LEVEL_INFO:
            return spdlog::level::info;
        case SPDLOG_LEVEL_WARN:
            return spdlog::level::warn;
        case SPDLOG_LEVEL_ERROR:
            return spdlog::level::err;
        case SPDLOG_LEVEL_CRITICAL:
            return spdlog::level::critical;
        case SPDLOG_LEVEL_OFF:
            return spdlog::level::off;
        default:
            return spdlog::level::n_levels;
    }
}

