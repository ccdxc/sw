//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "ipc.hpp"
#include "zmq_ipc.hpp"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <zmq.h>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"

namespace sdk {
namespace ipc {

class ipc_client_sync {
public:
    ipc_client_sync();
    ipc_msg_ptr send_recv(uint32_t recipient, uint32_t msg_code,
                          const void *data, size_t data_length);
private:
    zmq_ipc_client_sync_ptr clients[IPC_MAX_ID + 1];
};

static thread_local ipc_client_sync *t_client = NULL;

ipc_msg_ptr
request (uint32_t recipient, uint32_t msg_code, const void *data,
           size_t data_length)
{
    // return zmq_ipc_client_sync::send_recv_once(recipient, data, data_length);
    if (!t_client) {
        t_client = new ipc_client_sync();
    }
    return t_client->send_recv(recipient, msg_code, data, data_length);
}

ipc_client_sync::ipc_client_sync() {
    for (int i = 0; i < IPC_MAX_ID + 1; i++) {
        this->clients[i] = nullptr;
    }
}

ipc_msg_ptr
ipc_client_sync::send_recv(uint32_t recipient, uint32_t msg_code,
                           const void *data, size_t data_length) {
    assert(recipient <= IPC_MAX_ID);
    if (this->clients[recipient] == nullptr) {
        this->clients[recipient] = std::make_shared<zmq_ipc_client_sync>();
        this->clients[recipient]->connect(recipient);
    }
    return this->clients[recipient]->send_recv(msg_code, data, data_length);
}

ipc_server *
ipc_server::factory(uint32_t id) {
    return zmq_ipc_server::factory(id);
}

void
ipc_server::destroy(ipc_server *server) {
    zmq_ipc_server *zserver = dynamic_cast<zmq_ipc_server *>(server);
    zmq_ipc_server::destroy(zserver);
}

ipc_client *
ipc_client::factory(uint32_t recipient) {
    return zmq_ipc_client_async::factory(recipient);
}

void
ipc_client::destroy(ipc_client *client) {
    zmq_ipc_client_async *zclient =
        dynamic_cast<zmq_ipc_client_async *>(client);
    zmq_ipc_client_async::destroy(zclient);
}

} // namespace ipc
} // namespace sdk
