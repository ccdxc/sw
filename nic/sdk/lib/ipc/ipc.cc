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
namespace lib {
namespace ipc {

ipc_client *
ipc_client::factory(uint32_t id) {
    return zmq_ipc_client::factory(id, true);
}

ipc_msg_ptr
ipc_client::send_recv_once(uint32_t recipient, const void *data, size_t data_length)
{
    return zmq_ipc_client::send_recv_once(recipient, data, data_length);
}

void
ipc_client::destroy(ipc_client *client) {
    zmq_ipc_client *zclient = dynamic_cast<zmq_ipc_client *>(client);
    zmq_ipc_client::destroy(zclient);
}

} // namespace ipc
} // namespace lib
} // namespace sdk
