//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_CONSOLELOGGERFACTORY_H
#define SPDLOGDEMO_CONSOLELOGGERFACTORY_H


#include <string>
#include "spdlog/logger.h"

class ConsoleLoggerFactory{
public:
    std::shared_ptr<spdlog::logger> static createLogger(const std::string &modelName);
};

#endif //SPDLOGDEMO_CONSOLELOGGERFACTORY_H
