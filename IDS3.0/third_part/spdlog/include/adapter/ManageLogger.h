//
// Created by wangchunlong1 on 2020/4/29.
//

#ifndef SPDLOGDEMO_MANAGELOGGER_H
#define SPDLOGDEMO_MANAGELOGGER_H

#include <string>

#define LOG_PRINT_ANDROID_FORMATTING "%n-%v"
#define LOG_PRINT_COMMON_FORMATTING "%Y-%m-%d %H:%M:%S.%e %P-%t %L/%v"

class ManageLogger{
private:
    int level;
    bool consoleLogStatus;
    bool fileLogStatus;
    std::string parentFilePath;
    size_t maxFileSize;
    size_t maxFileNumbers;

    bool static hasEnding (std::string const &fullString, std::string const &ending);

public:
    ManageLogger();
    void setManagerLevel(int level);
    void setManagerLevel(const std::string& modelName, int level);
    void setManagerConsoleLogger(bool consoleFlag);
    void setManagerFileWriteLogger(bool fileFlag);

    int getLevel();
    bool getConsoleLogger();
    bool getFileWriteLogger();

    void setManagerFileWriteParms(const std::string& filePath, int max_file_size, int max_files);
    //设置文件打印的父路径（最大文件个数，文件最大尺寸设置默认值）（对所有模块生效）
    void setManagerFileWritePath(const std::string& filePath);

    std::string getFileWritePath();
    size_t getMaxFileSize();
    size_t getMaxFileNumbers();
    void setManagerModelLogger(const std::string& modelName,bool status);


    static void setConsoleLogger(bool consoleFlag);
    static void setFileWriteLogger(bool fileFlag);
    static void setFileWriteParms(const std::string& filePath, int max_file_size, int max_files);
    static void setFileWritePath(const std::string& filePath);
    static void setLevel(int level);
    static void setLevel(const std::string& modelName, int level);
    static void setModelLogger(const std::string& modelName, bool status);


    ManageLogger(const ManageLogger&)= delete;

    ManageLogger&operator=(const ManageLogger&)= delete;
    static ManageLogger& getInstance(){
        static ManageLogger singletonBBQueue;
        return singletonBBQueue;
    }

};

#endif //SPDLOGDEMO_MANAGELOGGER_H
