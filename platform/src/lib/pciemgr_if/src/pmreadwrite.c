/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pciesvc_impl.h"

/*
 * Read n bytes from file descriptor
 * @fd: file descriptor
 * @buf: buffer to read into
 * @n: how many bytes to read into buffer
 *
 * Returns 0 if EOF (client closed), < 0 if error, otherwise
 * returns the number of bytes read.
 *
 * Pretty much right-out-of Stevens UNIX Network Programming,
 * but don't block/unblock signals, just retry on EINTR.
 */
ssize_t
pm_readn(int fd, void *bufarg, const size_t n)
{
    char *buf = (char *)bufarg;
    size_t nleft = n;
    ssize_t nread;

    do {
        nread = read(fd, buf, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;       /* ignore EINTRs */
            }
            break;              /* real error */
        } else if (nread == 0) {
            break;              /* EOF */
        }
        nleft -= nread;
        buf += nread;
    } while (nleft);

    if (nread < 0) {
        return (nread);         /* error, return < 0 */
    } else {
        return (n - nleft);
    }
}

/*
 * Write n bytes to file descriptor
 * @fd: file descriptor
 * @buf: buffer to read into
 * @n: how many bytes to read into buffer
 *
 * Returns < 0 if error, otherwise
 * returns the number of bytes read.
 */
ssize_t
pm_writen(int fd, const void *bufarg, const size_t n)
{
    const char *buf = (const char *)bufarg;
    size_t nleft = n;
    ssize_t nwritten;

    do {
        nwritten = write(fd, buf, nleft);
        if (nwritten < 0) {
            if (errno == EINTR) {
                continue;       /* ignore EINTRs */
            }
            break;              /* real error */
        }
        nleft -= nwritten;
        buf += nwritten;
    } while (nleft);

    if (nwritten < 0) {
        return (nwritten);      /* error, return < 0 */
    } else {
        return (n - nleft);
    }
}
