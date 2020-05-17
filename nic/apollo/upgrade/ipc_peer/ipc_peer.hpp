//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines ipc peer communication
///
//----------------------------------------------------------------------------

#ifndef __IPC_PEER_HPP__
#define __IPC_PEER_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/lib/event_thread/event_thread.hpp"

typedef void (*ipc_peer_recv_cb_t)(const void *data, size_t size);
class ipc_peer_ctx;
typedef void (*ipc_peer_err_cb_t)(ipc_peer_ctx *);

class ipc_peer_ctx {
public:
    ipc_peer_ctx() { fd_ = -1; recv_fd_ = -1; };
    ~ipc_peer_ctx() {};
    static ipc_peer_ctx *factory(const char *path, struct ev_loop *loop);
    static void destroy(ipc_peer_ctx *ctx);
    ipc_peer_recv_cb_t recv_cb;
    ipc_peer_err_cb_t err_cb;
    sdk_ret_t listen(void);
    sdk_ret_t connect(void);
    sdk_ret_t send(const void *data, const size_t size);
    sdk_ret_t reply(const void *data, const size_t size);
    int fd(void) { return fd_; }
    int recv_fd(void) { return recv_fd_; }
    struct ev_loop *loop(void) { return loop_; }
    void set_recv_fd(int fd) { recv_fd_ = fd; }

private:
    union {
        struct sockaddr    sa_;
        struct sockaddr_un un_;
    };
    size_t sa_sz_;
    int fd_;
    int recv_fd_;
    struct ev_loop *loop_;

private:
    ipc_peer_ctx *init_(const char *path, struct ev_loop *loop);
};

typedef struct ipc_peer_msg_s {
    size_t data_size;
    char data[0];
} __PACK__ ipc_peer_msg_t;

#endif    // __IPC_PEER_HPP__
