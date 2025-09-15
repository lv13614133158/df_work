#!/bin/bash

sleep_time=20
file_path="./1.txt"
men_len=500  #mb

function1() {
    echo "文件事件 被触发: $(date)"
    touch $file_path
    sleep 1
    touch $file_path
    sleep 1
    rm $file_path
}

function2() {
    echo "用户事件 被触发: $(date)"
   
    
}

function3() {
    echo "内存事件 被触发: $(date)"
    
   
    # 一次性生成所需大小的字符串
    local temp_file="./large_file.dat"
    dd if=/dev/zero of="$temp_file" bs=1048576 count=$men_len 2>/dev/null
    LARGE_STRING=$(base64 "$temp_file")
    rm -f "$temp_file"
    while true; do
        # 简单引用变量防止被优化
        local str=${#LARGE_STRING}
        sleep 100
    done
}

function4() {
    echo "cpu占用事件 被触发: $(date)"
    while true; do
    :
    done
    
}
function_array=("function1" "function2" "function3" "function4")

# 指定要执行的函数索引
selected_index=0  # 添加这行来指定索引

# 主循环
while true; do
    if [ $selected_index -ge ${#function_array[@]} ]; then
        selected_index=0
    fi
    
    selected_function=${function_array[$selected_index]}
    
    # 确保杀死所有相关的后台进程
    jobs -p | xargs -r kill 2>/dev/null
    sleep 1
    
    # 启动新函数
    $selected_function &  
    FUNCTION_PID=$!
 
    sleep $sleep_time
    
    if ps -p $FUNCTION_PID > /dev/null; then
        echo "终止函数，PID: $FUNCTION_PID"
        kill $FUNCTION_PID
        wait $FUNCTION_PID 2>/dev/null 
    fi
    
    selected_index=$((selected_index+1))
    sleep 2  # 给进程完全终止的时间
done