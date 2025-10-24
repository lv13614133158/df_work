#!/usr/bin/env bash

# 检查是否提供了进程名参数
if [ $# -eq 1 ]; then
    PROCESS_NAME=$1
else
    PROCESS_NAME="IDPS"
fi

OUTPUT_FILE="${PROCESS_NAME}_monito.csv"

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

max_idle_cpu=0
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
count_IDS_stats=0
last_avg_IDS_cpu="0.00"
last_avg_IDS_mem_RSS="0"

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
    # 使用一次top命令获取CPU信息，避免重复调用
    top_output=$(top -bn1)
    cpu_line=$(echo "$top_output" | grep -E "Cpu\(s\):|CPU:" | head -1)
    
    # 判断top输出格式并相应解析
    if echo "$cpu_line" | grep -q "Cpu(s):"; then
        # 标准Linux格式: Cpu(s): 10.0% us,  5.0% sy,  0.0% ni, 85.0% id,  0.0% wa,  0.0% hi,  0.0% si,  0.0% st
        usr_cpu=$(echo "$cpu_line" | awk -F',' '{print $1}' | awk '{print $2}' | sed 's/%.*//' | tr -d ' \t')
        sys_cpu=$(echo "$cpu_line" | awk -F',' '{print $2}' | awk '{print $1}' | sed 's/%.*//' | tr -d ' \t')
        idle_cpu=$(echo "$cpu_line" | awk -F',' '{print $4}' | awk '{print $1}' | sed 's/%.*//' | tr -d ' \t')
    else
        # OpenWrt格式: CPU:   0% usr  18% sys   0% nic  81% idle   0% io   0% irq   0% sirq
        usr_cpu=$(echo "$cpu_line" | grep -o '[0-9.]*%' | head -1 | sed 's/%//')
        sys_cpu=$(echo "$cpu_line" | grep -o '[0-9.]*%' | head -2 | tail -1 | sed 's/%//')
        idle_cpu=$(echo "$cpu_line" | grep -o '[0-9.]*%' | head -4 | tail -1 | sed 's/%//')
    fi
    
    # 验证并确保获取到的是数字
    echo "$usr_cpu" | grep -qE '^[0-9]+\.?[0-9]*$' || usr_cpu="0"
    echo "$sys_cpu" | grep -qE '^[0-9]+\.?[0-9]*$' || sys_cpu="0"
    echo "$idle_cpu" | grep -qE '^[0-9]+\.?[0-9]*$' || idle_cpu="0"
    
    # 获取内存使用情况
    mem_line=$(free -m | grep "Mem:")
    used_mem=$(echo "$mem_line" | awk '{print $3}' | tr -d ' \t')
    free_mem=$(echo "$mem_line" | awk '{print $4}' | tr -d ' \t')
    
    # 验证内存值
    echo "$used_mem" | grep -qE '^[0-9]+$' || used_mem="0"
    echo "$free_mem" | grep -qE '^[0-9]+$' || free_mem="0"
}

