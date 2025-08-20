#!/bin/bash

VERSION="1.0.1"

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
    echo "#    Welecom to use the skygo terminal test tools     #"
    echo "#                                                     #"
    echo "#######################################################"
	echo "VERSION:$VERSION"
}

file_monitor()
{
    echo "1、创建文件"
    echo "2、创建目录"
    echo "3、查看文件"
    echo "4、修改文件"
    echo "5、删除文件/目录"
    echo "6、日志安全存储"
	echo "7、权限查看"
    read -p "请输入选项:" fileType

    read -p "请输入路径: " path

    case $fileType in
        1)
            if [ ! -f $path ];then
                touch $path;ret=$?
                if [ $ret != 0 ]; then
                    echo "创建失败"
                else
                    file $path  
                fi
            else
                echo "文件已存在" 
            fi
            ;;
        2)
            if [ ! -d $path ];then
                mkdir -p $path;ret=$?
                if [ $ret != 0 ]; then
                    echo "创建失败"
                fi
            else
                echo "目录已存在"
            fi
            ;;
        3)
            if [ -f $path ];then
                cat $path
            else
                echo "文件不存在"
            fi
            ;;
        4)
            if [ -f $path ];then
                echo "test" > $path
            else
                echo "文件不存在"
            fi
            ;;
        5)
            if [ -d "$path" ]; then
                #echo "路径是一个文件夹"
                cp -r $path $path"bck"
                echo "已备份目录，新路径为：" $path"bck，删除目录："$path
                rm -rf $path

            elif [ -f $path ];then
                #echo "路径是一个文件"
                cp -r $path $path".bck"
                echo "已备份文件，文件名为：" $path".bck，删除文件："$path
                rm -r $path

            else
                echo "无效路径"
            fi

            ;;
        6)
            if [ -f $path ];then
                file $path
            else
                echo "文件不存在"
            fi
            ;;
        7)
			stat $path
            ;;
		*)
			echo "无效的选项"
			;;
	esac
}

process_monitor()
{
    echo "1、执行进程"
    echo "2、关闭进程"
	echo "3、所有进程信息"
	echo "4、查看用户的进程信息"
	echo "5、查看指定进程信息"
	echo "6、连续监控进程状态(每秒刷新)"
    read -p "请输入选项:" proType
    

    if [ $proType -eq 1 ]; then
        read -p "请输入程序路径:" proPath
        read -p "请输入程序名称:" proFile
        cd $proPath
        ./$proFile&
    elif [ $proType -eq 2 ]; then
        read -p "请输入进程名称:" proFile
        pkill -f $proFile
    elif [ $proType -eq 3 ]; then
		ps aux
    elif [ $proType -eq 4 ]; then
		read -p "请输入用户名:" usrName
		ps -u $usrName
    elif [ $proType -eq 5 ]; then
		read -p "请输入进程名称:" proName
		ps aux | grep $proName
    elif [ $proType -eq 6 ]; then
        watch -n 1 'ps -e -o pid,ppid,cmd,%cpu,%mem'
    else
        echo "无效的输入"
    fi
}

cpu_test()
{
	cpunum=$(cat /proc/cpuinfo | grep "processor" | wc -l)
	echo "CPU数量："$cpunum

	read -p "请输入一个数字（0：停止；1-"$cpunum"：占用CPU数量）：" num
	 
	if [ $num -le $cpunum ] && [ $num -gt 0 ]; then
	#   echo $num
		pkill -9 dd
		for i in $(seq 1 $num); do
			dd if=/dev/zero of=/tmp/cpuTest &
		done
	elif [ $num -eq 0 ]; then
		pkill -9 dd
	else
		echo "无效的输入"
	fi
}

ram_test()
{
	read -p "请输入内存大小（0：释放内存；其他整数：占用内存大小(如：1G或1024M)）：" num

	if [ $num != "0" ]; then
		if [ -d  /tmp/mem ];then
			echo "/tmp/mem already exists"
		else
			mkdir /tmp/mem
		fi
		mount -t tmpfs -o size=$num tmpfs /tmp/mem   
		dd if=/dev/zero of=/tmp/mem/memTest
		
	elif [ $num -eq "0" ]; then
		rm /tmp/mem/memTest;ret=$?
		if [ $ret != 0 ]; then
			echo "remove mem data failed"
		fi
		
		umount /tmp/mem;ret=$?
		if [ $ret != 0 ]; then
			echo "umount mem filedir failed"
		fi
		
		rm -rf /tmp/mem;ret=$?
		if [ $ret != 0 ]; then
			echo "remove mem filedir failed"
		fi

	else
		echo "无效的输入"
	fi
}

