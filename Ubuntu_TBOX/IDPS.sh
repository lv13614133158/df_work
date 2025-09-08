#!/bin/bash

# 获取脚本所在目录作为当前路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ./output/bin/
sudo -E LD_LIBRARY_PATH="$SCRIPT_DIR/lib:$LD_LIBRARY_PATH" ./IDPS

sudo -E LD_LIBRARY_PATH=/home/nvidia/df/df_work/Ubuntu_TBOX/lib:$LD_LIBRARY_PATH ./IDPS