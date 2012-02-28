#!/bin/bash

which yum &> /dev/null 
fedora=$?

if [ $fedora -ne 0 ]; then
    echo "This script is meant for Fedora"
    exit 1;
fi

echo ">>> Installing boost dependencies"
sudo yum install boost-devel
sudo yum install boost-program-options
sudo yum install libevent-devel
