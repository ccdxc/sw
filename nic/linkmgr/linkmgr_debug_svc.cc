// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "linkmgr_debug_svc.hpp"
#include "linkmgr_src.hpp"
#include "linkmgr_debug.hpp"

namespace linkmgr {

Status
DebugServiceImpl::GenericOpn(ServerContext *context,
                             const GenericOpnRequestMsg *req_msg,
                             GenericOpnResponseMsg *rsp_msg)
{
    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::GenericOpnRequest  req  = req_msg->request(i);
        debug::GenericOpnResponse *rsp = rsp_msg->add_response();
        linkmgr::linkmgr_generic_debug_opn(req, rsp);
        rsp->set_api_status(types::API_STATUS_OK);
    }

    return Status::OK;
}

} // namespace linkmgr
