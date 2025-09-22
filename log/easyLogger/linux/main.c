#include "elog.h"
#include "elog_file.h" 
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Starting EasyLogger demo...\n");
    fflush(stdout);
    
    /* 初始化EasyLogger */
    elog_init();
    
    /* 设置过滤级别 */
    elog_set_filter_lvl(ELOG_LVL_VERBOSE);
    
    /* 设置日志格式 - 为所有级别设置完整格式 */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);

    elog_file_init();
    ElogFileCfg file_cfg = {
        .name = "/home/nvidia/df/df_work/log/easyLogger/linux/inc/my_log",           // 文件名前缀
        .max_size = 1024 * 1024,    // 1MB
        .max_rotate = 5             // 最多5个文件
    };
    elog_file_config(&file_cfg);
    /* 启动日志系统 */
    elog_start();
    
    printf("Logger started, now logging...\n");
    fflush(stdout);
    
    /* 测试日志输出 - 使用正确的函数 */
    elog_info("main", "Hello EasyLogger!");
    elog_debug("main", "This is debug message");
    elog_warn("main", "This is warning message");
    elog_error("main", "This is error message");
    elog_assert("main", "This is assert message");
    
    /* 等待日志输出完成 */
    sleep(1);
    fflush(stdout);
    
    /* 停止日志系统 */
    elog_stop();
    
    printf("Logger demo finished.\n");
    fflush(stdout);
    
    return 0;
}