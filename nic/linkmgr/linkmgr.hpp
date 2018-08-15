// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_HPP__
#define __LINKMGR_HPP__

#include "sdk/linkmgr.hpp"
#include "nic/gen/proto/hal/debug.grpc.pb.h"

using debug::GenericOpnRequest;
using debug::GenericOpnResponse;

namespace linkmgr {

hal_ret_t linkmgr_init(sdk::linkmgr::linkmgr_cfg_t *sdk_cfg);

hal_ret_t
linkmgr_generic_debug_opn(GenericOpnRequest& req,
                          GenericOpnResponse *resp);

}    // namespace linkmgr

#endif    // __LINKMGR_HPP__

