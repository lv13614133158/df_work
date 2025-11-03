//
// Created by tangxx on 5/19/21.
//

#ifndef SPDLOGLIB_SPDLOGLIB_H
#define SPDLOGLIB_SPDLOGLIB_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
    void set_console_logger(bool consoleFlag);
    void set_file_write_logger(bool fileWriteFlag);
    void set_file_logger(const char *filePath, int max_file_size, int max_files);
    void set_file_write_path(const char *filePath);

    void set_level(int level);
    void log_v(const char *tag, const char *msg);
    void log_d(const char *tag, const char *msg);
    void log_i(const char *tag, const char *msg);
    void log_i(const char *tag, const char *msg);
    void log_w(const char *tag, const char *msg);
    void log_e(const char *tag, const char *msg);
    void log_a(const char *tag, const char *msg);

    void set_model_level(const char *modelName, int level);
    void set_model_logger(const char *modelName, bool flag);
    void model_log_v(const char *modelName, const char *tag, const char *msg);
    void model_log_d(const char *modelName, const char *tag, const char *msg);
    void model_log_i(const char *modelName, const char *tag, const char *msg);
    void model_log_w(const char *modelName, const char *tag, const char *msg);
    void model_log_e(const char *modelName, const char *tag, const char *msg);
    void model_log_a(const char *modelName, const char *tag, const char *msg);
#ifdef __cplusplus
}
#endif
#endif