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
    stack_size=1024
fi

old=$(ulimit -s)
echo "OLD $old"
ulimit -s $stack_size
echo "stack size: $old -> $(ulimit -s)"

old=$(ulimit -n)
ulimit -n 65536
echo "open files: $old -> $(ulimit -n)"

sudo sysctl -w kern.ipc.somaxconn=8192
sudo sysctl -w kern.maxfilesperproc=65536

ulimit -s 128
ulimit -s
