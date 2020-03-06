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

// UpDown Events
typedef void(*updown_up_cb)(uint32_t thread_id, void *);

void updown_up_subscribe(uint32_t thread_it, updown_up_cb cb, void *ctx);

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
// Wrapper around ev_now
double timestamp_now(void);

//
// Message
//
typedef void (*message_cb)(void *message, void *ctx);

// Send a message to a thread
void message_send(uint32_t thread_id, void *message);

//
// Rest
//

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
                                 bool can_yield,
                                 bool sync_ipc = false);
    static void destroy(event_thread *thread);

    void updown_up_subscribe(uint32_t thread_id, updown_up_cb cb, void *ctx);
    void prepare_start(prepare_t *prepare);
    void prepare_stop(prepare_t *prepare);

    void io_start(io_t *);
    void io_stop(io_t *);

    void timer_start(timer_t *);
    void timer_again(timer_t *);
    void timer_stop(timer_t *);

    void message_send(void *message);

    void handle_thread_up(uint32_t thread_id);

    virtual sdk_ret_t start(void *ctx) override;
    virtual sdk_ret_t stop(void) override;
    struct ev_loop *ev_loop(void) { return loop_; }

protected:
    virtual int init(const char *name, uint32_t thread_id,
                     sdk::lib::thread_role_t thread_role, uint64_t cores_mask,
                     loop_init_func_t init_func, loop_exit_func_t exit_func,
                     message_cb message_cb, uint32_t prio,
                     int sched_policy, bool can_yield, bool sync_ipc);
    event_thread();
    ~event_thread();

private:
    bool stop_ = false;
    struct ev_loop *loop_;
    ev_async async_watcher_;
    loop_init_func_t init_func_;
    loop_init_func_t exit_func_;
    message_cb message_cb_;
    std::map<uint32_t, updown_up_cb> updown_up_cbs_;
    std::map<uint32_t, void*> updown_up_ctxs_;
    void *user_ctx_;
    bool sync_ipc_;
    void run_(void);
    void handle_async_(void);
    void process_lfq_(void);
    void create_ipc_watcher_(int fd, sdk::ipc::handler_cb cb, const void *ctx);

private:
    // Private static callback functions that are hooked to libev
    static void *event_thread_entry_(void *ctx);
    static void async_callback_(struct ev_loop *loop, ev_async *watcher,
                                int revents);
    static void create_ipc_watcher(int fd, sdk::ipc::handler_cb cb,
                                   const void *ctx,
                                   const void *ipc_poll_fd_ctx);
};

} // namespace sdk
} // namespace lib

#endif
