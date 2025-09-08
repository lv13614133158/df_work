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

# 统计值初始化
max_usr_cpu=0
min_usr_cpu=100
sum_usr_cpu=0

max_sys_cpu=0
min_sys_cpu=100
sum_sys_cpu=0

max_idle_cpu=100
min_idle_cpu=100
sum_idle_cpu=0

max_used_mem=0
min_used_mem=999999999
sum_used_mem=0

max_free_mem=0
min_free_mem=999999999
sum_free_mem=0

max_IDS_cpu=0
min_IDS_cpu=100
sum_IDS_cpu=0

max_IDS_mem_RSS=0
min_IDS_mem_RSS=999999999
sum_IDS_mem_RSS=0

count_stats=0

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
    echo "Timestamp,PID,Process,usr_cpu(%),sys_cpu(%),idle_cpu(%),used_mem,free_mem,IDS_cpu(%),IDS_mem_RSS(KB)" > "$file"
}

# 获取当前使用的文件名
CURRENT_FILE=$(get_filename)

# 写入第一个文件的CSV头部
write_header "$CURRENT_FILE"

# 获取系统CPU和内存使用情况
get_system_stats() {
    # 获取CPU使用情况 - 使用更准确的方法解析top输出
    cpu_line=$(top -bn1 | grep "Cpu(s)" | head -1)
    

    # 提取用户态CPU使用率
    
    usr_cpu=$(top -bn1 | grep "Cpu(s)" | head -1 | awk -F',' '{print $1}' | awk '{print $2}')
    sys_cpu=$(top -bn1 | grep "Cpu(s)" | head -1 | awk -F',' '{print $2}'| awk '{print $1}')
    idle_cpu=$(top -bn1 | grep "Cpu(s)" | head -1 | awk -F',' '{print $4}' | awk '{print $1}')

    
    # 获取内存使用情况（MB）
    mem_line=$(free -m | grep "Mem:")
    used_mem=$(echo "$mem_line" | awk '{print $3}')
    free_mem=$(echo "$mem_line" | awk '{print $4}')
    
}   

# 获取进程CPU和内存使用情况
get_process_stats() {
    PROCESS_pid=$(ps -A | grep "$PROCESS_NAME" | grep -v grep | grep -v bash | awk '{print $1}' | head -1)
    
    if [ -n "$PROCESS_pid" ]; then
        # 获取进程CPU使用百分比
        IDS_cpu=$(top -bn1 -p $PROCESS_pid | grep $PROCESS_pid | awk '{print $9}')
        [ -z "$IDS_cpu" ] && IDS_cpu="0"
        
        # 获取进程内存使用（KB）
        if [ -f "/proc/$PROCESS_pid/status" ]; then
            IDS_mem_RSS=$(grep VmRSS /proc/$PROCESS_pid/status | awk '{print $2}')
            [ -z "$IDS_mem_RSS" ] && IDS_mem_RSS="0"
        else
            IDS_mem_RSS="0"
        fi
    else
        PROCESS_pid=""
        IDS_cpu="0"
        IDS_mem_RSS="0"
    fi
}

