// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// APIs to use linkmgr as lib
//------------------------------------------------------------------------------

#ifndef __LINKMGR_DEBUG_HPP__
#define __LINKMGR_DEBUG_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/debug.grpc.pb.h"

using debug::GenericOpnRequest;
using debug::GenericOpnResponse;

namespace linkmgr {

hal_ret_t
linkmgr_generic_debug_opn(GenericOpnRequest& req,
                          GenericOpnResponse *resp);

}    // namespace linkmgr

#endif    // __LINKMGR_DEBUG_HPP__
