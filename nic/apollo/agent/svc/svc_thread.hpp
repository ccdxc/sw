//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __SVC_THREAD_HPP__
#define __SVC_THREAD_HPP__

#include "nic/apollo/api/include/pds_debug.hpp"
#include "gen/proto/types.pb.h"

#define FD_INVALID (-1)

sdk_ret_t handle_svc_req(int fd, types::ServiceRequestMessage *proto_req, int cmd_fd);

namespace core {

void svc_server_thread_init(void *ctxt);
void svc_server_thread_exit(void *ctxt);

}    // namespace core

#endif    // __SVC_THREAD_HPP__
