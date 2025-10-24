//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_LOGGERCONTAINER_H
#define SPDLOGDEMO_LOGGERCONTAINER_H

#include <string>
#include "map"
#include "ModelLogger.h"

class LoggerContainer{
private:
    std::map<std::string, ModelLogger*> loggerContainer;
    LoggerContainer();
public:

    ModelLogger* getLogger();
    ModelLogger* getLogger(const std::string &modelName);
    void setLevel(int level);
    void setConsoleLogger(bool consoleFlag);
    void setFileWriteLogger(bool fileFlag);
    void setFileWriteLoggerPath();

    LoggerContainer(const LoggerContainer&)= delete;

    LoggerContainer&operator=(const LoggerContainer&)= delete;
    static LoggerContainer& getInstance(){
        static LoggerContainer singletonBBQueue;
        return singletonBBQueue;
    }

};

#endif //SPDLOGDEMO_LOGGERCONTAINER_H
