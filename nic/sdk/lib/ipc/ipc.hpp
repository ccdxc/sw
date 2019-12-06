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

/// \brief send a message to an ipc_server and wait for the reply
/// \param[in] recipient is the id of the ipc_server to send the message to
/// \param[in] data is a pointer to the data of the message to be send
/// \param[in] data_length is the length of the data to send
extern ipc_msg_ptr request(uint32_t recipient, uint32_t msg_code,
                           const void *data, size_t data_length);

extern void broadcast(uint32_t msg_code, const void *data, size_t data_length);

class ipc_server {
public:
    /// \brief create a new ipc server
    /// \param[in] id is the id that will be used by clients to send this server
    ///               requests
    static ipc_server *factory(uint32_t id);
    static void destroy(ipc_server *server);
    virtual void subscribe(uint32_t msg_code) = 0;
    virtual int fd(void) = 0;
    virtual ipc_msg_ptr recv(void) = 0;
    virtual void reply(ipc_msg_ptr msg, const void *data,
                       size_t data_length) = 0;
};

class ipc_client {
public:
    static ipc_client *factory(uint32_t recipient);
    static void destroy(ipc_client *client);
    virtual int fd(void) = 0;
    virtual void send(uint32_t msg_code, const void *data, size_t data_length,
                      const void *cookie) = 0;
    virtual void broadcast(uint32_t msg_code, const void *data,
                           size_t data_length) = 0;
    virtual ipc_msg_ptr recv(const void** cokie) = 0;
};

} // namespace ipc
} // namespace sdk

#endif // __IPC_H__
