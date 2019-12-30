//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <poll.h>
#include <pthread.h> 
#include <unistd.h>

#include "lib/ipc/ipc.hpp"

#define T_CLIENT_1 1

using namespace sdk::ipc;

class ipc_test : public ::testing::Test {
public:
protected:
    ipc_test() {
    }
    
    virtual ~ipc_test() {
    }
    
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

struct poll_s {
    struct pollfd fds[64];
    const void *ctxs[64];
    handler_cb cbs[64];
    nfds_t nfds;
};

static ipc_msg_ptr g_async_thread_pending_message = nullptr;

void
poll_fd_watch_cb (int fd, handler_cb cb, const void *ctx,
                  const void *fd_watch_cb_ctx)
{
    struct poll_s *poll_fds = (struct poll_s *)fd_watch_cb_ctx;

    int next = poll_fds->nfds;
    poll_fds->fds[next].fd = fd;
    poll_fds->fds[next].events = POLLIN;
    poll_fds->ctxs[next] = ctx;
    poll_fds->cbs[next] = cb;
    poll_fds->nfds += 1;

    printf("Now also watching fd: %i. Total: %lu\n", fd, poll_fds->nfds);
}

void req_1_callback (ipc_msg_ptr msg, const void *ctx)
{
    printf("Got message: %s\n", (char *)msg->data());
    assert(g_async_thread_pending_message == nullptr);
    g_async_thread_pending_message = msg;
}

void req_99_callback (ipc_msg_ptr msg, const void *ctx)
{
    bool *exit = (bool *)ctx;

    printf("Got request 99. Will exit now\n");
    respond(msg, NULL, 0);
    *exit = true;
}

void sub_2_callback (ipc_msg_ptr msg, const void *ctx)
{
    printf("Got notification 2: %s\n", (char *)msg->data());
    const char rsp[] = "pong";
    respond(g_async_thread_pending_message, rsp, sizeof(rsp));
    g_async_thread_pending_message = nullptr;
}

void *async_thread_run (void *arg)
{
    bool exit = false;
    int event_count;
    struct poll_s poll_fds;
    poll_fds.nfds = 0;

    ipc_init_async(T_CLIENT_1, poll_fd_watch_cb, &poll_fds);

    reg_request_handler(1, req_1_callback, NULL, true);
    reg_request_handler(99, req_99_callback, &exit);

    subscribe(2, sub_2_callback, NULL);
    
    while (!exit) {
        event_count = poll(poll_fds.fds, poll_fds.nfds, -1);
        printf("event count: %i\n", event_count);
        if (event_count > 0) {
            for (nfds_t i = 0; i < poll_fds.nfds; i++) {
                if (poll_fds.fds[i].revents & POLLIN) {
                    printf("Calling callback for fd %i\n",
                           poll_fds.fds[i].fd);
                    poll_fds.cbs[i](poll_fds.fds[i].fd, poll_fds.ctxs[i]);
                }
            }
        }
    }

    return NULL;
}

void
rsp_1_callback (ipc_msg_ptr msg, const void *cookie, const void *ctx)
{
    printf("got response: %s\n",
           (char *)msg->data());
}

void
rsp_99_callback (ipc_msg_ptr msg, const void *cookie, const void *ctx)
{
    printf("got response to 99. Will exit now\n");
}

void *
sync_thread1_run (void *arg)
{
    const char msg[] = "ping1";
    
    reg_response_handler(1, rsp_1_callback, NULL);    
    request(T_CLIENT_1, 1, msg, sizeof(msg), NULL);

    return NULL;
}

void *
sync_thread2_run (void *arg)
{
    const char msg[] = "ping2";
    
    reg_response_handler(1, rsp_1_callback, NULL);    
    request(T_CLIENT_1, 1, msg, sizeof(msg), NULL);

    return NULL;
}

void *
sync_thread3_run (void *arg)
{
    const char msg[] = "process the last message";

    sleep(2);
    
    broadcast(2, msg, sizeof(msg));

    sleep(1);
    
    broadcast(2, msg, sizeof(msg));
    
    reg_response_handler(99, rsp_99_callback, NULL);    
    request(T_CLIENT_1, 99, NULL, 0, NULL);
    
    return NULL;
}

TEST_F (ipc_test, ping) {
    pthread_t async_thread_id;
    pthread_t sync_thread1_id;
    pthread_t sync_thread2_id;
    pthread_t sync_thread3_id;
    
    pthread_create(&async_thread_id, NULL, async_thread_run, NULL);
    pthread_create(&sync_thread1_id, NULL, sync_thread1_run, NULL);
    pthread_create(&sync_thread2_id, NULL, sync_thread2_run, NULL);
    pthread_create(&sync_thread3_id, NULL, sync_thread3_run, NULL);

    pthread_join(sync_thread1_id, NULL);
    pthread_join(sync_thread2_id, NULL);
    pthread_join(sync_thread3_id, NULL);
    pthread_join(async_thread_id, NULL);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
