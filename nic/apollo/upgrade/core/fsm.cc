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
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"
#include "nic/apollo/upgrade/core/utils.hpp"
#include "nic/apollo/upgrade/core/fsm.hpp"
#include "nic/apollo/upgrade/core/gen/upg_fsm.hpp"

namespace upg {

static transition_t
to_transition (stage_id_t stage_id)
{
    transition_t transitions;
    for (stage_transition_t& stage: stage_transitions) {
        if (stage_id == stage.from()){
            transitions.push_back(stage);
        }
    }
    return transitions;
}

static void
init_stage_id_name_mapping (void)
{
    fsm_stagename_to_id["compatibility_check"] = STAGE_ID_COMPAT_CHECK;
    fsm_stagename_to_id["start"]      = STAGE_ID_START;
    fsm_stagename_to_id["prepare"]    = STAGE_ID_PREPARE;
    fsm_stagename_to_id["backup"]     = STAGE_ID_BACKUP;
    fsm_stagename_to_id["switchover"]    = STAGE_ID_SWITCHOVER;
    fsm_stagename_to_id["verify"]     = STAGE_ID_VERIFY;
    fsm_stagename_to_id["finish"]     = STAGE_ID_FINISH;
    fsm_stagename_to_id["abort"]      = STAGE_ID_ABORT;
    fsm_stagename_to_id["rollback"]   = STAGE_ID_ROLLBACK;
    fsm_stagename_to_id["critical"]   = STAGE_ID_CRITICAL;
    fsm_stagename_to_id["exit"]       = STAGE_ID_EXIT;

    fsm_stageid_to_name[STAGE_ID_COMPAT_CHECK] = "compatibility_check";
    fsm_stageid_to_name[STAGE_ID_START] = "start";
    fsm_stageid_to_name[STAGE_ID_PREPARE] = "prepare";
    fsm_stageid_to_name[STAGE_ID_BACKUP] = "backup";
    fsm_stageid_to_name[STAGE_ID_SWITCHOVER] = "switchover";
    fsm_stageid_to_name[STAGE_ID_VERIFY] = "verify";
    fsm_stageid_to_name[STAGE_ID_FINISH] = "finish";
    fsm_stageid_to_name[STAGE_ID_ABORT] = "abort";
    fsm_stageid_to_name[STAGE_ID_ROLLBACK] = "rollback";
    fsm_stageid_to_name[STAGE_ID_CRITICAL] = "critical";
    fsm_stageid_to_name[STAGE_ID_EXIT] = "exit";
}

static void
init_svc (void)
{
    fsm_services.set_event_sequence(event_sequence);
    for (svc_t& service: svc) {
        fsm_services.add_svc(service);
    }
}

static void
init_lookup_table (void)
{
    for (stage_transition_t& stage: stage_transitions) {
        std::string key;
        key = std::to_string(stage.from()) +
            std::to_string(stage.svc_rsp_code());
        fsm_lookup_tbl[key] = stage.to();
    }
}

static void
init_fsm_stages (void)
{
    stage_id_t          stage_id;
    time_t              svc_rsp_timeout;
    svc_sequence_t      svcs_seq;
    event_sequence_t    event_seq;
    transition_t        transitions;
    scripts_t           prehook;
    scripts_t           posthook;

    // fsm_stages is ordered by stage_id_t
    for (idl_stage_t& stage : idl_stages_cfg) {
        stage_id    = name_to_stage_id(stage.stage());
        svc_rsp_timeout = str_to_timeout(stage.svc_rsp_timeout());
        svcs_seq    = str_to_svc_sequence(stage.svc_sequence());
        event_seq   = str_to_event_sequence(stage.event_sequence());
        transitions = to_transition(stage_id);
        prehook     = str_to_scripts(stage.pre_hook_scripts());
        posthook    = str_to_scripts(stage.post_hook_scripts());

        fsm_stages[stage_id] =  stage_t(svc_rsp_timeout, svcs_seq,
                                      event_seq, transitions,
                                      prehook,posthook);
    }
}

static void
init (void)
{
    init_stage_id_name_mapping();
    init_svc();
    init_lookup_table();
    init_fsm_stages();
}

static svc_rsp_code_t
dispatch_event (stage_id_t id)
{
    // TODO: This will call service specific dispatch
    //   -   we need to implement another lib for external
    //   delivery of those events,grpc, skd ipc, etc

    std::string stage_name;
    stage_name = id_to_stage_name(id);

    printf("\n %s : SVC_RSP -> ", stage_name.c_str());
    uint32_t x;
    scanf("%d", &x);

    // can checkc internal datastructrure and verify
    // if it needs to send async or sync notification
    // if sync , what is there in the order list
    // if svc order list is empty get svc prder from
    // global list

    return (svc_rsp_code_t)x;
}

static stage_id_t
lookup_stage_transition (stage_id_t cur, svc_rsp_code_t rsp)
{
    stage_id_t next_stage = STAGE_ID_EXIT ;
    next_stage = fsm_lookup_tbl[std::to_string(cur) +std::to_string(rsp)];
    return next_stage;
}

static sdk_ret_t
register_callback_inernal (stage_callback_t cb_type, stage_id_t cb_stage,
                   void *(*callback)(void *), void *arg)
{
    // TODO:
    //
    sdk_ret_t ret;

    ret = SDK_RET_OK;
    return ret;
}

static sdk_ret_t
run_fsm (stage_id_t start_stage, stage_id_t end_stage) {

    sdk_ret_t ret           = SDK_RET_OK;
    stage_id_t cur_stage    = start_stage;
    svc_rsp_code_t rc;
    //sdk_ret_t (* send_notification)(stage_id_t);
    (void)init();

    while (cur_stage != end_stage) {
        // call pre hooks
        // call pre call backs
        rc = (svc_rsp_code_t) dispatch_event(cur_stage);
        // call post hooks
        // call post call backs
        cur_stage = lookup_stage_transition(cur_stage, rc);
    }
    return ret;
}

sdk_ret_t
register_callback (stage_callback_t cb_type, stage_id_t cb_stage,
                   void *(*callback)(void *), void *arg)
{
    // TODO:
    //
    return register_callback_inernal(cb_type, cb_stage, callback, arg);
}

sdk_ret_t
do_switchover (void)
{
    return run_fsm(STAGE_ID_COMPAT_CHECK, STAGE_ID_EXIT);
}
}
