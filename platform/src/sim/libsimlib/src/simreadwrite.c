/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simmsg.h"
#include "simlib_impl.h"

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
sim_readn(int fd, void *bufarg, const size_t n)
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
sim_writen(int fd, const void *bufarg, const size_t n)
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

/*
 * We sent a msg and expect a response of a certain msgtype.
 * Wait for the response here, but continue to handle incoming
 * msgs while we wait.
 */
int
sim_wait_for_resp(int s, simmsgtype_t msgtype, simmsg_t *m,
                  msg_handler_t msg_handler)
{
    int got_sync_req = 0;
    int r = -1;

    while (sim_readn(s, m, sizeof(simmsg_t)) > 0) {
        /*
         * If this is the msg we were waiting for, we're done.
         */
        if (m->msgtype == msgtype) {
            r = 0;
            break;
        }
        /*
         * While waiting for our msg we received a SYNC_REQ.
         * This means our blocking request was sent from us
         * at the same time as the peer sent a SYNC_REQ.
         * We are still expecting the response we are waiting
         * for so remember the pending SYNC_REQ and continue to
         * drain the pipe waiting for our response.  After we
         * get the response we are waiting for we can handle
         * the SYNC_REQ below.
         */
        if (m->msgtype == SIMMSG_SYNC_REQ) {
            got_sync_req = 1;
            continue;
        }
        /*
         * Not the msg we are waiting for, so pass to caller's
         * handler for processing.  We will continue to wait
         * for our response.
         */
        if (msg_handler)
            msg_handler(s, m);
    }
    /*
     * If we got a SYNC_REQ while waiting for our msgtype,
     * ack it now that we have drained the pipe and received
     * our expected response.
     */
    if (got_sync_req) {
        sim_sync_ack(s, msg_handler);
    }
    return r;
}

int
sim_do_read(int s, simmsgtype_t msgtype,
            u_int16_t bdf, u_int8_t bar,
            u_int64_t addr, u_int32_t size, u_int64_t *val,
            msg_handler_t msg_handler)
{
    int r;
    simmsg_t m = {
        .msgtype = msgtype,
        .u.read.bdf = bdf,
        .u.read.bar = bar,
        .u.read.addr = addr,
        .u.read.size = size,
    };

    r = sim_writen(s, &m, sizeof(m));
    if (r < 0) return r;

    r = sim_wait_for_resp(s, SIMMSG_RDRESP, &m, msg_handler);
    if (r < 0) return r;

    if (m.u.readres.error == 0) {
        *val = m.u.readres.val;
    }
    return -m.u.readres.error;
}

int
sim_do_write(int s, simmsgtype_t msgtype,
             u_int16_t bdf, u_int8_t bar,
             u_int64_t addr, u_int32_t size, u_int64_t val,
             msg_handler_t msg_handler, int sync)
{
    int r;
    simmsg_t m = {
        .msgtype = msgtype,
        .u.write.bdf = bdf,
        .u.write.bar = bar,
        .u.write.addr = addr,
        .u.write.size = size,
        .u.write.val = val,
    };

    r = sim_writen(s, &m, sizeof(m));
    if (r < 0) return r;

    if (sync) {
        r = sim_wait_for_resp(s, SIMMSG_WRRESP, &m, msg_handler);
        if (r < 0) return r;
        r = -m.u.writeres.error;
    }
    return r;
}

void
sim_discard(int s, size_t size)
{
    char buf[512];
    size_t sz;

    while (size > 0) {
        sz = MIN(size, sizeof(buf));
        sim_readn(s, buf, sz);
        size -= sz;
    }
}

int
sim_sync_ack(int s, msg_handler_t msg_handler)
{
    simmsg_t m = {
        .msgtype = SIMMSG_SYNC_ACK,
    };

    if (sim_writen(s, &m, sizeof(m)) < 0) {
        return -1;
    }
    return sim_wait_for_resp(s, SIMMSG_SYNC_REL, &m, msg_handler);
}
