/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
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
pmreadn(int fd, void *bufarg, const size_t n)
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
pmwriten(int fd, const void *bufarg, const size_t n)
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

int
pciemgr_msgalloc(pmmsg_t **m, size_t len)
{
    *m = calloc(1, sizeof(pmmsg_hdr_t) + len);
    (*m)->hdr.msglen = len;
    assert(*m != NULL);
    return 0;
}

void
pciemgr_msgfree(pmmsg_t *m)
{
    free(m);
}

int
pciemgr_msgsend(int fd, pmmsg_t *m)
{
    const int len = sizeof(pmmsg_hdr_t) + m->hdr.msglen;
    return  pmwriten(fd, m, len);
}

int
pciemgr_msgrecv(int fd, pmmsg_t **m)
{
    pmmsg_hdr_t msghdr;
    pmmsg_t *newmsg;
    int r;

    /* read msg header */
    pmreadn(fd, &msghdr, sizeof(msghdr));
    /* alloc msg of indicated size to contain msg */
    pciemgr_msgalloc(&newmsg, msghdr.msglen);
    /* copy header to new msg */
    newmsg->hdr = msghdr;
    /* read message body */
    r = pmreadn(fd, (char *)newmsg + sizeof(msghdr), msghdr.msglen);
    *m = newmsg;
    return r;
}
