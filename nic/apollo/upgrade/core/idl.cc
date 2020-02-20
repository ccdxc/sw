
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
#include "nic/apollo/upgrade/core/idl.hpp"

namespace upg {
    idl_stage_t::idl_stage_t(void) {
        stage_             = "";
        svc_rsp_timeout_       = "";
        svc_sequence_      = "";
        event_sequence_    = "";
        pre_hook_scripts_  = "";
        post_hook_scripts_ = "";
    };

    idl_stage_t::idl_stage_t( std::string stage, std::string timeout,
                              std::string svcs, std::string evt_seq,
                              std::string pre_hooks,
                              std::string post_hooks) {
        stage_             = stage;
        svc_rsp_timeout_       = timeout ;
        svc_sequence_      = svcs ;
        event_sequence_    = evt_seq ;
        pre_hook_scripts_  = pre_hooks ;
        post_hook_scripts_ = post_hooks ;
    };

    idl_stage_t::~idl_stage_t(void) {};

    std::string& idl_stage_t::stage(void) {
        return stage_;
    };

    std::string& idl_stage_t::svc_rsp_timeout(void) {
        return svc_rsp_timeout_;
    };

    std::string& idl_stage_t::svc_sequence(void) {
        return svc_sequence_;
    };

    std::string& idl_stage_t::event_sequence(void) {
        return event_sequence_;
    };

    std::string& idl_stage_t::pre_hook_scripts(void) {
        return pre_hook_scripts_;
    };

    std::string& idl_stage_t::post_hook_scripts(void) {
        return post_hook_scripts_;
    };
}
