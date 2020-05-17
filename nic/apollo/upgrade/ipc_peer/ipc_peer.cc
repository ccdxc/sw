//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines ipc peer communication
///
//----------------------------------------------------------------------------

#include <fcntl.h>
#include "nic/sdk/upgrade/core/logger.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "ipc_peer.hpp"

static int
socket_un (const char *path, struct sockaddr_un *a)
{
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    a->sun_family = AF_UNIX;
    strncpy(a->sun_path, path, sizeof(a->sun_path) - 1);
    a->sun_path[sizeof(a->sun_path) - 1] = '\0';
    return fd;
}

ipc_peer_ctx *
ipc_peer_ctx::init_(const char *path, struct ev_loop *loop) {
    int fd = socket_un(path, &un_);

    if (fd < 0) {
        return NULL;
    }
    sa_sz_ = sizeof(struct sockaddr_un);
    fd_ = fd;
    recv_fd_ = -1;
    loop_ = loop;
    UPG_TRACE_DEBUG("IPC peer create fd %d", fd_);
    return this;
}

ipc_peer_ctx *
ipc_peer_ctx::factory(const char *path, struct ev_loop *loop) {
    ipc_peer_ctx *ctx;
    void *mem;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_UPGRADE, sizeof(ipc_peer_ctx));
    if (!mem) {
        UPG_TRACE_ERR("Peer context alloc failed");
        return NULL;
    }
    ctx = new (mem) ipc_peer_ctx();
    return ctx->init_(path, loop);
}

void
ipc_peer_ctx::destroy(ipc_peer_ctx *ctx) {
    if (ctx->fd() >= 0) {
        close(ctx->fd());
    }
    if (ctx->recv_fd() >= 0) {
        close(ctx->recv_fd());
    }
    ctx->~ipc_peer_ctx();
    SDK_FREE(SDK_MEM_ALLOC_UPGRADE, ctx);
}

static void
msg_read_cb (void *arg)
{
    ipc_peer_ctx *ctx = (ipc_peer_ctx *)arg;
    size_t nleft;
    size_t nread;
    ipc_peer_msg_t msg;
    int fd = ctx->recv_fd() >= 0 ? ctx->recv_fd() : ctx->fd();
    char *data, *buf;

    nread = read(fd, &msg, sizeof(msg));
    if(nread != sizeof(msg)) {
        if (nread == 0 && ctx->err_cb) {
            UPG_TRACE_INFO("IPC peer remote connection closed, fd %d", fd);
            ctx->err_cb(ctx);
        }
        return;
    }
    data = buf = (char *)SDK_CALLOC(SDK_MEM_ALLOC_UPGRADE, msg.data_size);
    if (!data) {
        UPG_TRACE_ERR("IPC peer memory allocation failed, fd %d", fd);
        if (ctx->err_cb) {
            ctx->err_cb(ctx);
        }
        return;
    }
    nleft = msg.data_size;

    do {
        nread = read(fd, data, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;       // ignore EINTRs
            }
            break;              // real error
        } else if (nread == 0) {
            break;              // EOF
        }
        nleft -= nread;
        data += nread;
    } while (nleft);

    if (nleft == 0) {
        ctx->recv_cb(buf, msg.data_size);
    } else {
        UPG_TRACE_ERR("IPC peer error in read");
    }
    SDK_FREE(SDK_MEM_ALLOC_UPGRADE, buf);
}

static void
client_connect_cb (void *arg)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int fd;
    ipc_peer_ctx *ctx = (ipc_peer_ctx *)arg;

    fd = accept(ctx->fd(), (struct sockaddr *)&addr, &addrlen);
    if (fd < 0) {
        UPG_TRACE_ERR("IPC peer accept failed on fd %d, err %s",
                      ctx->fd(), strerror(errno));
        return;
    }
    ctx->set_recv_fd(fd);
    UPG_TRACE_DEBUG("IPC peer received connection request on fd %d", fd);
    evutil_add_fd(ctx->loop(), fd, msg_read_cb, NULL, ctx);
}

sdk_ret_t
ipc_peer_ctx::listen(void) {
    unlink(un_.sun_path);
    if (bind(fd(), &sa_, sa_sz_) < 0) {
        UPG_TRACE_ERR("IPC peer bind failed, error %s", strerror(errno));
        return SDK_RET_ERR;
    }
    ::listen(fd(), 5);
    evutil_add_fd(loop(), fd(), client_connect_cb, NULL, this);
    return SDK_RET_OK;
}

sdk_ret_t
ipc_peer_ctx::connect(void) {
    while (fd() >= 0 && ::connect(fd(), &sa_, sa_sz_) == -1 &&
           errno != EISCONN) {
        UPG_TRACE_ERR("IPC peer connect failed, error %s", strerror(errno));
        // retry connect if signal interrupted us
        if (errno == EINTR || errno == EAGAIN) {
            continue;
        }
        return SDK_RET_ERR;
    }
    evutil_add_fd(loop(), fd(), msg_read_cb, NULL, this);
    return SDK_RET_OK;
}

static sdk_ret_t
send_to (const int fd, const void *data, const size_t n)
{
    const char *buf = (const char *)data;
    size_t nleft = n;
    ssize_t nwritten;

    do {
        nwritten = write(fd, buf, nleft);
        if (nwritten < 0) {
            if (errno == EINTR) {
                continue;       // ignore EINTRs
            }
            break;              // real error
        }
        nleft -= nwritten;
        buf += nwritten;
    } while (nleft);

    if (nleft == 0) {
        return SDK_RET_OK;
    }
    return SDK_RET_ERR;
}

sdk_ret_t
ipc_peer_ctx::send(const void *data, const size_t size) {
    sdk_ret_t ret;
    size_t msg_size = sizeof(ipc_peer_msg_t) + size;
    ipc_peer_msg_t *msg = (ipc_peer_msg_t *)SDK_CALLOC(SDK_MEM_ALLOC_UPGRADE, msg_size);

    if (!msg) {
        return SDK_RET_OOM;
    }
    memcpy(msg->data, data, size);
    msg->data_size = size;
    ret = send_to(fd(), msg, msg_size);
    SDK_FREE(SDK_MEM_ALLOC_UPGRADE, msg);
    return ret;
}

sdk_ret_t
ipc_peer_ctx::reply(const void *data, const size_t size) {
    sdk_ret_t ret;
    size_t msg_size = sizeof(ipc_peer_msg_t) + size;
    ipc_peer_msg_t *msg = (ipc_peer_msg_t *)SDK_CALLOC(SDK_MEM_ALLOC_UPGRADE, msg_size);

    if (!msg) {
        return SDK_RET_OOM;
    }
    memcpy(msg->data, data, size);
    msg->data_size = size;
    ret = send_to(recv_fd(), msg, msg_size);
    SDK_FREE(SDK_MEM_ALLOC_UPGRADE, msg);
    return ret;
}
