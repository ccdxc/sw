
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
#include "nic/apollo/upgrade/include/pds_upgmgr.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

namespace upg {
    stage_transition_t::stage_transition_t(void):
        form_(STAGE_ID_EXIT),
        svc_rsp_code_(SVC_RSP_CRIT)
        ,to_(STAGE_ID_EXIT){};

    stage_transition_t::stage_transition_t(stage_id_t curr,
                                           svc_rsp_code_t rsp,
                                           stage_id_t next) {
        form_ = curr;
        svc_rsp_code_ = rsp;
        to_ = next;
    };

    stage_transition_t::~stage_transition_t(void) {};

    stage_id_t stage_transition_t::from(void) {
        return form_;
    };

    void stage_transition_t::set_from(stage_id_t id) {
        form_ = id;
    };

    stage_id_t stage_transition_t::to(void) {
        return to_;
    };

    void stage_transition_t::set_to(stage_id_t id) {
        to_ = id;
    };

    svc_rsp_code_t stage_transition_t::svc_rsp_code(void) {
        return svc_rsp_code_;
    };

    void stage_transition_t::set_svc_rsp_code(svc_rsp_code_t id) {
        svc_rsp_code_ = id;
    };

    script_t::script_t(std::string path) {
        path_ = path;
    };
    script_t::~script_t(void){};

    stage_t::stage_t(void) {
        svc_rsp_timeout_       = DEFAULT_RSP_TIMEOUT ;
        event_sequence_    = PARALLEL;
    };

    stage_t::stage_t(time_t svc_rsp_timeout, svc_sequence_t svc_seq,
                     event_sequence_t evt_seq,
                     transition_t transitions, scripts_t pre_sc,
                     scripts_t post_sc) {
        svc_rsp_timeout_       = svc_rsp_timeout ;
        svc_sequence_      = svc_seq;
        event_sequence_    = evt_seq;
        transitions_       = transitions;
        pre_hook_scripts_  = pre_sc ;
        post_hook_scripts_ = post_sc;
    };

    stage_t::~stage_t(void) {};

    time_t& stage_t::svc_rsp_timeout(void) {
        return svc_rsp_timeout_;
    };

    svc_sequence_t& stage_t::svc_sequence(void) {
        return svc_sequence_;
    };

    event_sequence_t& stage_t::event_sequence(void) {
        return event_sequence_;
    };

    transition_t& stage_t::transitions(void) {
        return transitions_;
    };

    scripts_t& stage_t::pre_hook_scripts(void) {
        return pre_hook_scripts_;
    };

    scripts_t& stage_t::post_hook_scripts(void) {
        return post_hook_scripts_;
    };
}
