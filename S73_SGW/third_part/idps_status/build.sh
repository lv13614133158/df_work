#!/bin/bash
path=$( cd $( dirname " ${BASH_SOURCE[0]} " ) && pwd ) ##
echo $path


mkdir build
cd $path/build
rm -r *

cmake ..

make

cp libidps_status.so ../lib
