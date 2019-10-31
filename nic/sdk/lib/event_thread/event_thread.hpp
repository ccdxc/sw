//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_EVENT_THREAD_HPP__
#define __SDK_EVENT_THREAD_HPP__

#include <map>

#include <ev.h>

#include "lib/thread/thread.hpp"
#include "lib/ipc/ipc.hpp"

#define EVENT_READ  0x1
#define EVENT_WRITE 0x2

namespace sdk {
namespace event_thread {

class event_thread;

//
// Prepare
//
typedef void(*prepare_cb)(struct prepare_ *, void *);

typedef struct prepare_{
    ev_prepare ev_watcher; // private
    prepare_cb callback;
    void *ctx;
} prepare_t;

void prepare_init(prepare_t *prepare, prepare_cb callback, void *ctx);
void prepare_start(prepare_t *prepare);
void prepare_stop(prepare_t *prepare);

//
// IO
//
typedef void(*io_cb)(struct io_ *, int fd, int events);

typedef struct io_ {
    ev_io ev_watcher; // private
    io_cb callback;
    void  *ctx;
} io_t;

// Wrapper around ev_io_init
void io_init(io_t *io, io_cb callback, int fd, int events);
// Wrapper around ev_io_start
void io_start(io_t *io);
// Wrapper around ev_io_stop
void io_stop(io_t *io);

//
// Timer
//
typedef void (*timer_cb)(struct timer_ *);

typedef struct timer_ {
    ev_timer ev_watcher; // private
    timer_cb callback;
    void     *ctx;
} timer_t;

// Wapper around ev_timer_init
void timer_init(timer_t *timer, timer_cb callback,
                double initial_delay, double repeat);
// Wrapper around ev_timer_set
void timer_set(timer_t *timer, double initial_delay, double repeat);
// Wrapper around ev_timer_start
void timer_start(timer_t *timer);
// Wrapper around ev_timer_stop
void timer_stop(timer_t *timer);
// Wrapper around ev_timer_again. Restarts the timer.
void timer_again(timer_t *timer);

//
// Message
//
typedef void (*message_cb)(void *message, void *ctx);

// Send a message to a thread
void message_send(uint32_t thread_id, void *message);

//
// Pub-Sub
//

// Callback for subscribed message
typedef void (*sub_cb)(void *data, size_t data_length, void *ctx);

// Subscribe to a message
void subscribe(uint32_t msg_code, sub_cb callback);

// Publish a message
void publish(uint32_t msg_code, void *data, size_t data_length);

//
// RPC
//

// RPC request callback
typedef void (*rpc_request_cb)(ipc::ipc_msg_ptr ipc_msg, void *ctx);

// Register an request handler
void rpc_reg_request_handler(uint32_t msg_code,
                             rpc_request_cb callback);

// Send a reply to an ipc_msg(Must always send a reply!)
void rpc_response(ipc::ipc_msg_ptr ipc_msg, const void *data,
                  size_t data_length);

// RPC response callback
typedef void (*rpc_response_cb)(uint32_t sender, ipc::ipc_msg_ptr ipc_msg,
                                void *ctx, const void *cookie);

// Register response callback
void rpc_reg_response_handler(uint32_t msg_code,
                              rpc_response_cb callback);

// Send a message to an RPC server. The response will come on the callback above
void rpc_request(uint32_t recipient, uint32_t msg_code, const void *data,
                 size_t data_length, const void *cookie);

// Use this function to set the initial watchers
typedef void (*loop_init_func_t)(void *ctx);

// Use this function callback to relaese any memory allocated for watchers
// and cleaup
typedef void (loop_exit_func_t)(void *ctx);

//
// The main class for the thread. Users should not need to use any of the methods
// directly
//
class event_thread : public sdk::lib::thread
{
public:
    static event_thread *factory(const char *name, uint32_t thread_id,
                                 sdk::lib::thread_role_t thread_role,
                                 uint64_t cores_mask,
                                 loop_init_func_t init_func,
                                 loop_exit_func_t exit_func,
                                 message_cb message_cb,
                                 uint32_t prio, int sched_policy,
                                 bool can_yield);

    void prepare_start(prepare_t *prepare);
    void prepare_stop(prepare_t *prepare);

    void io_start(io_t *);
    void io_stop(io_t *);

    void timer_start(timer_t *);
    void timer_again(timer_t *);
    void timer_stop(timer_t *);

    void message_send(void *message);

    void subscribe(uint32_t msg_code, sub_cb callback);

    void rpc_reg_request_handler(uint32_t msg_code,
                                 rpc_request_cb callback);

    void rpc_response(ipc::ipc_msg_ptr msg, const void *data,
                      size_t data_legnth);

    void rpc_reg_response_handler(uint32_t msg_code,
                                  rpc_response_cb callback);

    void rpc_request(uint32_t recipient, uint32_t msg_code, const void *data,
                     size_t data_length, const void *cookie);

    virtual sdk_ret_t start(void *ctx) override;
    virtual sdk_ret_t stop(void) override;
    struct ev_loop *ev_loop(void) { return loop_; }

protected:
    virtual int init(const char *name, uint32_t thread_id,
                     sdk::lib::thread_role_t thread_role, uint64_t cores_mask,
                     loop_init_func_t init_func, loop_exit_func_t exit_func,
                     message_cb message_cb, uint32_t prio,
                     int sched_policy, bool can_yield);
    event_thread();
    ~event_thread();

private:
    bool stop_ = false;
    struct ev_loop *loop_;
    ipc::ipc_server *ipc_server_;
    ev_io ipc_watcher_;
    ipc::ipc_client *ipc_clients_[IPC_MAX_ID + 1];
    ev_io ipc_client_watchers_[IPC_MAX_ID + 1];
    ev_async async_watcher_;
    loop_init_func_t init_func_;
    loop_init_func_t exit_func_;
    message_cb message_cb_;
    std::map<uint32_t, sub_cb> sub_cbs_;
    std::map<uint32_t, rpc_request_cb> rpc_req_cbs_;
    std::map<uint32_t, rpc_response_cb > rpc_rsp_cbs_;
    void *user_ctx_;
    void run_(void);
    void handle_async_(void);
    void process_lfq_(void);
    void handle_ipc_(void);
    void handle_ipc_client_(ev_io *watcher);

private:
    // Private static callback functions that are hooked to libev
    static void *event_thread_entry_(void *ctx);
    static void async_callback_(struct ev_loop *loop, ev_async *watcher,
                                int revents);
    static void ipc_callback_(struct ev_loop *loop, ev_io *watcher,
                              int revents);
    static void ipc_client_callback_(struct ev_loop *loop, ev_io *watcher,
                                     int revents);
};

} // namespace sdk
} // namespace lib

#endif
