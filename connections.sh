#!/bin/bash

prev=-1

while [ 1 ]; do
    count=$(netstat -aon | grep 32000 | wc -l)
    if [ $count -eq 0 ]; then
        echo "Server is not running." 
        # wait for server to come up
        while [ ! $(netstat -aon | grep 32000 | wc -l) ]; do
            sleep 1
        done
    else
        # don't count the listening socket
        count=$(($count - 1))
        if [ "$count" -ne "$prev" ]; then
            echo Connected clients: $count
            prev=$count
        fi
    fi
done
