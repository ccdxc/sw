//                                                                                                                                                                              
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena svc layer stubs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "gen/proto/types.pb.h"

sdk_ret_t
handle_svc_req (int fd, types::ServiceRequestMessage *proto_req, int cmd_fd)
{
    return SDK_RET_OK;
}
