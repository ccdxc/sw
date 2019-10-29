//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "event_thread.hpp"

#include "include/sdk/mem.hpp"

#define MAX_THREAD_ID 63

namespace sdk {
namespace lib {

static thread_local event_thread *t_event_thread_ = NULL;

static event_thread *g_event_thread_table[MAX_THREAD_ID + 1];

// Converts ev values to event values.
// e.g. EV_READ to EVENT_READ
int
ev_to_event (int ev_value) {
    int event_value = 0;

    if (EV_READ & ev_value) {
        event_value |= EVENT_READ;
    }
    if (EV_WRITE & ev_value) {
        event_value |= EVENT_WRITE;
    }

    return event_value;
}

// Converts event values to ev values
// e.g. EVENT_READ to EV_READ
int
event_to_ev (int event_value) {
    int ev_value = 0;

    if (EVENT_READ & event_value) {
        ev_value |= EV_READ;
    }
    if (EVENT_WRITE & event_value) {
        ev_value |= EV_WRITE;
    }

    return ev_value;
}

event_thread *
event_thread::factory(const char *name, uint32_t thread_id,
                      thread_role_t thread_role, uint64_t cores_mask,
                      loop_init_func_t init_func, loop_exit_func_t exit_func,
                      event_message_cb message_cb, event_ipc_cb ipc_cb,
                      event_ipc_client_cb ipc_client_cb,
                      uint32_t prio, int sched_policy, bool can_yield) {
    int          rv;
    void         *mem;
    event_thread *new_thread;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_EVENT_THREAD, sizeof(event_thread));
    if (!mem) {
        return NULL;
    }

    new_thread = new (mem) event_thread();
    rv = new_thread->init(name, thread_id, thread_role, cores_mask,
                          init_func, exit_func, message_cb, ipc_cb,
                          ipc_client_cb, prio, sched_policy, can_yield);
    if (rv < 0) {
        new_thread->~event_thread();
        SDK_FREE(SDK_MEM_ALLOC_LIB_THREAD, new_thread);
        return NULL;
    }

