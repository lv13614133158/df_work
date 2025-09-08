#!/bin/bash


# ====================
# IDPS 测试主逻辑
# ====================
if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_log_file>"
    exit 1
fi

file_path="$1"
output_file="idps_test_ivi.log"


# 清空输出文件
echo -n "" > "$output_file"




test_event() {
    type=$1
    name=$2
    echo "正在测试 $name $type"
while true; do
    # 修复null byte警告问题，使用tr命令过滤掉null bytes
    content=$(tr -d '\0' < "$file_path")
    matched=$(echo "$content"| grep "$type" | grep -i "$name")
    if [ -n "$matched" ]; then
        matched=$(echo "$matched" | tail -n 1)
        echo "">> "$output_file"
        echo "$name $type">> "$output_file"
        echo "">> "$output_file"
        echo "$matched" >> "$output_file"
        echo "测试完成 已保存"
        break
    fi
done
    
}
Network="0020109000111"
# 测试事件列表

echo "文件监控 "
test_event "001010100011" "1.txt"
echo "tcp监控 "
test_event "0010102000211" "body"
echo "udp监控 "
test_event "0010102000311" "body"
echo "端口监控 "
test_event "0010102000611" "body"
echo "普通应用安装监控 "
test_event "001010400011" "191"
echo "非法应用安装监控 "
test_event "001010400011" "480"
echo "root应用安装监控 "
test_event "001010400011" "Kingo ROOT"
echo "卸载应用监控 "
test_event "001010400011" ':-'
echo "应用权限监控 "
test_event "0010104000211" "FINE_LOCATION"
echo "剪贴板权限监控 "
test_event "0010104000211" "READ_CLIPBOARD"
echo "gps权限监控 "
test_event "0010104000211" "GPS"
echo "音视频权限监控 "
test_event "0010104000211" "RECORD_AUDIO"
echo "资源占用监控 "
test_event "0010106000111" "data"
echo "cpu监控 "
test_event "0020106000211" "cpu_num"
echo "arm监控 "
test_event "0020106000311" "ram_rate"
echo "aom监控 "
test_event "0020106000411" "rom_rate"
echo "病毒应用监控 "
test_event "002011000011" "body"
echo "流量采集监控 "
test_event "001010800011" "iface_data"
echo "持久化root监控 "
test_event "001010100011" "su"
echo "adb调试监控 "
test_event "002010700011" "body"
echo "网络攻击测试 "
test_event "$Network" "TCP_SYN_SCAN"
test_event "$Network" "TCP_NULL_SCAN"
test_event "$Network" "TCP_FIN_SCAN"
test_event "$Network" "TCP_ACK_SCAN"
test_event "$Network" "TCP_FIN_RST_DOS"
test_event "$Network" "TCP_ACK_FIN_DOS"
test_event "$Network" "TCP_SYN_ACK_FLOOD"
test_event "$Network" "TCP_ACK_RST_DOS"
test_event "$Network" "TCP_ACK_PSH_FLOOD"
test_event "$Network" "TCP_SYN_FLOOD"
test_event "$Network" "TCP_XMAS_SCAN"
test_event "$Network" "UDP_SRC_PORT_ZERO"
test_event "$Network" "FRAGGLE_ATTACK"
test_event "$Network" "UDP_PORT_FLOOD"
test_event "$Network" "TCP_LAND_ATTACK"
test_event "$Network" "ICMP_LARGE_PING"
test_event "$Network" "TCP_SRC_PORT_ZERO"
test_event "$Network" "TCP_FIN_SYN_DOS"
test_event "$Network" "UDP_PORT_SCAN"

# 格式化临时文件中的内容

echo " 保存文件： $output_file"