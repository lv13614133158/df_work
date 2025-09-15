#!/usr/bin/env bash

cleanup() {
    
    if [ -f ./1.txt ]; then
        rm -f ./1.txt
        echo ""
    fi
    echo "cleanup"
    exit 0

}

trap cleanup SIGINT

while true; do
        sleep 300
        echo "当前时间: $(date)"
#   for ((i=0; i<10; i++)); do
         touch ./1.txt
         rm -f ./1.txt
         touch ./1.txt
#   done




done    