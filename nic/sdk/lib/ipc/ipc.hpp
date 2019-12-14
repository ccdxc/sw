//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __IPC_H__
#define __IPC_H__

#include <memory>
#include <vector>

#include <stddef.h>
#include <stdint.h>

#define IPC_MAX_ID 63
#define IPC_MAX_CLIENT 63

namespace sdk {
namespace ipc {

typedef enum ipc_msg_type {
    DIRECT    = 0,
    BROADCAST = 1,
} ipc_msg_type_t;

class ipc_msg {
public:
    virtual uint32_t code(void) = 0;
    /// \brief get a pointer to the data of the message
    virtual void *data(void) = 0;
    /// \brief get the size of the data payload
    virtual size_t length(void) = 0;
    /// \brief get the type of the message (DIRECT or BROADCAST)
    virtual ipc_msg_type_t type(void) = 0;
};
typedef std::shared_ptr<struct ipc_msg> ipc_msg_ptr;

///
/// Callbacks
///

typedef void (*request_cb)(ipc_msg_ptr msg, const void *ctx);

typedef void (*response_cb)(ipc_msg_ptr msg, const void *request_cookie,
                            const void *ctx);

typedef void (*response_oneshot_cb)(ipc_msg_ptr msg,
                                    const void *request_cookie);

typedef void (*subscription_cb)(ipc_msg_ptr msg, const void *ctx);

///
/// Init
///

typedef void (*handler_cb)(int fd, const void *ctx);
typedef void (*fd_watch_cb)(int fd, handler_cb cb, const void *set_ctx,
                            const void *ctx);

extern void ipc_init_async(uint32_t client_id, fd_watch_cb fd_watch_cb,
                           const void *fd_watch_cb_ctx);

typedef void (*handler_ms_cb)(int fd, int, void *ctx);

typedef void (*fd_watch_ms_cb)(int fd, handler_ms_cb cb, void *cb_ctx);

extern void ipc_init_metaswitch(uint32_t client_id,
                                fd_watch_ms_cb fw_watch_ms_cb);

extern void ipc_init_sync(uint32_t client_id);

///
/// Sending
///

extern void request(uint32_t recipient, uint32_t msg_code, const void *data,
                    size_t length, const void *cookie);

extern void request(uint32_t recipient, uint32_t msg_code, const void *data,
                    size_t length, response_oneshot_cb response_cb,
                    const void *cookie);

extern void broadcast(uint32_t msg_code, const void *data, size_t data_length);

extern void respond(ipc_msg_ptr msg, const void *data, size_t data_length);

///
/// Receiving
///

extern void reg_request_handler(uint32_t msg_code, request_cb callback,
                                const void *ctx);


extern void reg_response_handler(uint32_t msg_code, response_cb callback,
                                 const void *ctx);

extern void subscribe(uint32_t msg_code, subscription_cb callback,
                      const void *ctx);

// This is to be used in sync cases when we want to receive requests or
// subscription messages
extern void receive(void);

} // namespace ipc
} // namespace sdk

#endif // __IPC_H__
