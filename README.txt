SETUP

First run the setup.sh script for either Fedora or OSX. You will be asked if the
shell will be running a client or not. This is important since the shell running
the server will require more stack space.

Run 'make' to build the binaries, or 'make debug' to build binaries that display
more information to stdout.


USING THE PROGRAM

Server

The server requires an argument for which event base to use (select, poll,
epoll, etc.). The options can be viewed with 'server --help'.

Client

The client has many configurable options. These can be viewed with
'client --help'.
