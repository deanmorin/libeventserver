#include "network.hpp"
#include <errno.h>
#include <iostream>
#include <stdlib.h>
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
#ifdef DEBUG
            perror("recv(): EAGAIN");
#endif
            break;
        } 
        else if (!read)
        {
            return -1; 
        }
        bp += read;
        bytesToRead -= read;
    }
    return 0;
}

} // namespace dm
