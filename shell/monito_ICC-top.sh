#!/usr/bin/env bash
# 一汽tbox定制获取使用详情20220908

# 获取cpu总核数
#cpu_num=`grep -c "model name" /proc/cpuinfo`
#echo "cpu总核数："$cpu_num
ptah_file=./monitor_info.csv
cpu_total=""
mem_total=""
cpu_Tsp=""
mem_Tsp=""

fun() {
    # 获取top输出
    TOP_OUTPUT=`top -b -n 1`
    # pid获取
    Tsp_pid=$(echo "$TOP_OUTPUT"  | grep -v 'grep' | grep  'IDPS' | grep -v 'sh'| awk '{print $1}')

    if [ -n "$Tsp_pid" ]; then
        if [ -f "/proc/$Tsp_pid/status" ]; then
            mem_Tsp=`cat /proc/$Tsp_pid/status | grep VmRSS | awk '{print $2}'`
        else
            mem_Tsp="0"
        fi
    else
        cpu_Tsp="0"
        mem_Tsp="0"
    fi
    # 判断是否为新风格输出（包含 "MiB Mem"）
    if echo "$TOP_OUTPUT" | grep -q "MiB Mem"; then
        # 新风格（Solaris-style）
        cpu_total=`echo "$TOP_OUTPUT" | grep 'Cpu(s)' | awk '{print $2, "," $4, "," $8}' | tr -d '%'`
        mem_total=`echo "$TOP_OUTPUT" | grep 'MiB Mem' | awk '{print $4, "," $6, "," $8}'`
            # 获取Tsp CPU百分比
        cpu_Tsp=$(echo "$TOP_OUTPUT"  | grep -v 'grep' | grep 'IDPS' | grep -v 'sh'| awk '{print $9}')
        mem_Tsp=$((mem_Tsp / 1024)) 
    else
        # 旧风格（BSD-style）
        cpu_total=`echo "$TOP_OUTPUT" | sed -n '2p' | awk '{print $2, "," $4, "," $8}' | tr -d '%'`
        mem_total=`echo "$TOP_OUTPUT" | sed -n '1p' | awk '{print $2, "," $4, "," $8}' | tr -d 'K'`
            # 获取Tsp CPU百分比
        cpu_Tsp=$(echo "$TOP_OUTPUT"  | grep -v 'grep' | grep 'IDPS' | grep -v 'sh'| awk '{print $7}')
    fi





    # 写入csv
    echo `date +"%Y/%m/%d %H:%M:%S, $cpu_total, $mem_total, $cpu_Tsp, $mem_Tsp"` >> $ptah_file
    echo "pid:$Tsp_pid  cpu:$cpu_total, mem:$mem_total, IDPS:$cpu_Tsp, $mem_Tsp"
}



# 标题	
if [ ! -f "$ptah_file" ]; then
    echo "time, usr_cpu, sys_cpu, idle_cpu, used_mem, free_mem, buff_mem, idps_cpu, idps_mem" >> $ptah_file
fi

# 获取总物理内存占用大小 ：分别为使用物理内存大小和剩余物理内存大小
mem_line=$(top -b -n 1 | grep "Mem:" | head -n 1)
if echo "$mem_line" | grep -q "used"; then
    # BSD 风格: Mem: 186996K used, 11888K free, 2584K shrd, 29336K buff, 91124K cached
    mem_total=$(echo "$mem_line" | awk '{print $3, "," $5, "," $7}' | tr -d 'K ')
else
    # Solaris 风格: MiB Mem :   3868.6 total,   1010.4 free,   1940.4 used
    mem_total=$(echo "$mem_line" | awk '{print $8, "," $6, "," $4}' | sed 's/\.[0-9]//g')
fi

# 进入top命令的环境变量
source /etc/profile 

# 循环采集 10 分钟，每秒一次（共 600 次）
i=1
while [ $i -le 600 ]
do
    fun
    sleep 1
    i=$((i + 1))
done



 
 

