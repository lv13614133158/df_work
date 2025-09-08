#!/usr/bin/env bash

# 检查是否提供了进程名参数
if [ $# -eq 0 ]; then
    echo "Usage: $0 <process_name> [output_file]"
    echo "Example: $0 IDPS /tmp/memory_stats.csv"
    exit 1
fi

# 获取传入的进程名参数
PROCESS_NAME=$1

# 设置输出文件名，默认为进程名_memory_stats.csv
if [ $# -eq 2 ]; then
    OUTPUT_FILE=$2
else
    OUTPUT_FILE="${PROCESS_NAME}_memory_stats.csv"
fi

# 设置每个文件最大行数（4万条数据）
MAX_LINES_PER_FILE=40000

# 初始化文件计数器和行计数器
file_counter=1
line_counter=0

# 生成带编号的文件名
get_filename() {
    if [ "$file_counter" -eq 1 ]; then
        echo "$OUTPUT_FILE"
    else
        # 在文件名扩展名前插入序号
        basename=$(basename "$OUTPUT_FILE")
        dirname=$(dirname "$OUTPUT_FILE")
        filename="${basename%.*}"
        extension="${basename##*.}"
        if [ "$filename" = "$extension" ]; then
            # 没有扩展名的情况
            echo "${dirname}/${filename}_${file_counter}.csv"
        else
            # 有扩展名的情况
            echo "${dirname}/${filename}_${file_counter}.${extension}"
        fi
    fi
}

# 写入CSV文件头部
write_header() {
    local file=$1
    echo "Timestamp,PID,Process,VmSize(KB),VmRSS(KB),VmHWM(KB)" > "$file"
}

# 获取当前使用的文件名
CURRENT_FILE=$(get_filename)

# 写入第一个文件的CSV头部
write_header "$CURRENT_FILE"

fun() {
    # 获取指定进程的PID
    PROCESS_pid=$(ps -A | grep "$PROCESS_NAME" | grep -v grep |grep -v bash| awk '{print $1}' | head -1)
    
    # 获取当前时间戳
    current_time=$(date "+%Y-%m-%d %H:%M:%S")
    
    if [ -n "$PROCESS_pid" ]; then
        if [ -f "/proc/$PROCESS_pid/status" ]; then
            # 提取内存指标
            mem_VmSize=$(cat /proc/$PROCESS_pid/status | grep VmSize | awk '{print $2}')
            mem_VmRSS=$(cat /proc/$PROCESS_pid/status | grep VmRSS | awk '{print $2}')
            mem_VmHWM=$(cat /proc/$PROCESS_pid/status | grep VmHWM | awk '{print $2}')
            
            # 处理空值
            [ -z "$mem_VmSize" ] && mem_VmSize="0"
            [ -z "$mem_VmRSS" ] && mem_VmRSS="0"
            [ -z "$mem_VmHWM" ] && mem_VmHWM="0"
            
            # 写入CSV文件
            echo "$current_time,$PROCESS_pid,$PROCESS_NAME,$mem_VmSize,$mem_VmRSS,$mem_VmHWM" >> "$CURRENT_FILE"
         #   echo "$current_time - PID: $PROCESS_pid, Process: $PROCESS_NAME, VmSize: ${mem_VmSize} KB, VmRSS: ${mem_VmRSS} KB, VmHWM: ${mem_VmHWM} KB"
        else
        #    echo "$current_time - Process status file not found for PID: $PROCESS_pid"
            echo "$current_time,,,$PROCESS_pid,$PROCESS_NAME,0,0,0" >> "$CURRENT_FILE"
        fi
    else
  #      echo "$current_time - Process '$PROCESS_NAME' not found"
        echo "$current_time,,,,$PROCESS_NAME,0,0,0" >> "$CURRENT_FILE"
    fi
    
    # 更新行计数器
    line_counter=$((line_counter + 1))
    
    # 检查是否需要创建新文件
    if [ "$line_counter" -ge "$MAX_LINES_PER_FILE" ]; then
        file_counter=$((file_counter + 1))
        CURRENT_FILE=$(get_filename)
        write_header "$CURRENT_FILE"
        line_counter=0
        echo "已创建新文件: $CURRENT_FILE (每4万条数据分割一次)"
    fi
}

echo "开始采集进程 $PROCESS_NAME 的内存数据，结果将保存到 $OUTPUT_FILE"
echo "每4万条数据会创建一个新文件"
echo "按 Ctrl+C 可以提前停止采集"

# 循环采集，每秒一次
count=1
while true
do
    fun
    sleep 1
    count=$((count + 1))
    
    # 每100次输出一次进度信息（可选）
    if [ $((count % 100)) -eq 0 ]; then
        echo "已采集 ${count} 次数据..."
    fi
done