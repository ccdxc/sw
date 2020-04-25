// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "gen/proto/types.pb.h"

sdk_ret_t
handle_svc_req (int fd, types::ServiceRequestMessage *proto_req, int cmd_fd)
{
    return SDK_RET_OK;
}
