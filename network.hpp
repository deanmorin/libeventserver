#ifndef DM_NETWORK_HPP
#define DM_NETWORK_HPP
#include <stdio.h>
#include <sys/socket.h>
namespace dm
{

/** The total size of the request packets sent to the server. */
#define REQUEST_SIZE    66

/**
 * Keep reading from the socket until bufsize bytes are read.
 *
 * @author Dean Morin
 * @param fd The socket to read from.
 * @param buf The buffer to fill.
 * @param bufsize The size of the buffer and the number of bytes to read.
 * @return The number of bytes read, or -1 if the socket has closed.
 */
int clearSocket(int fd, char* buf, int bufsize);

/**
 * Display detailed socket error info. The msg will be passed to perror() if err
 * is set to 0 (indicating that errno was set), otherwise it will be written to 
 * stderr.
 *
 * @author Dean Morin
 * @param msg The msg to display.
 * @param err The error returned by the failed function call. This should be 0 
 *      if the call sets errno.
 * @return The error value (errno if err was 0, otherwise err).
 */
int sockError(const char* msg, int err);

} // namespace dm
#endif
