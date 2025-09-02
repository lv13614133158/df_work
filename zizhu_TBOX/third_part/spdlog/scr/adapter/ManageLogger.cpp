//
// Created by wangchunlong1 on 2020/4/30.
//
#include "ManageLogger.h"
#include "LoggerContainer.h"

ManageLogger::ManageLogger() {}

void ManageLogger::setManagerLevel(int level){
    ManageLogger::level = level;
    LoggerContainer::getInstance().setLevel(level);
}
void ManageLogger::setManagerLevel(const std::string& modelName, int level){
    LoggerContainer::getInstance().getLogger(modelName)->setLevel(level);
}
void ManageLogger::setManagerConsoleLogger(bool consoleFlag){
    ManageLogger::consoleLogStatus = consoleFlag;
    LoggerContainer::getInstance().setConsoleLogger(consoleFlag);
}
void ManageLogger::setManagerFileWriteLogger(bool fileFlag){
    ManageLogger::fileLogStatus = fileFlag;
    LoggerContainer::getInstance().setFileWriteLogger(fileFlag);

}

int ManageLogger::getLevel(){
    return level;
}
bool ManageLogger::getConsoleLogger(){
    return consoleLogStatus;
}
bool ManageLogger::getFileWriteLogger(){
    return fileLogStatus;
}

void ManageLogger::setManagerFileWriteParms(const std::string& filePath, int max_file_size, int max_files){
    if(hasEnding(filePath, "/")){
        ManageLogger::parentFilePath = filePath;
    }else{
        ManageLogger::parentFilePath = filePath + "/";
    }
    ManageLogger::maxFileSize = max_file_size;
    ManageLogger::maxFileNumbers = max_files;
    LoggerContainer::getInstance().setFileWriteLoggerPath();
}
//设置文件打印的父路径（最大文件个数，文件最大尺寸设置默认值）（对所有模块生效）
void ManageLogger::setManagerFileWritePath(const std::string& filePath){
    setManagerFileWriteParms(filePath, 1024 * 1024 * 5, 5);
}

std::string ManageLogger::getFileWritePath(){
    return parentFilePath;
}
size_t ManageLogger::getMaxFileSize(){
    return maxFileSize;
}
size_t ManageLogger::getMaxFileNumbers(){
    return maxFileNumbers;
}

void ManageLogger::setManagerModelLogger(const std::string& modelName,bool status){
    LoggerContainer::getInstance().getLogger(modelName)->setModelLogger(status);
}


void ManageLogger::setConsoleLogger(bool consoleFlag){
    ManageLogger::getInstance().setManagerConsoleLogger(consoleFlag);
}
void ManageLogger::setFileWriteLogger(bool fileFlag){
    ManageLogger::getInstance().setManagerFileWriteLogger(fileFlag);
}
void ManageLogger::setFileWriteParms(const std::string& filePath, int max_file_size, int max_files){
    ManageLogger::getInstance().setManagerFileWriteParms(filePath, max_file_size, max_files);
}
void ManageLogger::setFileWritePath(const std::string& filePath){
    ManageLogger::getInstance().setManagerFileWritePath(filePath);
}
void ManageLogger::setLevel(int level){
    ManageLogger::getInstance().setManagerLevel(level);
}
void ManageLogger::setLevel(const std::string& modelName, int level){
    ManageLogger::getInstance().setManagerLevel(modelName, level);
}
void ManageLogger::setModelLogger(const std::string& modelName, bool status){
    ManageLogger::getInstance().setManagerModelLogger(modelName, status);
}

bool ManageLogger::hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}