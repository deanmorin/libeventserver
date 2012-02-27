#include "network.hpp"
#include <errno.h>
#include <iostream>
namespace dm
{

int clearSocket(int fd, char* buf, int bufsize)
{
    int	read = 0;
    int bytesToRead = bufsize;
    char* bp = buf;

    while ((read = recv(fd, bp, bytesToRead, 0)) < bytesToRead)
    {
        if (read == -1)
        {
            if (errno != EAGAIN)
            {
                perror("recv()");
                exit(1);
            }
            std::cerr << "DOES IT EVER HAPPEN?\n";
            break;
        } 
        bp += read;
        bytesToRead -= read;
    }
    return 0;
}

} // namespace dm
