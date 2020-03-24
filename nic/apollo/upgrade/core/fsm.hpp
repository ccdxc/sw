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
#include <ev.h>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "platform/pal/include/pal.h"
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

class fsm {
    public:
       fsm(stage_id_t start = STAGE_ID_COMPAT_CHECK,
                 stage_id_t end = STAGE_ID_EXIT) {
            current_stage_    = start;
            start_stage_      = start;
            end_stage_        = end;
            pending_response_ = 0;
            size_             = 0;
            timeout_          = DEFAULT_SVC_RSP_TIMEOUT;
        }

        ~fsm(void) {};

        stage_id_t current_stage(void) const {
            return current_stage_;
        }

        stage_id_t start_stage(void) const {
            return start_stage_;
        }

        stage_id_t end_stage(void) const {
            return end_stage_;
        }

        uint32_t pending_response(void) const {
            return pending_response_;
        }

        void set_pending_response(const uint32_t count) {
            pending_response_ = count;
        }

        void set_timeout(const ev_tstamp timeout) {
            timeout_ = timeout;
        }

        ev_tstamp timeout(void) const {
            return timeout_;
        }

        svc_sequence_t svc_sequence(void) const {
            return svc_sequence_;
        }

        bool has_next_svc(void) const {
            return pending_response_ > 0;
        }

        void            set_current_stage(const stage_id_t id);
        void            update_stage_progress(const svc_rsp_code_t rsp);
        bool            is_current_stage_over(void);
        bool            is_serial_event_sequence(void) const;
        bool            is_valid_service(const std::string svc) const;
        svc_t           next_svc(void) const;
        void            timer_init(const void* ctxt);
        void            timer_start(void);
        void            timer_stop(void);
        void            timer_set(void);

    private:
        stage_id_t      current_stage_;
        stage_id_t      start_stage_;
        stage_id_t      end_stage_;
        uint32_t        pending_response_;
        uint32_t        size_;
        svc_sequence_t  svc_sequence_;
        ev_tstamp       timeout_;
};

void init(void *ctxt);
sdk_ret_t do_switchover(void);
sdk_ret_t register_callback(stage_callback_t cb_type, stage_id_t cb_stage,
                            void *(*callback)(void *), void *arg);

}
#endif    //  __UPG_FSM_HPP__
