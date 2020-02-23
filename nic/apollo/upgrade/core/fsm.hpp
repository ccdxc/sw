// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------


#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/include/pds_upgmgr.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

namespace upg {
#if 0
    //static sdk_ret_t (* send_event)(stage_id_t);
    static sdk_ret_t (* pre_hook_callbacks[STAGE_COUNT])(void) = { NULL };
    static sdk_ret_t (* post_hook_callbacks[STAGE_COUNT])(void) = { NULL };
#endif

// NOTE:
// ASSUMPTIONS:
//  - It is a valid script and reviewd internally.
//  - This script hasn't been compromised with malicious code.
//  - Application using  this upgrade code lib doesnt have
//    set-user-ID or set-group-ID privilege.
//  - System() is a cross-platform call, so the script it is
//    executing needs to be tested on the target before.
#if 0
static  sdk_ret_t execute_script(/* eventid, service ep order*/) {
    SDK_RET_OK;
}

static  sdk_ret_t send_serial_event(/* eventid, service ep order*/) {
    SDK_RET_OK;
}

static  sdk_ret_t send_paralle_event(/* eventi, service ep order*/) {
    SDK_RET_OK;
}
#endif

    sdk_ret_t register_callback(stage_callback_t cb_type, stage_id_t cb_stage,
                                    void *(*callback)(void *), void *arg);

    sdk_ret_t do_switchover(void);
}
