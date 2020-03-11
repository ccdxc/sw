// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------

#ifndef __UPG_FSM_HPP__
#define __UPG_FSM_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"
#include "platform/evutils/include/evutils.h"
#include "platform/pal/include/pal.h"
#include "nic/sdk/platform/pal/include/pal.h"

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

class fsm {
    public:
        fsm(stage_id_t start = STAGE_ID_COMPAT_CHECK,
                    stage_id_t end = STAGE_ID_EXIT);
        ~fsm(void) ;
        stage_id_t current_stage(void);
        void       set_current_stage(stage_id_t id);
        stage_id_t start_stage(void);
        stage_id_t end_stage(void);
        uint32_t   pending_response(void);
        void       set_pending_response(uint32_t count);
        void       update_stage_progress(svc_rsp_code_t rsp);
        void       set_timeout(ev_tstamp timeout);
        bool       is_current_stage_over(void);
        bool       is_serial_event_sequence(void);
        void       start_timer(void);
        void       stop_timer(void);
        ev_tstamp  timeout(void);
    private:
        stage_id_t   current_stage_;
        stage_id_t   start_stage_;
        stage_id_t   end_stage_;
        uint32_t     pending_response_;
        ev_tstamp    timeout_;
        EV_P;
};

void init(void);
static fsm fsm_states;

sdk_ret_t do_switchover(void);
sdk_ret_t register_callback(stage_callback_t cb_type, stage_id_t cb_stage,
                                void *(*callback)(void *), void *arg);

}
#endif    //  __UPG_FSM_HPP__
