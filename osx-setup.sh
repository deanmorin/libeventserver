if [ "$(uname)" != "Darwin" ]; then
    echo "This script is meant for OSX"
    exit 1
fi

sudo ls &> /dev/null

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

sudo sysctl -w kern.maxfiles=65536
sudo sysctl -w kern.maxfilesperproc=65536
sudo sysctl -w kern.ipc.somaxconn=65536
