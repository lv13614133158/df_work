//
// Created by wangchunlong1 on 2020/4/30.
//
#include "BussinessLogger.h"
#include "LoggerContainer.h"

/**
     * verbose级别日志打印
     * @param verbose 级别日志信息
     */
void BussinessLogger::v(const std::string &tag, const std::string &verbose){
    LoggerContainer::getInstance().getLogger()->v(tag, verbose);
}
/**
 * debug级别日志打印
 * @param debug 级别日志信息
 */
void BussinessLogger::d(const std::string &tag, const std::string &debug){
    LoggerContainer::getInstance().getLogger()->d(tag, debug);
}
/**
 * info级别日志打印
 * @param info 级别日志信息
 */
void BussinessLogger::i(const std::string &tag, const std::string &info){
    LoggerContainer::getInstance().getLogger()->i(tag, info);
}
/**
 * warn级别日志打印
 * @param warn 级别日志信息
 */
void BussinessLogger::w(const std::string &tag, const std::string &warn){
    LoggerContainer::getInstance().getLogger()->w(tag, warn);
}
/**
 * error级别日志打印
 * @param error 级别日志信息
 */
void BussinessLogger::e(const std::string &tag, const std::string &error){
    LoggerContainer::getInstance().getLogger()->e(tag, error);
}
/**
 * asset级别日志打印
 * @param asset 级别日志信息
 */
void BussinessLogger::a(const std::string &tag, const std::string &asset){
    LoggerContainer::getInstance().getLogger()->a(tag, asset);
}




/**
 * verbose级别日志打印
 * @param verbose 级别日志信息
 */
void BussinessLogger::v(const std::string &modelName, const std::string &tag, const std::string &verbose){
    LoggerContainer::getInstance().getLogger(modelName)->v(tag, verbose);
}
/**
 * debug级别日志打印
 * @param debug 级别日志信息
 */
void BussinessLogger::d(const std::string &modelName, const std::string &tag, const std::string &debug){
    LoggerContainer::getInstance().getLogger(modelName)->d(tag, debug);
}
/**
 * info级别日志打印
 * @param info 级别日志信息
 */
void BussinessLogger::i(const std::string &modelName, const std::string &tag, const std::string &info){
    LoggerContainer::getInstance().getLogger(modelName)->i(tag, info);
}
/**
 * warn级别日志打印
 * @param warn 级别日志信息
 */
void BussinessLogger::w(const std::string &modelName, const std::string &tag, const std::string &warn){
    LoggerContainer::getInstance().getLogger(modelName)->w(tag, warn);
}
/**
 * error级别日志打印
 * @param error 级别日志信息
 */
void BussinessLogger::e(const std::string &modelName, const std::string &tag, const std::string &error){
    LoggerContainer::getInstance().getLogger(modelName)->e(tag, error);
}
/**
 * asset级别日志打印
 * @param asset 级别日志信息
 */
void BussinessLogger::a(const std::string &modelName, const std::string &tag, const std::string &asset){
    LoggerContainer::getInstance().getLogger(modelName)->a(tag, asset);
}
