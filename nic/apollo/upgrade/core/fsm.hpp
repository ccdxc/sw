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
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"


namespace upg {
class fsm {
    public:
        fsm(upg_stage_t start = UPG_STAGE_COMPAT_CHECK,
            upg_stage_t end   = UPG_STAGE_EXIT) {
            current_stage_    = start;
            start_stage_      = start;
            end_stage_        = end;
            pending_response_ = 0;
            size_             = 0;
            timeout_          = DEFAULT_SVC_RSP_TIMEOUT;
        }

        ~fsm(void) {};

        upg_stage_t current_stage(void) const {
            return current_stage_;
        }

        upg_stage_t start_stage(void) const {
            return start_stage_;
        }

        void set_start_stage(const upg_stage_t entry_stage) {
            start_stage_ = entry_stage;
        }

        upg_stage_t end_stage(void) const {
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

        svc_sequence_list svc_sequence(void) const {
            return svc_sequence_;
        }

        bool has_next_svc(void) const {
            return pending_response_ > 0;
        }

        void            set_current_stage(const upg_stage_t id);
        void            update_stage_progress(const svc_rsp_code_t rsp);
        bool            is_current_stage_over(void);
        bool            is_serial_event_sequence(void) const;
        bool            is_valid_service(const std::string svc) const;
        std::string     next_svc(void) const;
        void            timer_init(const void* ctxt);
        void            timer_start(void);
        void            timer_stop(void);
        void            timer_set(void);

    private:
        upg_stage_t       current_stage_;
        upg_stage_t       start_stage_;
        upg_stage_t       end_stage_;
        uint32_t          pending_response_;
        uint32_t          size_;
        svc_sequence_list svc_sequence_;
        ev_tstamp         timeout_;
};

void init(void *ctxt);
}
#endif    //  __UPG_FSM_HPP__
