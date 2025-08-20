#!/bin/bash    
# IDPS start up script 
#添加本地执行路径
#export LD_LIBRARY_PATH=/opt/app/pki/lib:/opt/app/lib/:$LD_LIBRARY_PATH
while true; 
do
	num=$(ps aux | grep ./IDPS | grep -v 'IDPS_start' |  grep -v grep|wc -l)
	if [ $num -eq 0 ];then
		cd /opt/app/idps/bin
		./IDPS &
		echo "IDPS START"
		#启动后沉睡10s
		sleep 5
	fi
	#echo "IDPS AREADY RUNNINGi" ${server}
	sleep 5
done
