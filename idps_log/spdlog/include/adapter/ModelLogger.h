//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_MODELLOGGER_H
#define SPDLOGDEMO_MODELLOGGER_H
#include <string>
#include "spdlog/logger.h"
class ModelLogger{
private:
    std::string modleName;
    int level;
    std::shared_ptr<spdlog::logger> consoleLogger;
    std::shared_ptr<spdlog::logger> fileLogger;
    bool consoleLoggerStatus;
    bool fileLoggerStatus;
    bool loggerStatus;

public:
    ModelLogger(const std::string& modelName);
    void setLevel(int level);
    void setConsoleLogger(bool consoleFlag);
    void setFileWriteLogger(bool fileFlag);

    void setModelLogger(bool status);
    void setFileWriteLoggerPath();


    /**
   * 格式化日志内容
   * @param tag 日志标签
   * @param msg 日志内容
   */
    std::string formatMsg(const std::string &tag, const std::string &msg);
    spdlog::level::level_enum getEnumLevelFromInteger(int level);

    /**
     * verbose级别日志打印
     * @param verbose 级别日志信息
     */
    void v(const std::string &tag, const std::string &verbose);
    /**
     * debug级别日志打印
     * @param debug 级别日志信息
     */
    void d(const std::string &tag, const std::string &debug);
    /**
     * info级别日志打印
     * @param info 级别日志信息
     */
    void i(const std::string &tag, const std::string &info);
    /**
     * warn级别日志打印
     * @param warn 级别日志信息
     */
    void w(const std::string &tag, const std::string &warn);
    /**
     * error级别日志打印
     * @param error 级别日志信息
     */
    void e(const std::string &tag, const std::string &error);
    /**
     * asset级别日志打印
     * @param asset 级别日志信息
     */
    void a(const std::string &tag, const std::string &asset);
};


#endif //SPDLOGDEMO_MODELLOGGER_H
