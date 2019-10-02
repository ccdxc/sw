//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __IPC_H__
#define __IPC_H__

#include <memory>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace sdk {
namespace lib {
namespace ipc {

class ipc_msg {
public:
    // Returns a pointer to the payload
    virtual void *data(void) = 0;
    // Returns the length of the payload
    virtual size_t length(void) = 0;
};
typedef std::shared_ptr<ipc_msg> ipc_msg_ptr;

class ipc_client {
public:
    static ipc_client *factory(uint32_t id);
    static void destroy(ipc_client *client);
    virtual ipc_msg_ptr send_recv(uint32_t recipient, const void *data,
                                      size_t data_length) = 0;
    virtual ipc_msg_ptr recv(void) = 0;
    virtual void reply(ipc_msg_ptr msg, const void *data,
                       size_t data_length) = 0;
    virtual int fd(void) = 0;
};

} // namespace ipc
} // namespace lib
} // namespace sdk

#endif // __IPC_H__
