//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_EVENT_THREAD_HPP__
#define __SDK_EVENT_THREAD_HPP__

#include <ev.h>

#include "lib/thread/thread.hpp"

#define EVENT_READ  0x1
#define EVENT_WRITE 0x2

namespace sdk {
namespace lib {

class event_thread;

//
// IO
//
typedef void(*event_io_cb)(struct event_io *, int fd, int events);

typedef struct event_io {
    ev_io       ev_watcher; // private
    event_io_cb callback;
    void        *ctx;
} event_io_t;

// Wrapper around ev_io_init
void event_io_init(event_io_t *io, event_io_cb callback, int fd, int events);
// Wrapper around ev_io_start
void event_io_start(event_io_t *io);
// Wrapper around ev_io_stop
void event_io_stop(event_io_t *io);

//
// Timer
//
typedef void (*event_timer_cb)(struct event_timer *);

typedef struct event_timer {
    ev_timer       ev_watcher; // private
    event_timer_cb callback;
    void           *ctx;
} event_timer_t;

// Wapper around ev_timer_init
void event_timer_init(event_timer_t *timer, event_timer_cb callback,
                      double initial_delay, double repeat);
// Wrapper around ev_timer_set
void event_timer_set(event_timer_t *timer, double initial_delay, double repeat);
// Wrapper around ev_timer_start
void event_timer_start(event_timer_t *timer);
// Wrapper around ev_timer_stop
void event_timer_stop(event_timer_t *timer);
// Wrapper around ev_timer_again. Restarts the timer.
void event_timer_again(event_timer_t *timer);

//
// Message
//
typedef void (*event_message_cb)(void *message, void *ctx);

// Send a message to a thread
void event_message_send(uint32_t thread_id, void *message);

// Use this function to set the initial watchers
typedef void (*loop_init_func_t)(void *ctx);

// Use this function callback to relaese any memory allocated for watchers
// and cleaup
typedef void (loop_exit_func_t)(void *ctx);

//
// The main class for the thread. Users should not need to use any of the methods
// directly
//
class event_thread : public thread
{
public:
    static event_thread *factory(const char *name, uint32_t thread_id,
                                 thread_role_t thread_role, uint64_t cores_mask,
                                 loop_init_func_t init_func,
                                 loop_exit_func_t exit_func,
                                 event_message_cb message_cb,
                                 uint32_t prio, int sched_policy,
                                 bool can_yield);

    void io_init(event_io_t *);
    void io_start(event_io_t *);
    void io_stop(event_io_t *);
    
    void timer_start(event_timer_t *);
    void timer_again(event_timer_t *);
    void timer_stop(event_timer_t *);

    void message_send(void *message);

    virtual sdk_ret_t start(void *ctx) override;
    virtual sdk_ret_t stop(void) override;
    
protected:
    virtual int init(const char *name, uint32_t thread_id,
                     thread_role_t thread_role, uint64_t cores_mask,
                     loop_init_func_t init_func, loop_exit_func_t exit_func,
                     event_message_cb message_cb, uint32_t prio,
                     int sched_policy, bool can_yield);
    event_thread();
    ~event_thread();

private:
    bool stop_ = false;
    struct ev_loop *loop_;
    ev_async async_watcher_;
    loop_init_func_t init_func_;
    loop_init_func_t exit_func_;
    event_message_cb message_cb_;
    void *user_ctx_;
    void run_(void);
    void handle_async_(void);
    void process_lfq_(void);

private:
    // Private static callback functions that are hooked to libev
    static void *event_thread_entry_(void *ctx);
    static void async_callback_(struct ev_loop *loop, ev_async *watcher,
                                int revents);
};

} // namespace sdk
} // namespace lib
    
#endif
