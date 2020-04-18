
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
///  Implementation of upgrade classes and methods
///
//----------------------------------------------------------------------------

#include <iostream>
#include "ipc.hpp"
#include "service.hpp"

namespace sdk {
namespace upg {

upg_svc&
upg_svc::operator=(const upg_svc& obj) {
    name_ = obj.name_;
    ipc_id_ = obj.ipc_id_;
    has_valid_ipc_id_ = obj.has_valid_ipc_id_;
    return *this;
};

void
upg_svc::dispatch_event(ipc_svc_dom_id_t dom, upg_stage_t event,
                        upg_mode_t upg_mode) const {
    upg_send_request(dom, event, ipc_id_, upg_mode);
};

}    // namespace upg
}    // namespace sdk