# 打印统计数据
print_stats() {
    if [ $count_stats -gt 0 ]; then
        # 使用更安全的方式计算平均值
        if command -v bc >/dev/null 2>&1; then
          avg_usr_cpu=$(echo "scale=2; $sum_usr_cpu / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_sys_cpu=$(echo "scale=2; $sum_sys_cpu / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_idle_cpu=$(echo "scale=2; $sum_idle_cpu / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_used_mem=$(echo "scale=2; $sum_used_mem / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_free_mem=$(echo "scale=2; $sum_free_mem / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_IDS_cpu=$(echo "scale=2; $sum_IDS_cpu / $count_stats" | bc 2>/dev/null | awk '{printf "%.2f", $0}' || echo "0.00")
            avg_IDS_mem_RSS=$(echo "scale=0; $sum_IDS_mem_RSS / $count_stats" | bc 2>/dev/null | awk '{printf "%.0f", $0}' || echo "0.00")
        else
            # 如果没有bc命令，使用简单的整数运算
            avg_usr_cpu=0
            avg_sys_cpu=0
            avg_idle_cpu=0
            avg_used_mem=0
            avg_free_mem=0
            avg_IDS_cpu=0
            avg_IDS_mem_RSS=0
        fi
        
        # 格式化输出，减小间距
        printf "%-6s  %-6s  %-6s  %-8s  %-8s  %-8s  %-8s  %-6s\n" \
            "avg" "${avg_usr_cpu}%" "${avg_sys_cpu}%" "${avg_idle_cpu}%" \
            "${avg_used_mem}" "${avg_free_mem}" "${avg_IDS_cpu}%" "${avg_IDS_mem_RSS}K"
        printf "%-6s  %-6s  %-6s  %-8s  %-8s  %-8s  %-8s  %-6s\n" \
            "max" "${max_usr_cpu}%" "${max_sys_cpu}%" "${max_idle_cpu}%" \
            "${max_used_mem}" "${max_free_mem}" "${max_IDS_cpu}%" "${max_IDS_mem_RSS}K"
        printf "%-6s  %-6s  %-6s  %-8s  %-8s  %-8s  %-8s  %-6s\n" \
            "min" "${min_usr_cpu}%" "${min_sys_cpu}%" "${min_idle_cpu}%" \
            "${min_used_mem}" "${min_free_mem}" "${min_IDS_cpu}%" "${min_IDS_mem_RSS}K"
        echo "------------------------------------------------------------"
    fi
}

fun() {
    # 获取当前时间戳
    current_time=$(date "+%Y-%m-%d %H:%M:%S")
    
    # 获取系统统计信息
    get_system_stats
    
    # 获取进程统计信息
    get_process_stats
    
    # 更新统计数据
    count_stats=$((count_stats + 1))
    
    # 更新CPU统计值（添加更多检查）
    if [ -n "$usr_cpu" ] && [ "$usr_cpu" != "" ] && [ "$usr_cpu" != " " ]; then
        # 使用awk进行数值比较，避免bc语法错误
        if [ "$(echo "$usr_cpu $max_usr_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_usr_cpu=$usr_cpu
        fi
        
        # 只有当usr_cpu不为0时才更新最小值
        if [ "$(echo "$usr_cpu $min_usr_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_usr_cpu=$usr_cpu
        fi
        
        sum_usr_cpu=$(echo "$sum_usr_cpu $usr_cpu" | awk '{print $1 + $2}')
    fi
    
    if [ -n "$sys_cpu" ] && [ "$sys_cpu" != "" ] && [ "$sys_cpu" != " " ]; then
        if [ "$(echo "$sys_cpu $max_sys_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_sys_cpu=$sys_cpu
        fi
        
        if [ "$(echo "$sys_cpu $min_sys_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_sys_cpu=$sys_cpu
        fi
        
        sum_sys_cpu=$(echo "$sum_sys_cpu $sys_cpu" | awk '{print $1 + $2}')
    fi
    
    if [ -n "$idle_cpu" ] && [ "$idle_cpu" != "" ] && [ "$idle_cpu" != " " ]; then
        if [ "$(echo "$idle_cpu $max_idle_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_idle_cpu=$idle_cpu
        fi
        
        if [ "$(echo "$idle_cpu $min_idle_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_idle_cpu=$idle_cpu
        fi
        
        sum_idle_cpu=$(echo "$sum_idle_cpu $idle_cpu" | awk '{print $1 + $2}')
    fi
    
    # 更新内存统计值
    if [ -n "$used_mem" ] && [ "$used_mem" != "" ] && [ "$used_mem" != " " ]; then
        if [ "$used_mem" -gt "$max_used_mem" ] 2>/dev/null; then 
            max_used_mem=$used_mem
        fi
        
        if [ "$used_mem" -lt "$min_used_mem" ] 2>/dev/null; then 
            min_used_mem=$used_mem
        fi
        
        sum_used_mem=$((sum_used_mem + used_mem))
    fi
    
    if [ -n "$free_mem" ] && [ "$free_mem" != "" ] && [ "$free_mem" != " " ]; then
        if [ "$free_mem" -gt "$max_free_mem" ] 2>/dev/null; then 
            max_free_mem=$free_mem
        fi
        
        if [ "$free_mem" -lt "$min_free_mem" ] 2>/dev/null; then 
            min_free_mem=$free_mem
        fi
        
        sum_free_mem=$((sum_free_mem + free_mem))
    fi
    
    # 更新进程统计值
    if [ -n "$IDS_cpu" ] && [ "$IDS_cpu" != "" ] && [ "$IDS_cpu" != " " ]; then
        if [ "$(echo "$IDS_cpu $max_IDS_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_IDS_cpu=$IDS_cpu
        fi
        
        if [ "$(echo "$IDS_cpu $min_IDS_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_IDS_cpu=$IDS_cpu
        fi
        
        sum_IDS_cpu=$(echo "$sum_IDS_cpu $IDS_cpu" | awk '{print $1 + $2}')
    fi
    
    if [ -n "$IDS_mem_RSS" ] && [ "$IDS_mem_RSS" != "" ] && [ "$IDS_mem_RSS" != " " ]; then
        if [ "$IDS_mem_RSS" -gt "$max_IDS_mem_RSS" ] 2>/dev/null; then 
            max_IDS_mem_RSS=$IDS_mem_RSS
        fi
        
        if [ "$IDS_mem_RSS" -lt "$min_IDS_mem_RSS" ] 2>/dev/null; then 
            min_IDS_mem_RSS=$IDS_mem_RSS
        fi
        
        sum_IDS_mem_RSS=$((sum_IDS_mem_RSS + IDS_mem_RSS))
    fi
    
    clear
    printf  " %-20s\n" "${current_time}"
    printf "%-7s %-6s  %-6s  %-8s  %-8s  %-8s  %-8s  %-6s  \n" \
        "   " "usr" "sys" "idle" "used" "free" "IDS_cpu" "IDS_mem_RSS"
    printf "%-7s %-6s  %-6s  %-8s  %-8s  %-8s  %-8s  %-6s  \n" \
        "   " "${usr_cpu}%" "${sys_cpu}%" "${idle_cpu}%" "${used_mem}" "${free_mem}" "${IDS_cpu}%" "${IDS_mem_RSS}K"
    
    echo "$current_time,$PR0OCESS_pid,$PROCESS_NAME,\
    $usr_cpu,$sys_cpu,$idle_cpu,\
    $used_mem,$free_mem,\
    $IDS_cpu,$IDS_mem_RSS" \
    >> "$CURRENT_FILE"
    
    # 更新行计数器
    line_counter=$((line_counter + 1))
    
    # 检查是否需要创建新文件
    if [ "$line_counter" -ge "$MAX_LINES_PER_FILE" ]; then
        file_counter=$((file_counter + 1))
        CURRENT_FILE=$(get_filename)
        write_header "$CURRENT_FILE"
        line_counter=0
        echo "Created new file: $CURRENT_FILE (split every 40K records)"
    fi
}

echo "Start monitoring process $PROCESS_NAME, results saved to $OUTPUT_FILE"

# 捕获Ctrl+C信号以打印最终统计
trap 'echo ""; echo "Final stats:"; print_stats; exit 0' INT

# 循环采集，每秒一次
count=1
while true
do
    fun
    print_stats
    sleep 1
    count=$((count + 1))
    
    # 每100次输出一次进度信息（可选）
    if [ $((count % 100)) -eq 0 ]; then
        echo "Collected ${count} records..."
    fi
done