//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_BUSSINESSLOGGER_H
#define SPDLOGDEMO_BUSSINESSLOGGER_H

#include <string>

class BussinessLogger{
public:

    /**
     * verbose级别日志打印
     * @param verbose 级别日志信息
     */
    static void v(const std::string &tag, const std::string &verbose);
    /**
     * debug级别日志打印
     * @param debug 级别日志信息
     */
    static void d(const std::string &tag, const std::string &debug);
    /**
     * info级别日志打印
     * @param info 级别日志信息
     */
    static void i(const std::string &tag, const std::string &info);
    /**
     * warn级别日志打印
     * @param warn 级别日志信息
     */
    static void w(const std::string &tag, const std::string &warn);
    /**
     * error级别日志打印
     * @param error 级别日志信息
     */
    static void e(const std::string &tag, const std::string &error);
    /**
     * asset级别日志打印
     * @param asset 级别日志信息
     */
    static void a(const std::string &tag, const std::string &asset);




    /**
     * verbose级别日志打印
     * @param verbose 级别日志信息
     */
    static void v(const std::string &modelName, const std::string &tag, const std::string &verbose);
    /**
     * debug级别日志打印
     * @param debug 级别日志信息
     */
    static void d(const std::string &modelName, const std::string &tag, const std::string &debug);
    /**
     * info级别日志打印
     * @param info 级别日志信息
     */
    static void i(const std::string &modelName, const std::string &tag, const std::string &info);
    /**
     * warn级别日志打印
     * @param warn 级别日志信息
     */
    static void w(const std::string &modelName, const std::string &tag, const std::string &warn);
    /**
     * error级别日志打印
     * @param error 级别日志信息
     */
    static void e(const std::string &modelName, const std::string &tag, const std::string &error);
    /**
     * asset级别日志打印
     * @param asset 级别日志信息
     */
    static void a(const std::string &modelName, const std::string &tag, const std::string &asset);
};
#endif //SPDLOGDEMO_BUSSINESSLOGGER_H
