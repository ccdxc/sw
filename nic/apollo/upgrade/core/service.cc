
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
///  Implementation of upgrade classes and methods
///
//----------------------------------------------------------------------------


#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/ipc/ipc.hpp"

namespace upg {

upg_svc& upg_svc::operator = (const upg_svc &obj) {
    name_        = obj.name_;
    ipc_id_      = obj.ipc_id_;
    return *this;
};

void upg_svc::dispatch_event(ipc::ipc_svc_dom_id_t dom,
                             upg_stage_t event) const {
    ipc::request(dom, event, ipc_id_, NULL);
};
}
