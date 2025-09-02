#!/bin/bash    
# IDPS start up script
script_name=$(basename $0)
# echo $script_name

#添加本地执行路径
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib
while true; 
do
	num=$(ps aux | grep IDPS | grep -v grep | grep -v $script_name |wc -l)
	if [ $num -eq 0 ];then
		cd /oemapp/bin/
		./IDPS &
		echo "IDPS START"
		#启动后沉睡10s
		sleep 5
	fi
	#echo "IDPS AREADY RUNNINGi" ${server}
	sleep 5
done
