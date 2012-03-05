#!/bin/bash

cat /etc/issue | grep -i fedora &> /dev/null

if [ "$?" -ne 0 ]; then
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
echo
echo ">>> Installing libevent dependencies"
yum install libevent-devel

echo
read -n 1 -p ">>> Is this a client machine? [y/n] "
echo
if [ "$REPLY" == "Y" ] || [ "$REPLY" == "y" ]; then
    stack_size=512
else
    stack_size=8192
fi

old=$(ulimit -s)
ulimit -s $stack_size
echo "stack size: $old -> $(ulimit -s)"

old=$(ulimit -n)
ulimit -n 65536
echo "open files: $old -> $(ulimit -n)"

old=$(sysctl net.core.somaxconn | tr -cd [:digit:])
sysctl -w net.core.somaxconn=8192 &> /dev/null
new=$(sysctl net.core.somaxconn | tr -cd [:digit:])
echo "max socket backlog: $old -> $new"

echo
echo ">>> Ensure that iptables are cleared"
echo
iptables -X
iptables -F
iptables -L
