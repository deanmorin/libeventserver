#ifndef DM_NETWORK_HPP
#define DM_NETWORK_HPP
#include <sys/socket.h>
namespace dm
{

#define REQUEST_SIZE    16

/**
 * Keep reading from the socket until bufsize bytes are read.
 * @author Dean Morin
 */
int clearSocket(int fd, char* buf, int bufsize);

} // namespace dm
#endif
