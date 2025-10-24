//
// Created by tangxx on 5/19/21.
//

#include "adapter/BussinessLogger.h"
#include "adapter/ManageLogger.h"
#include "spdloglib.h"

void set_console_logger(bool consoleFlag) {
    ManageLogger::setConsoleLogger(consoleFlag);
}

void set_file_write_logger(bool fileWriteFlag) {
    ManageLogger::setFileWriteLogger(fileWriteFlag);
}

void set_file_logger(const char *filePath, int max_file_size, int max_files) {
    if (!filePath) {
        return;
    }
    ManageLogger::setFileWriteParms(filePath, max_file_size, max_files);
}

void set_file_write_path(const char *filePath) {
    if (!filePath) {
        return;
    }
    ManageLogger::setFileWritePath(filePath);
}

void set_level(int level) {
    ManageLogger::setLevel(level);
}

void log_v(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::v(tag, msg);
}

void log_d(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::d(tag, msg);
}

void log_i(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::i(tag, msg);
}

void log_w(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::w(tag, msg);
}

void log_e(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::e(tag, msg);
}

void log_a(const char *tag, const char *msg) {
    if (!msg || !tag) {
        return;
    }
    BussinessLogger::a(tag, msg);
}


void set_model_level(const char *modelName, int level) {
    if (!modelName) {
        return;
    }
    ManageLogger::setLevel(modelName, level);
}

void set_model_logger(const char *modelName, bool flag) {
    if (!modelName) {
        return;
    }
    ManageLogger::setModelLogger(modelName, flag);
}

void model_log_v(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::v(modelName, tag, msg);
}

void model_log_d(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::d(modelName, tag, msg);
}

void model_log_i(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::i(modelName, tag, msg);
}

void model_log_w(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::w(modelName, tag, msg);
}

void model_log_e(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::e(modelName, tag, msg);
}

void model_log_a(const char *modelName, const char *tag, const char *msg) {
    if (!modelName || !msg || !tag) {
        return;
    }
    BussinessLogger::a(modelName, tag, msg);
}