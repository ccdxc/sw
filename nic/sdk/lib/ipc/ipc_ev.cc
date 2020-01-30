//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <assert.h>
#include <map>
#include <memory>
#include <queue>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/eventfd.h>
#include <unistd.h>
#include <vector>
#include <ev.h>

#include "include/sdk/base.hpp"
#include "ipc.hpp"
#include "ipc_internal.hpp"
#include "subscribers.hpp"
#include "zmq_ipc.hpp"

namespace sdk {
namespace ipc {

typedef struct ev_watcher_ {
    ev_io ev;
    handler_cb cb;
    const void *ctx;
} ev_watcher_t;

static void
ev_watch_cb_wrap (struct ev_loop *loop, ev_io *w, int revents)
{
    ev_watcher_t *watcher = (ev_watcher_t *)w;

    watcher->cb(watcher->ev.fd, watcher->ctx);
}
    
static void
ev_watch_cb (int fd, handler_cb cb, const void *set_ctx, const void *ctx)
{
    ev_watcher_t *watcher = (ev_watcher_t *)malloc(sizeof(*watcher));

    watcher->cb = cb;
    watcher->ctx = ctx;
    
    ev_io_init((ev_io *)watcher, ev_watch_cb_wrap, fd, EV_READ);
    ev_io_start(EV_DEFAULT, (ev_io *)watcher);
}

void
ipc_init_ev_default (uint32_t client_id)
{
    ipc_init_async(client_id, ev_watch_cb, NULL);
}

}
}
