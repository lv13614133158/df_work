#!/bin/bash
# 获取脚本路径
path=$( cd $( dirname " ${BASH_SOURCE[0]} " ) && pwd ) ##
echo $path

# 进入build目录
cd $path/build

# 使用书梅派库
cp ../lib/rap_lib/* ../lib

#清空
rm -r ./*

#source /home/toolchain/DF/ql-ol-crosstool-env-init

# 编译
cmake ..

make 

cp IDPS ../lib
cp IDPS ../output


