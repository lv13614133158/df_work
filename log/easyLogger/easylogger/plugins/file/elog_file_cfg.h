/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: 这是文件日志插件的配置头文件。
 * Created on: 2015-07-30
 */

#ifndef _ELOG_FILE_CFG_H_
#define _ELOG_FILE_CFG_H_

/* EasyLogger 文件日志名称 */
#define ELOG_FILE_NAME                   "elog.log"
/* EasyLogger 文件日志路径 */
#define ELOG_FILE_PATH                   "./"
/* EasyLogger 文件日志缓冲区大小 */
#define ELOG_FILE_BUF_SIZE               1024
/* EasyLogger 文件最大大小 */
#define ELOG_FILE_MAX_SIZE               (10 * 1024 * 1024)
/* EasyLogger 文件最大轮转数量 */
#define ELOG_FILE_MAX_ROTATE             10
/* EasyLogger 文件日志重新打开小时 */
#define ELOG_FILE_REOPEN_HOUR            0
/* EasyLogger 文件日志重新打开分钟 */
#define ELOG_FILE_REOPEN_MIN             0

#endif /* _ELOG_FILE_CFG_H_ */