# 获取进程CPU和内存使用情况
get_process_stats() {
    # 重用之前获取的top输出，避免重复调用top命令
    PROCESS_pid=$(ps -A | grep "$PROCESS_NAME" | grep -v grep|grep -v sh | grep -v bash | awk '{print $1}' | head -1)
        
    if [ -n "$PROCESS_pid" ]; then
        # 判断top输出格式并相应解析
        if echo "$cpu_line" | grep -q "Cpu(s):"; then
            # 标准Linux格式
            IDS_cpu=$(echo "$top_output" | grep " $PROCESS_pid " | awk '{print $9}' | grep -E '^[0-9]+\.?[0-9]*$' | tr -d ' \t' || echo "0")
        else 
            # OpenWrt格式
            IDS_cpu=$(echo "$top_output" | grep " $PROCESS_pid " | awk '{print $7}' | sed 's/%//' | grep -E '^[0-9]+\.?[0-9]*$' | tr -d ' \t' || echo "0")
        fi

        IDS_mem_RSS=$(grep VmRSS /proc/$PROCESS_pid/status 2>/dev/null | awk '{print $2}' | grep -E '^[0-9]+$' | tr -d ' \t' || echo "0")
        
        # 确保进程CPU使用率在有效范围内
        if echo "$IDS_cpu" | grep -qE '^[0-9]+\.?[0-9]*$'; then
            # 限制进程CPU使用率在0-100之间
            if [ "$(echo "$IDS_cpu > 100" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
                IDS_cpu="100"
            fi
            if [ "$(echo "$IDS_cpu < 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
                IDS_cpu="0"
            fi
        else
            IDS_cpu="0"
        fi
    else
        PROCESS_pid=""
        IDS_cpu=""
        IDS_mem_RSS=""
    fi
}

# 打印统计数据
print_stats() {
    # 确保所有变量都是有效的数字
    count_stats=$(echo "$count_stats" | grep -E '^[0-9]+$' || echo "0")
    count_IDS_stats=$(echo "$count_IDS_stats" | grep -E '^[0-9]+$' || echo "0")
    
    sum_usr_cpu=$(echo "$sum_usr_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_sys_cpu=$(echo "$sum_sys_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_idle_cpu=$(echo "$sum_idle_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_used_mem=$(echo "$sum_used_mem" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_free_mem=$(echo "$sum_free_mem" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_IDS_cpu=$(echo "$sum_IDS_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    sum_IDS_mem_RSS=$(echo "$sum_IDS_mem_RSS" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    
    if [ "$count_stats" -gt 0 ]; then
        # 使用更安全的计算方式
        avg_usr_cpu=$(awk "BEGIN {printf \"%.2f\", $sum_usr_cpu/$count_stats}" 2>/dev/null || echo "0.00")
        avg_sys_cpu=$(awk "BEGIN {printf \"%.2f\", $sum_sys_cpu/$count_stats}" 2>/dev/null || echo "0.00")
        avg_idle_cpu=$(awk "BEGIN {printf \"%.2f\", $sum_idle_cpu/$count_stats}" 2>/dev/null || echo "0.00")
        avg_used_mem=$(awk "BEGIN {printf \"%.2f\", $sum_used_mem/$count_stats}" 2>/dev/null || echo "0.00")
        avg_free_mem=$(awk "BEGIN {printf \"%.2f\", $sum_free_mem/$count_stats}" 2>/dev/null || echo "0.00")
    else
        # 当还没有统计数据时，使用当前值作为平均值
        avg_usr_cpu=${usr_cpu:-"0.00"}
        avg_sys_cpu=${sys_cpu:-"0.00"}
        avg_idle_cpu=${idle_cpu:-"0.00"}
        avg_used_mem=${used_mem:-"0.00"}
        avg_free_mem=${free_mem:-"0.00"}
    fi
    
    # 计算进程CPU和内存的平均值
    if [ "$count_IDS_stats" -gt 0 ]; then
        avg_IDS_cpu=$(awk "BEGIN {printf \"%.2f\", $sum_IDS_cpu/$count_IDS_stats}" 2>/dev/null || echo "0.00")
        avg_IDS_mem_RSS=$(awk "BEGIN {printf \"%.0f\", $sum_IDS_mem_RSS/$count_IDS_stats}" 2>/dev/null || echo "0")
        
        # 确保avg_IDS_cpu在合理范围内(0-100)
        avg_IDS_cpu=$(echo "$avg_IDS_cpu" | awk '{print ($1 > 100) ? 100 : ($1 < 0 ? 0 : $1)}')
        # 格式化为两位小数
        avg_IDS_cpu=$(printf "%.2f" "$avg_IDS_cpu")
        
        last_avg_IDS_cpu="$avg_IDS_cpu"
        last_avg_IDS_mem_RSS="$avg_IDS_mem_RSS"
    else
        avg_IDS_cpu="$last_avg_IDS_cpu"
        avg_IDS_mem_RSS="$last_avg_IDS_mem_RSS"
    fi

    # 处理最大值和最小值显示
    if [ -n "$PROCESS_pid" ]; then
        display_max_IDS_cpu="${max_IDS_cpu}%"
        display_min_IDS_cpu="${min_IDS_cpu}%"
        display_max_IDS_mem_RSS="${max_IDS_mem_RSS}K"
        display_min_IDS_mem_RSS="${min_IDS_mem_RSS}K"
    else
        display_max_IDS_cpu=""
        display_min_IDS_cpu=""
        display_max_IDS_mem_RSS=""
        display_min_IDS_mem_RSS=""
    fi
    
    # 清屏并显示统计信息
    clear
    echo "Monitoring process: $PROCESS_NAME (PID: ${PROCESS_pid:-N/A})"
    echo "Data saved to: $OUTPUT_FILE"
    echo "Timestamp: $(date "+%Y-%m-%d %H:%M:%S")"
    echo "------------------------------------------------------------------------"
    printf "%-6s  %-10s  %-10s  %-10s  %-12s  %-12s  %-12s  %-12s\n" \
        "Type" "usr_cpu" "sys_cpu" "idle_cpu" "used_mem" "free_mem" "IDS_cpu" "IDS_mem_RSS"
    printf "%-6s  %-10s  %-10s  %-10s  %-12s  %-12s  %-12s  %-12s\n" \
        "avg" "${avg_usr_cpu}%" "${avg_sys_cpu}%" "${avg_idle_cpu}%" \
        "${avg_used_mem}" "${avg_free_mem}" "${avg_IDS_cpu}%" "${avg_IDS_mem_RSS}K"
    printf "%-6s  %-10s  %-10s  %-10s  %-12s  %-12s  %-12s  %-12s\n" \
        "max" "${max_usr_cpu}%" "${max_sys_cpu}%" "${max_idle_cpu}%" \
        "${max_used_mem}" "${max_free_mem}" "${display_max_IDS_cpu}" "${display_max_IDS_mem_RSS}"
    printf "%-6s  %-10s  %-10s  %-10s  %-12s  %-12s  %-12s  %-12s\n" \
        "min" "${min_usr_cpu}%" "${min_sys_cpu}%" "${min_idle_cpu}%" \
        "${min_used_mem}" "${min_free_mem}" "${display_min_IDS_cpu}" "${display_min_IDS_mem_RSS}"
    echo "------------------------------------------------------------------------"
    echo "Records collected: $count_stats"
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
    
    # 更新CPU统计值
    if [ -n "$usr_cpu" ] && echo "$usr_cpu" | grep -qE '^[0-9]+\.?[0-9]*$'; then
        usr_cpu_float=$(printf "%.2f" "$usr_cpu")
        # 使用awk进行数值比较，避免(( ))中的语法错误
        if [ "$(echo "$usr_cpu_float $max_usr_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_usr_cpu=$usr_cpu_float
        fi
        
        if [ "$(echo "$usr_cpu_float $min_usr_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_usr_cpu=$usr_cpu_float
        fi
        
        sum_usr_cpu=$(echo "$sum_usr_cpu $usr_cpu_float" | awk '{print $1 + $2}')
        # 确保sum_usr_cpu是有效数字
        sum_usr_cpu=$(echo "$sum_usr_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    fi
    
    if [ -n "$sys_cpu" ] && echo "$sys_cpu" | grep -qE '^[0-9]+\.?[0-9]*$'; then
        sys_cpu_float=$(printf "%.2f" "$sys_cpu")
        if [ "$(echo "$sys_cpu_float $max_sys_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_sys_cpu=$sys_cpu_float
        fi
        
        if [ "$(echo "$sys_cpu_float $min_sys_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_sys_cpu=$sys_cpu_float
        fi
        
        sum_sys_cpu=$(echo "$sum_sys_cpu $sys_cpu_float" | awk '{print $1 + $2}')
        # 确保sum_sys_cpu是有效数字
        sum_sys_cpu=$(echo "$sum_sys_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    fi
    
    if [ -n "$idle_cpu" ] && echo "$idle_cpu" | grep -qE '^[0-9]+\.?[0-9]*$'; then
        idle_cpu_float=$(printf "%.2f" "$idle_cpu")
        if [ "$(echo "$idle_cpu_float $max_idle_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then 
            max_idle_cpu=$idle_cpu_float
        fi
        
        if [ "$(echo "$idle_cpu_float $min_idle_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then 
            min_idle_cpu=$idle_cpu_float
        fi
        
        sum_idle_cpu=$(echo "$sum_idle_cpu $idle_cpu_float" | awk '{print $1 + $2}')
        # 确保sum_idle_cpu是有效数字
        sum_idle_cpu=$(echo "$sum_idle_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    fi
    
    # 更新内存统计值
    if [ -n "$used_mem" ] && echo "$used_mem" | grep -qE '^[0-9]+$'; then
        if [ "$used_mem" -gt "$max_used_mem" ] 2>/dev/null; then 
            max_used_mem=$used_mem
        fi
        
        if [ "$used_mem" -lt "$min_used_mem" ] 2>/dev/null; then 
            min_used_mem=$used_mem
        fi
        
        sum_used_mem=$((sum_used_mem + used_mem))
        # 确保sum_used_mem是有效数字
        sum_used_mem=$(echo "$sum_used_mem" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    fi
    
    if [ -n "$free_mem" ] && echo "$free_mem" | grep -qE '^[0-9]+$'; then
        if [ "$free_mem" -gt "$max_free_mem" ] 2>/dev/null; then 
            max_free_mem=$free_mem
        fi
        
        if [ "$free_mem" -lt "$min_free_mem" ] 2>/dev/null; then 
            min_free_mem=$free_mem
        fi
        
        sum_free_mem=$((sum_free_mem + free_mem))
        # 确保sum_free_mem是有效数字
        sum_free_mem=$(echo "$sum_free_mem" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
    fi
    
    # 只有当进程存在时才更新进程统计值
    if [ -n "$PROCESS_pid" ] && [ "$PROCESS_pid" != "" ]; then
        # 验证当前进程CPU使用率是否为有效数字
        if echo "$IDS_cpu" | grep -qE '^[0-9]+\.?[0-9]*$'; then
            # 确保进程CPU使用率在0-100之间
            if [ "$(echo "$IDS_cpu > 100" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
                IDS_cpu="100"
            fi
            if [ "$(echo "$IDS_cpu < 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
                IDS_cpu="0"
            fi

            # 只有当是有效数字时才更新统计值
            count_IDS_stats=$((count_IDS_stats + 1))
            
            if [ "$(echo "$IDS_cpu $max_IDS_cpu" | awk '{print ($1 > $2)}')" = "1" ]; then
                max_IDS_cpu=$IDS_cpu
            fi
            
            if [ "$(echo "$IDS_cpu $min_IDS_cpu" | awk '{print ($1 < $2)}')" = "1" ]; then
                min_IDS_cpu=$IDS_cpu
            fi
            
            sum_IDS_cpu=$(echo "$sum_IDS_cpu $IDS_cpu" | awk '{print $1 + $2}')
            # 确保sum_IDS_cpu是有效数字
            sum_IDS_cpu=$(echo "$sum_IDS_cpu" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
        fi
        
        # 更新进程内存统计值（同样需要验证）
        if echo "$IDS_mem_RSS" | grep -qE '^[0-9]+$'; then
            if [ "$IDS_mem_RSS" -gt "$max_IDS_mem_RSS" ] 2>/dev/null; then 
                max_IDS_mem_RSS=$IDS_mem_RSS
            fi
            
            if [ "$IDS_mem_RSS" -lt "$min_IDS_mem_RSS" ] 2>/dev/null; then 
                min_IDS_mem_RSS=$IDS_mem_RSS
            fi
            
            sum_IDS_mem_RSS=$((sum_IDS_mem_RSS + IDS_mem_RSS))
            # 确保sum_IDS_mem_RSS是有效数字
            sum_IDS_mem_RSS=$(echo "$sum_IDS_mem_RSS" | grep -E '^[0-9]+\.?[0-9]*$' || echo "0")
        fi
    fi

    # 写入文件时，如果进程不存在则写入空值
    file_IDS_cpu=${IDS_cpu:-""}
    file_IDS_mem_RSS=${IDS_mem_RSS:-""}
    
    echo "$current_time,$PROCESS_pid,$PROCESS_NAME,\
    $usr_cpu,$sys_cpu,$idle_cpu,\
    $used_mem,$free_mem,\
    $file_IDS_cpu,$file_IDS_mem_RSS" \
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
    if [ $count -eq 1 ] || [ $((count % 5)) -eq 0 ]; then
        print_stats
    fi
    sleep 1
    count=$((count + 1))
    
    # 每100次输出一次进度信息（可选）
    if [ $((count % 100)) -eq 0 ]; then
        echo "Collected ${count} records..."
    fi
done