#!/bin/bash
path=$( cd $( dirname " ${BASH_SOURCE[0]} " ) && pwd ) ##
echo $path


# cd /home/toolchain/DFTC-MKT
# # 设定环境，
# source config.rc
# 指定路径，make extsdk
#make extsdk -C /home/toolchain/WHDF/ 
#make extsdk 

mkdir -p $path/build
cd $path/build
rm -r *
cmake ..
make 

mkdir -p $path/output
rm -rf $path/output/*
mkdir $path/output/lib
mkdir $path/output/bin
mkdir $path/output/conf
mkdir $path/output/log
mkdir $path/output/conf/config

cp $path/build/IDPS $path/output/bin/
cp $path/script/IDPS_start.sh $path/output/bin/IDPS_start.sh
cp $path/script/IDPS_stop.sh $path/output/bin/
cp $path/lib/libwebsockets.so.19 $path/output/lib/
cp $path/tier1_part/swg_info/lib/libidpsProxy.so $path/output/lib/
cp $path/config/base_config.json $path/output/conf/config/
cp $path/config/policy_config.json $path/output/conf/config/
cp $path/config/device_info.conf $path/output/conf/config/
