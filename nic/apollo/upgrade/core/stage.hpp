// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structures
///
//----------------------------------------------------------------------------

#ifndef __UPGRADE_FSM_STAGE_HPP__
#define __UPGRADE_FSM_STAGE_HPP__

#include <string>
#include <ev.h>
#include <boost/unordered_map.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/assign/list_of.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

#define DEFAULT_SVC_RSP_TIMEOUT 5

namespace upg {

/// \brief function call back type
/// \remark
/// Type of the registered callback , whether to call the address
/// before executing the stage or after executing the stage.
typedef enum stage_callback_t {
    PRE_STAGE  = 0,
    POST_STAGE = 1
} stage_callback_t;

/// \brief  smallest data structure to decide what would be the next
/// stage
/// \remark
///  stage transition happens based on the current stage and responses
///  from all the services. This data structure helps building lookup
///  table for stage transition.
class upg_stage_transition {
public:
    upg_stage_transition(void):
        from_(UPG_STAGE_EXIT),
        svc_rsp_code_(SVC_RSP_CRIT)
        ,to_(UPG_STAGE_EXIT){};

    upg_stage_transition(upg_stage_t curr,
                       svc_rsp_code_t rsp,
                       upg_stage_t next) {
        from_ = curr;
        svc_rsp_code_ = rsp;
        to_ = next;
    };

    ~upg_stage_transition(void) {};

    upg_stage_t from(void) const {
        return from_;
    };

    void set_from(upg_stage_t id) {
        from_ = id;
    };

    upg_stage_t to(void) const {
        return to_;
    };

    void set_to(upg_stage_t id) {
        to_ = id;
    };

    svc_rsp_code_t svc_rsp_code(void) const {
        return svc_rsp_code_;
    };

    void set_svc_rsp_code(svc_rsp_code_t id) {
        svc_rsp_code_ = id;
    };

private:
    upg_stage_t     from_;
    svc_rsp_code_t  svc_rsp_code_;
    upg_stage_t     to_;
};

/// \brief a container for a list of stages
// typedef class upg_stage_transition upg_stage_transition;
typedef boost::container::vector<upg_stage_transition>  transition_stages;
/// \brief A lookup table
/// \remark
/// A hash map to identify the next stage based on current stage id and
/// the response from the stage. current stage id and response is
/// converted to string and that is used as a key to find the next stage
/// id.
typedef boost::unordered_map<std::string, upg_stage_t> stage_map;

/// \brief    A path to a script name.
/// \remark
/// an absolute/relative path of a filename in the target system.
/// This file may be instructed to be called either pre hooks or a post
/// hook of any stage.
class upg_script {
public:
    upg_script(std::string path) {
        path_ = path;
    };

    ~upg_script(void){};

    std::string path(void) const {
        return path_;
    };

private:
    std::string path_;
};

typedef boost::container::vector<upg_script> upg_scripts;

/// \brief  The container of all internal upgrade data structures
/// \remark
/// A single source of truth to decide what has to be done in any stage
/// . This holds the response timeout, event sequence order, service
/// order, pre hooks, post hooks, also pre and post function callback
/// to the application.
class upg_stage {
public:
    upg_stage(void) {
        svc_rsp_timeout_     = DEFAULT_SVC_RSP_TIMEOUT ;
        svc_event_sequence_  = PARALLEL;
    };

    upg_stage(ev_tstamp svc_rsp_timeout, svc_sequence_list svc_seq,
              event_sequence_t evt_seq,
              upg_scripts pre_sc, upg_scripts post_sc) {
        svc_rsp_timeout_    = svc_rsp_timeout ;
        svc_sequence_       = svc_seq;
        svc_event_sequence_ = evt_seq;
        pre_hook_scripts_   = pre_sc ;
        post_hook_scripts_  = post_sc;
    };

    ~upg_stage(void) {};

    ev_tstamp& svc_rsp_timeout(void) {
        return svc_rsp_timeout_;
    };

    svc_sequence_list& svc_sequence(void) {
        return svc_sequence_;
    };

    event_sequence_t& event_sequence(void) {
        return svc_event_sequence_;
    };

    upg_scripts& pre_hook_scripts(void) {
        return pre_hook_scripts_;
    };

    upg_scripts& post_hook_scripts(void) {
        return post_hook_scripts_;
    };

private:
    ev_tstamp          svc_rsp_timeout_;
    svc_sequence_list  svc_sequence_;
    event_sequence_t   svc_event_sequence_;
    upg_scripts        pre_hook_scripts_;
    upg_scripts        post_hook_scripts_;
};

/// \brief A lookup container for stage id and object
typedef boost::unordered_map<upg_stage_t, upg_stage> upg_stages_map;

}
#endif    // __UPGRADE_FSM_STAGE_HPP___

