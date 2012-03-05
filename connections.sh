#!/bin/bash

while [ 1 ]; do
    count=$(netstat -aon | grep 32000 | wc -l)
    if [ $count -eq 0 ]; then
        echo "Server is not running." 
        sleep 5
    fi
    count=$(($count - 1))
    if [ $count -ne $prev ]; then
        echo Connected clients: $count
        prev=$count
    fi
done
