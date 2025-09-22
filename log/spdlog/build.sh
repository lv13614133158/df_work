#!/bin/bash
path=$( cd $( dirname " ${BASH_SOURCE[0]} " ) && pwd ) ##
echo $path

cd /mnt/work/tool
# 设定环境，
source ag35-crosstool-env-init-v2
#cd /home/toolchain/WHDF5G-MKT
# 设定环境，
#source /home/toolchain/WHDF/config.rc
#source config.rc
# 指定路径，make extsdk
#make extsdk -C /home/toolchain/WHDF/ 
#make extsdk 


cd $path/build
rm -r *

cmake ..

make

cp libspdlog.a ../lib