    return new_thread;
}

int
event_thread::init(const char *name, uint32_t thread_id,
                   thread_role_t thread_role, uint64_t cores_mask,
                   loop_init_func_t init_func, loop_exit_func_t exit_func,
                   event_message_cb message_cb, event_ipc_cb ipc_cb,
                   event_ipc_client_cb ipc_client_cb,
                   uint32_t prio, int sched_policy, bool can_yield) {
    int rc;

    if (thread_id > MAX_THREAD_ID) {
        return -1;
    }
    assert(g_event_thread_table[thread_id] == NULL);

    rc = thread::init(name, thread_id, thread_role, cores_mask,
                      &event_thread_entry_, prio, sched_policy, can_yield);
    if (rc < 0) {
        return rc;
    }

    this->loop_ = ev_loop_new(0);
    if (this->loop_ == NULL) {
        return -1;
    }

    this->init_func_ = init_func;
    this->exit_func_ = exit_func;
    this->message_cb_ = message_cb;
    this->ipc_cb_ = ipc_cb;
    this->ipc_client_cb_ = ipc_client_cb;
    this->user_ctx_ = NULL;

    // The async watcher is for getting messages from different threads
    this->async_watcher_.data = this;
    ev_async_init(&this->async_watcher_, event_thread::async_callback_);
    ev_async_start(this->loop_, &this->async_watcher_);

    g_event_thread_table[thread_id] = this;

    for (int i = 0; i < IPC_MAX_ID + 1; i++) {
        this->ipc_clients_[i] = NULL;
    }

    return 0;
}

event_thread::event_thread() {
}

event_thread::~event_thread() {
    g_event_thread_table[this->thread_id()] = NULL;
}

void*
event_thread::event_thread_entry_(void *ctx) {
    ((event_thread *)ctx)->run_();
    return NULL;
}

void
event_thread::async_callback_(struct ev_loop *loop, ev_async *watcher,
                              int revents) {
    // our class is watcher->data
    ((event_thread *)(watcher->data))->handle_async_();
}

void
event_thread::handle_async_(void) {
    if (this->stop_) {
        ev_break(this->loop_, EVBREAK_ONE);
        return;
    }

    this->process_lfq_();
}

void
event_thread::process_lfq_(void) {
    void *message;
    while (true) {
        message = this->dequeue();
        if (message == NULL) {
            return;
        }
        if (this->message_cb_) {
            this->message_cb_(message, this->user_ctx_);
        }
    }
}

void
event_thread::run_(void) {
    t_event_thread_ = this;
    if (this->init_func_) {
        this->init_func_(this->user_ctx_);
    }

    // IPC
    this->ipc_server_ = ipc::ipc_server::factory(this->thread_id());
    ev_io_init(&this->ipc_watcher_, &this->ipc_callback_,
               this->ipc_server_->fd(), EV_READ);
    this->ipc_watcher_.data = this;

    ev_io_start(this->loop_, &this->ipc_watcher_);

    ev_run(this->loop_, 0);

    if (this->exit_func_) {
        this->exit_func_(this->user_ctx_);
    }

    ev_loop_destroy(this->loop_);
    this->loop_ = NULL;
    ipc::ipc_server::destroy(this->ipc_server_);
}

sdk_ret_t
event_thread::start(void *ctx) {
    // This function will be called from different thread
    // No locking required
    this->user_ctx_ = ctx;
    thread::start(this);

    return SDK_RET_OK;
}

sdk_ret_t
event_thread::stop(void) {
    // This function will be called from different thread
    // No locking required
    this->stop_ = true;

    ev_async_send(this->loop_, &this->async_watcher_);

    return SDK_RET_OK;
}

static void
ev_io_callback_ (struct ev_loop *loop, ev_io *watcher, int revents)
{
    event_io_t *io = (event_io_t *)watcher;
    io->callback(io, watcher->fd, ev_to_event(revents));
}

void
event_io_init (event_io_t *io, event_io_cb callback, int fd, int events)
{
    io->callback = callback;
    ev_io_init(&io->ev_watcher, &ev_io_callback_, fd,
               event_to_ev(events));
}

void
event_io_start (event_io_t *io)
{
    // We can only add and remove events from inside the context of the thread
    assert(t_event_thread_ != NULL);

    t_event_thread_->io_start(io);
}

void
event_io_stop (event_io_t *io)
{
    // We can only add and remove events from inside the context of the thread
    assert(t_event_thread_ != NULL);

    t_event_thread_->io_stop(io);
}

void
event_thread::io_start(event_io_t *io) {
    assert(t_event_thread_ == this);
    assert(io->ev_watcher.cb == ev_io_callback_);

    ev_io_start(this->loop_, &io->ev_watcher);
}

void
event_thread::io_stop(event_io_t *io) {
    assert(t_event_thread_ == this);

    ev_io_stop(this->loop_, &io->ev_watcher);
}

static void
ev_timer_callback_ (struct ev_loop *loop, ev_timer *watcher, int revents)
{
    event_timer_t *timer = (event_timer_t *)watcher;
    timer->callback(timer);
}

void
event_timer_init (event_timer_t *timer, event_timer_cb callback,
                  double initial_delay, double repeat)
{
    timer->callback = callback;
    ev_timer_init(&timer->ev_watcher, &ev_timer_callback_, initial_delay,
        repeat);
}

void
event_timer_start (event_timer_t *timer)
{
    // We can only add and remove events from inside the context of the thread
    assert(t_event_thread_ != NULL);

    t_event_thread_->timer_start(timer);
}

void
event_timer_stop (event_timer_t *timer)
{
    // We can only add and remove events from inside the context of the thread
    assert(t_event_thread_ != NULL);

    t_event_thread_->timer_stop(timer);
}

void
event_timer_again (event_timer_t *timer)
{
    // We can only manipulate events from inside the context of the thread
    assert(t_event_thread_ != NULL);

    t_event_thread_->timer_again(timer);
}

void
event_thread::timer_start(event_timer_t *timer) {
    assert(t_event_thread_ == this);
    assert(timer->ev_watcher.cb == ev_timer_callback_);

    ev_timer_start(this->loop_, &timer->ev_watcher);
}

void
event_thread::timer_stop(event_timer_t *timer) {
    assert(t_event_thread_ == this);

    ev_timer_stop(this->loop_, &timer->ev_watcher);
}

void
event_thread::timer_again(event_timer_t *timer) {
    assert(t_event_thread_ == this);

    ev_timer_again(this->loop_, &timer->ev_watcher);
}

void
event_message_send (uint32_t thread_id, void *message)
{
    assert(thread_id <= MAX_THREAD_ID);
    assert(g_event_thread_table[thread_id] != NULL);

    g_event_thread_table[thread_id]->message_send(message);
}

void
event_thread::message_send(void *message) {
    // This function can be called from different thread
    // No locking required

    this->enqueue(message);
    ev_async_send(this->loop_, &this->async_watcher_);
}

void
event_thread::handle_ipc_(void) {
    while (1) {
        ipc::ipc_msg_ptr msg = this->ipc_server_->recv();
        if (msg == nullptr) {
            return;
        }
        // We received an IPC message but don't have a handler
        assert(this->ipc_cb_ != NULL);
        this->ipc_cb_(msg, this->user_ctx_);
    }
}

void
event_thread::handle_ipc_client_(ev_io *watcher) {
    // Todo: Fixme: Stavros: Inefficient
    uint32_t sender;
    for (sender = 0; sender < IPC_MAX_ID + 1; sender++) {
        if (watcher == &this->ipc_client_watchers_[sender]) {
            break;
        }
    }
    assert(sender < IPC_MAX_ID + 1);
    while (1) {
        const void *cookie;
        ipc::ipc_msg_ptr msg = this->ipc_clients_[sender]->recv(&cookie);
        if (msg == nullptr) {
            return;
        }
        // We received an IPC reply but don't have a handler
        assert(this->ipc_client_cb_ != NULL);
        this->ipc_client_cb_(sender, msg, this->user_ctx_, cookie);
    }
}

void
event_ipc_reply (ipc::ipc_msg_ptr msg, const void *data, size_t data_length)
{
    assert(t_event_thread_ != NULL);

    t_event_thread_->ipc_reply(msg, data, data_length);
}

void
event_thread::ipc_reply(ipc::ipc_msg_ptr msg, const void *data,
                        size_t data_length) {
    assert(t_event_thread_ == this);
    this->ipc_server_->reply(msg, data, data_length);
}


void
event_thread::ipc_callback_(struct ev_loop *loop, ev_io *watcher, int revents) {
    ((event_thread *)watcher->data)->handle_ipc_();
}

void
event_ipc_send (uint32_t recipient, const void *data, size_t data_length,
                const void *cookie)
{
    assert(t_event_thread_ != NULL);
    t_event_thread_->ipc_send(recipient, data, data_length, cookie);
}

void
event_thread::ipc_send(uint32_t recipient, const void *data, size_t data_length,
                       const void *cookie)
{
    ipc::ipc_client *client;
    
    assert(t_event_thread_ == this);

    client = this->ipc_clients_[recipient];
    if (client == NULL) {
        ev_io *watcher = &this->ipc_client_watchers_[recipient];

        client = ipc::ipc_client::factory(recipient);
        this->ipc_clients_[recipient] = client;
        ev_io_init(watcher, &this->ipc_client_callback_, client->fd(), EV_READ);
        watcher->data = this;
        ev_io_start(this->loop_, watcher);

        // If we don't do this we don't get any events coming from ZMQ
        // It doesn't play ver well with libevent
        client->recv(NULL);
    }
    client->send(data, data_length, cookie);
}

void
event_thread::ipc_client_callback_(struct ev_loop *loop, ev_io *watcher,
                                   int revents) {
    ((event_thread *)watcher->data)->handle_ipc_client_(watcher);
}

} // namespace lib
} // namespace sdk
