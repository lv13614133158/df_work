
file_path="./hping3.log"
file_test=0
main_test=0
user_test=0
data_test=0
cpu_test=0
arm_test=0
aom_test=0

TCP_SYN_SCAN=0
TCP_NULL_SCAN=0
TCP_FIN_SCAN=0
TCP_ACK_SCAN=0
TCP_FIN_RST_DOS=0
TCP_ACK_FIN_DOS=0
TCP_SYN_ACK_FLOOD=0
TCP_ACK_RST_DOS=0
TCP_ACK_PSH_FLOOD=0
TCP_SYN_FLOOD=0
TCP_XMAS_SCAN=0
UDP_SRC_PORT_ZERO=0
FRAGGLE_ATTACK=0
UDP_PORT_FLOOD=0
TCP_LAND_ATTACK=0
ICMP_LARGE_PING=0
TCP_SRC_PORT_ZERO=0
TCP_FIN_SYN_DOS=0
UDP_PORT_SCAN=0

test() { 

echo 'idps_test.sh  start' 

num=0
content=$(cat "$file_path")
while true; 
do
    content=$(cat "$file_path")
	if [ $num -eq 26 ]; then
        break
    elif [ $file_test -eq 0 ]  && echo "$content" | grep -q "00101010001"; then
        file_test=1
        echo 'File Resources_test       OK'
        num=$((num + 1))
    elif [ $main_test -eq 0 ]  && echo "$content" | grep -q "0010103000124"; then
        main_test=1
        echo 'Process Resources_test    OK'
        num=$((num + 1))
    elif [ $user_test -eq 0 ]  && echo "$content" | grep -q "0010105000124"; then
        user_test=1
        echo 'User Resources_test       OK'
        num=$((num + 1))
    elif [ $data_test -eq 0 ]  && echo "$content" | grep -q "0010106000123"; then
        data_test=1
        echo 'System Resources_test     OK'
        num=$((num + 1))
    elif [ $cpu_test -eq 0 ]  && echo "$content" | grep -q "0010106000224"; then
        cpu_test=1
        echo 'CPU Resources_test        OK'
        num=$((num + 1))
    elif [ $arm_test -eq 0 ]  && echo "$content" | grep -q "0010106000324"; then
        arm_test=1
        echo 'ARM Resources_test        OK'
        num=$((num + 1))
    elif [ $aom_test -eq 0 ]  && echo "$content" | grep -q "0010106000424"; then
        aom_test=1
        echo 'AOM Resources_test        OK'
        num=$((num + 1))
    elif [ $TCP_SYN_SCAN -eq 0 ]  && echo "$content" | grep -q "TCP_SYN_SCAN"; then
        TCP_SYN_SCAN=1
        echo 'TCP_SYN_SCAN              OK'
        num=$((num + 1))
    elif [ $TCP_NULL_SCAN -eq 0 ]  && echo "$content" | grep -q "TCP_NULL_SCAN"; then
        TCP_NULL_SCAN=1
        echo 'TCP_NULL_SCAN             OK'
        num=$((num + 1))
    elif [ $TCP_FIN_SCAN -eq 0 ]  && echo "$content" | grep -q "TCP_FIN_SCAN"; then
        TCP_FIN_SCAN=1
        echo 'TCP_FIN_SCAN              OK'
        num=$((num + 1))
    elif [ $TCP_ACK_SCAN -eq 0 ]  && echo "$content" | grep -q "TCP_ACK_SCAN"; then
        TCP_ACK_SCAN=1
        echo 'TCP_ACK_SCAN              OK'
        num=$((num + 1))
    elif [ $TCP_FIN_RST_DOS -eq 0 ]  && echo "$content" | grep -q "TCP_FIN_RST_DOS"; then
        TCP_FIN_RST_DOS=1
        echo 'TCP_FIN_RST_DOS           OK'
        num=$((num + 1))
    elif [ $TCP_ACK_FIN_DOS -eq 0 ]  && echo "$content" | grep -q "TCP_ACK_FIN_DOS"; then
        TCP_ACK_FIN_DOS=1
        echo 'TCP_ACK_FIN_DOS           OK'
        num=$((num + 1))
    elif [ $TCP_SYN_ACK_FLOOD -eq 0 ]  && echo "$content" | grep -q "TCP_SYN_ACK_FLOOD"; then
        TCP_SYN_ACK_FLOOD=1
        echo 'TCP_SYN_ACK_FLOOD         OK'
        num=$((num + 1))
    elif  [ $TCP_ACK_RST_DOS -eq 0 ]  &&echo "$content" | grep -q "TCP_ACK_RST_DOS"; then
        TCP_ACK_RST_DOS=1
        echo 'TCP_ACK_RST_DOS           OK'
        num=$((num + 1))
    elif  [ $TCP_ACK_PSH_FLOOD -eq 0 ]  &&echo "$content" | grep -q "TCP_ACK_PSH_FLOOD"; then
        TCP_ACK_PSH_FLOOD=1
        echo 'TCP_ACK_PSH_FLOOD         OK'
        num=$((num + 1))
    elif [ $TCP_SYN_FLOOD -eq 0 ]  && echo "$content" | grep -q "TCP_SYN_FLOOD"; then
        TCP_SYN_FLOOD=1
        echo 'TCP_SYN_FLOOD             OK'
        num=$((num + 1))
    elif [ $TCP_XMAS_SCAN -eq 0 ]  && echo "$content" | grep -q "TCP_XMAS_SCAN"; then
        TCP_XMAS_SCAN=1
        echo 'TCP_XMAS_SCAN             OK'
        num=$((num + 1))
    elif [ $UDP_SRC_PORT_ZERO -eq 0 ]  && echo "$content" | grep -q "UDP_SRC_PORT_ZERO"; then 
        UDP_SRC_PORT_ZERO=1
        echo 'UDP_SRC_PORT_ZERO         OK'
        num=$((num + 1))
    elif [ $FRAGGLE_ATTACK -eq 0 ]  && echo "$content" | grep -q "FRAGGLE_ATTACK"; then
        FRAGGLE_ATTACK=1
        echo 'FRAGGLE_ATTACK            OK'
        num=$((num + 1))
    elif [ $UDP_PORT_FLOOD -eq 0 ]  && echo "$content" | grep -q "UDP_PORT_FLOOD"; then
        UDP_PORT_FLOOD=1
        echo 'UDP_PORT_FLOOD            OK'
        num=$((num + 1))
    elif [ $TCP_LAND_ATTACK -eq 0 ]  && echo "$content" | grep -q "TCP_LAND_ATTACK"; then
        TCP_LAND_ATTACK=1
        echo 'TCP_LAND_ATTACK           OK'
        num=$((num + 1))
    elif [ $ICMP_LARGE_PING -eq 0 ]  && echo "$content" | grep -q "ICMP_LARGE_PING"; then
        ICMP_LARGE_PING=1
        echo 'ICMP_LARGE_PING           OK'
        num=$((num + 1))
    elif [ $TCP_SRC_PORT_ZERO -eq 0 ]  && echo "$content" | grep -q "TCP_SRC_PORT_ZERO"; then
        TCP_SRC_PORT_ZERO=1
        echo 'TCP_SRC_PORT_ZERO         OK'
        num=$((num + 1))
    elif [ $TCP_FIN_SYN_DOS -eq 0 ]  && echo "$content" | grep -q "TCP_FIN_SYN_DOS"; then
        TCP_FIN_SYN_DOS=1
        echo 'TCP_FIN_SYN_DOS           OK'
        num=$((num + 1))
    elif [ $UDP_PORT_SCAN -eq 0 ]  && echo "$content" | grep -q "UDP_PORT_SCAN"; then
        UDP_PORT_SCAN=1
        echo 'UDP_PORT_SCAN             OK'
        num=$((num + 1))

    fi
    sleep 1
done 
echo 'idps_test  over' 

}




if [ $# = 0 ]; then
    echo './idps_test.sh  ./log.log' 
else
    file_path=$1
    test
fi

