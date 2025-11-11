#!/bin/bash
# 修改后的 writer.sh

# 启动10个后台进程，每个进程传递不同的参数
for i in {1..10}; do
    sudo LD_LIBRARY_PATH=./ ./writer $i &
    echo "Started writer process $i with PID $!"
done

# 等待所有后台进程完成
wait
