#!/bin/bash

which yum &> /dev/null 
yumresult=$?

if [ $yumresult -ne 0 ]; then
    echo "This script is meant for Fedora"
    exit 1
fi

user=$(whoami)

if [ "$user" != "root" ]; then 
    echo ">>> su:"
    su
fi

echo ">>> Installing boost dependencies"
yum install boost-devel
yum install boost-program-options
yum install libevent-devel

read -n 1 -p ">>> Is this a client machine? [y/n] "
echo
if [ "$REPLY" != "Y" ] && [ "$REPLY" != "y" ]; then
    exit 0
fi

echo ">>> Current stack size:"
ulimit -s
echo ">>> Reducing stack size:"
ulimit -s 512
echo ">>> Increasing max socket backlog:"
sysctl -w net.core.somaxconn=8096
echo ">>> Increasing max number of open files:"
ulimit -n 8096
