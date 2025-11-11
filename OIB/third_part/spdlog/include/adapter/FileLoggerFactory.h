//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_FILELOGGERFACTORY_H
#define SPDLOGDEMO_FILELOGGERFACTORY_H

#include <string>
#include "spdlog/logger.h"

class FileLoggerFactory{
private:
    bool static isValidFile(const std::string &filePath);
public:
    std::shared_ptr<spdlog::logger> static createLogger(const std::string &modelName);
};


#endif //SPDLOGDEMO_FILELOGGERFACTORY_H