rom_test()
{
	read -p "请输入磁盘大小（0：释放磁盘；其他整数：占用磁盘大小(单位为MB)）：" num

	if [ $num != "0" ]; then
		dd if=/dev/zero of=outputfile bs=1M count=$num
	elif [ $num -eq "0" ]; then
		rm outputfile
	else
		echo "无效的输入"
	fi
}

randomize_va_space()
{
	echo "1、查看地址空间随机化配置"
	echo "2、修改地址空间随机化配置"
	read -p "请输入选项:" type

	if [ $type -eq 1 ]; then
		ret=$(cat /proc/sys/kernel/randomize_va_space)
		if [ $ret -eq 0 ]; then
			echo "已关闭地址空间随机化"

		elif [ $ret -eq 1 ]; then
			echo "部分随机化，即共享库、栈、mmap()以及VSDO被随机化"

		elif [ $ret -eq 2 ]; then
			echo "完全的随机化，通过brk()分配的内存空间也被随机化"

		else
			echo "读取出错"
		fi

	elif [ $type -eq 2 ]; then
		echo "1、关闭随机化"
		echo "2、部分随机化"
		echo "3、完全随机化"
		read -p "请输入随机化等级:" level

		if [ $level -eq 1 ]; then
			bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"

		elif [ $level -eq 2 ]; then
			bash -c "echo 1 > /proc/sys/kernel/randomize_va_space"

		elif [ $level -eq 3 ]; then
			bash -c "echo 2 > /proc/sys/kernel/randomize_va_space"

		else
			echo "输入错误"
		fi
		

	else
		echo "输入错误"

	fi

}

resourse_monitor()
{
	echo "1. CPU占用测试"
	echo "2. RAM占用测试"
	echo "3. ROM占用测试"
	read -p "请输入选项： " resourse_monitor_choice

	case $resourse_monitor_choice in
        1)
            cpu_test
            ;; 
		2)
			ram_test
			;; 
		3)
			rom_test
			;; 
		4)
			echo 'RAM（百分比）升到 某百分比 以上，持续多少分钟'
			;; 
		5)
			echo '返回上一级'
			;; 
		*)
			echo "无效的选项"
			;;
    esac
}

system_monitor()
{
	#用户权限

	echo "1、有效用户检测"
	echo "2、USB设备接入检测"
	echo "3、WIFI连接检测"
	echo "4、蓝牙设备接入检测"
	echo "5、热点连接检测"
	echo "6、内存地址随机化检测"
	read -p "请输入选项:" type

	case $type in
		1)
			echo "用户名 用户ID 用户组 主目录"
			awk -F: '(($3>=1000 && $3<60000)||$3==0){print $1,$3,$4,$6}' /etc/passwd

			echo "当前用户:"
			who am i
			
			id
			;;
		2)
			lsusb
			;;
		3)
			ifconfig
			;;
		4)
			hciconfig
			;;
		5)
			iw dev wlan0 station dump
			;;
		6)
			randomize_va_space
			;;
		*)
			echo "无效的输入"
	esac
}

# 第一级选择
logo
echo "1. 文件操作"
echo "2. 进程操作"
echo "3. 资源操作"
echo "4. 系统检测"
read -p "请输入选项： " choice1

case $choice1 in
	1)
		# 执行操作1的命令
		echo "文件操作"
		file_monitor
		;;
	2)
		# 执行操作2的命令
		echo "进程操作"
		process_monitor
		;;
	3)
		# 执行操作3的命令
		echo "资源操作"
		resourse_monitor
		;;
	4)
		# 执行操作4的命令
		echo "系统检测"
		system_monitor
		;;
	5)
		# 返回上一级
		;;
    9)
        # 退出脚本
        echo "退出"
        exit 0
        ;;
	*)
		echo "无效的选项"
		;;
esac