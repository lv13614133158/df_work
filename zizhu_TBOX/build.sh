#!/bin/bash
path=$( cd $( dirname " ${BASH_SOURCE[0]} " ) && pwd ) ##
echo $path

# cd /mnt/work/tool/tbox_self
# # 设定环境，
# source ag35-crosstool-env-init-tbox_self
export STAGING_DIR=/home/nvidia/df/S73/ql-ag35-1806e-gcc-8.4.0-glibc-v1-toolchain/arm-openwrt-linux-gnueabi
# 指定路径，make extsdk
#make extsdk -C /home/toolchain/WHDF/ 
#make extsdk 

#清除ouput路径
rm -rf $path/output

rm -rf $path/build
mkdir $path/build
cd $path/build

cmake ..
make 

mkdir -p $path/output
rm -rf $path/output/*
mkdir -p $path/output/lib/
mkdir $path/output/bin
mkdir $path/output/log
mkdir -p $path/output/conf/config

cp $path/build/IDPS $path/output/bin/
cp $path/script/IDPS_start.sh $path/output/bin/IDPS_start.sh
cp $path/script/IDPS_stop.sh $path/output/bin/
cp $path/lib/libwebsockets.so.19 $path/output/lib/
#cp $path/tier1_part/tbox_info/lib/* $path/output/lib/
#cp $path/tier1_part/lib/* $path/output/lib/libtbox_self
cp $path/config/base_config.json $path/output/conf/config/
cp $path/config/device_info.conf $path/output/conf/config/
cp $path/config/policy_config.json $path/output/conf/config/


echo "IDPS dir is "$path/output/bin
