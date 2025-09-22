#include <stdio.h>
#include <stdlib.h>
#include "zlog.h"

int main(int argc, char** argv) {
    int rc;
    zlog_category_t *category;
    
    // 初始化zlog系统
    rc = zlog_init("./zlog.conf");
    if (rc) {
        printf("zlog initialization failed\n");
        return -1;
    }

    // 获取日志分类
    category = zlog_get_category("my_cat");
    if (!category) {
        printf("Failed to get category\n");
        zlog_fini();
        return -2;
    }

    // 测试各种日志级别
    zlog_debug(category, "这是一个debug日志");
    zlog_info(category, "这是一个信息日志 ");
    zlog_notice(category, "这是一个通知日志");
    zlog_warn(category, "这是一个警告日志");
    zlog_error(category, "这是一个错误日志");
    zlog_fatal(category, "这是一个致命错误日志");

   
    // 清理zlog系统
    zlog_fini();
    
    printf("日志测试完成，请检查输出结果\n");
    return 0;
}