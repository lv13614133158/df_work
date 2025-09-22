/*
 * 本文件是 EasyLogger 库的一部分。
 *
 * 版权所有 (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * 特此免费授予任何获得本软件及相关文档文件（以下简称"软件"）副本的人，
 * 无限制地处理本软件的权利，包括但不限于使用、复制、修改、合并、发布、
 * 分发、再许可和/或销售软件副本的权利，并允许向其提供软件的人员这样做，
 * 但须遵守以下条件：
 *
 * 上述版权声明和本许可声明应包含在所有副本或大部分软件中。
 *
 * 本软件按"原样"提供，不提供任何形式的明示或暗示保证，
 * 包括但不限于适销性、特定用途适用性和非侵权性的保证。
 * 在任何情况下，作者或版权持有人均不对因软件或使用或其他处理软件而
 * 引起或与之相关的任何索赔、损害或其他责任承担责任，
 * 无论是合同诉讼、侵权行为还是其他方式。
 *
 * 功能：这是本库的配置头文件。
 * 创建日期：2015-07-30
 */

#ifndef _ELOG_CFG_H_
#define _ELOG_CFG_H_

/* 启用日志输出。默认开启此宏 */
#define ELOG_OUTPUT_ENABLE
/* 启用终端输出。默认开启此宏 */
#define ELOG_TERMINAL_ENABLE
/* 启用日志写入文件。默认开启此宏 */
#define ELOG_FILE_ENABLE
/* 启用刷新文件缓存。默认开启此宏 */
#define ELOG_FILE_FLUSH_CACHE_ENABLE
/* 设置静态输出日志级别 */
#define ELOG_OUTPUT_LVL                      ELOG_LVL_VERBOSE
/* 启用断言检查 */
#define ELOG_ASSERT_ENABLE
/* 每行日志的缓冲区大小 */
#define ELOG_LINE_BUF_SIZE                   512
/* 输出行号最大长度 */
#define ELOG_LINE_NUM_MAX_LEN                5
/* 输出过滤器标签最大长度 */
#define ELOG_FILTER_TAG_MAX_LEN              16
/* 输出过滤器关键字最大长度 */
#define ELOG_FILTER_KW_MAX_LEN               16
/* 输出过滤器标签级别最大数量 */
#define ELOG_FILTER_TAG_LVL_MAX_NUM          5
/* 输出换行符 */
#define ELOG_NEWLINE_SIGN                    "\n"
/* 启用日志颜色 */
#define ELOG_COLOR_ENABLE
/* 启用日志颜色 */

/* 自定义颜色 */

// #define ELOG_COLOR_ASSERT   "\033[1;35m"  // 紫色加粗
// #define ELOG_COLOR_ERROR    "\033[1;31m"  // 红色加粗
// #define ELOG_COLOR_WARN     "\033[1;33m"  // 黄色加粗
// #define ELOG_COLOR_INFO     "\033[1;32m"  // 绿色加粗
// #define ELOG_COLOR_DEBUG    "\033[1;36m"  // 青色加粗
// #define ELOG_COLOR_VERBOSE  "\033[1;37m"  // 白色加粗
// #define ELOG_COLOR_END      "\033[0m"     // 颜色结束


/* 启用异步输出模式 */
#define ELOG_ASYNC_OUTPUT_ENABLE
/* 异步模式的最高输出级别，其他级别将同步输出 */
#define ELOG_ASYNC_OUTPUT_LVL                ELOG_LVL_DEBUG
/* 异步输出模式的缓冲区大小 */
#define ELOG_ASYNC_OUTPUT_BUF_SIZE           (ELOG_LINE_BUF_SIZE * 50)
/* 每个异步输出的日志必须以换行符结尾 */
#define ELOG_ASYNC_LINE_OUTPUT
/* 异步输出模式使用 POSIX pthread 实现 */
#define ELOG_ASYNC_OUTPUT_USING_PTHREAD

#endif /* _ELOG_CFG_H_ */