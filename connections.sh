#!/bin/bash

while [ 1 ]; do
    count=$(netstat -aon | grep 32000 | wc -l)
    count=$(($count - 1))
    echo Connected clients: $count

    sleep 1
done
