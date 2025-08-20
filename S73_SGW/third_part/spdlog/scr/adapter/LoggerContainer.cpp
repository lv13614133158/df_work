//
// Created by wangchunlong1 on 2020/4/30.
//
#include "LoggerContainer.h"

LoggerContainer::LoggerContainer() {}

ModelLogger* LoggerContainer::getLogger(const std::string &modelName) {
    auto iter = loggerContainer.find(modelName);
    if (iter != loggerContainer.end()) {
        return iter->second;
    }
    ModelLogger* modelLogger = new ModelLogger(modelName); // OK
    loggerContainer[modelName] = modelLogger;
    return  modelLogger;
}

ModelLogger* LoggerContainer::getLogger() {
    std::string modelName = "CommonModel";
    auto iter = loggerContainer.find(modelName);
    if (iter != loggerContainer.end()) {
        return iter->second;
    }
    ModelLogger* modelLogger = new ModelLogger(modelName); // OK
    loggerContainer[modelName] = modelLogger;
    return  modelLogger;
}

void LoggerContainer::setLevel(int level){
    auto iter = loggerContainer.begin();
    while (iter != loggerContainer.end()) {
        iter->second->setLevel(level);
        iter++;
    }
}
void LoggerContainer::setConsoleLogger(bool consoleFlag){
    auto iter = loggerContainer.begin();
    while (iter != loggerContainer.end()) {
        iter->second->setConsoleLogger(consoleFlag);
        iter++;
    }
}
void LoggerContainer::setFileWriteLogger(bool fileFlag){
    auto iter = loggerContainer.begin();
    while (iter != loggerContainer.end()) {
        iter->second->setFileWriteLogger(fileFlag);
        iter++;
    }
}
void LoggerContainer::setFileWriteLoggerPath(){
    auto iter = loggerContainer.begin();
    while (iter != loggerContainer.end()) {
        iter->second->setFileWriteLoggerPath();
        iter++;
    }
}