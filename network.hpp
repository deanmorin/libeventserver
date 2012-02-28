#ifndef DM_NETWORK_HPP
#define DM_NETWORK_HPP
#include <stdio.h>
#include <sys/socket.h>
namespace dm
{

#define REQUEST_SIZE    16

/**
 * Keep reading from the socket until bufsize bytes are read.
 *
 * @param fd The socket to read from.
 * @param buf The buffer to fill.
 * @param bufsize The size of the buffer and the number of bytes to read.
 * @author Dean Morin
 */
int clearSocket(int fd, char* buf, int bufsize);

} // namespace dm
#endif
