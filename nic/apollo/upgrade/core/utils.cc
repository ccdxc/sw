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
#include "nic/apollo/upgrade/core/fsm.hpp"
#include "nic/apollo/upgrade/core/utils.hpp"

namespace upg
{

void
token_parse (std::string& text, std::vector<std::string>& results)
{
    boost::split(results, text, [](char c){return c == ':';});
};

time_t
str_to_timeout (std::string& timeout)
{
    if (timeout.empty()) {
        return (time_t) DEFAULT_RSP_TIMEOUT;
    }
    // TODO:
    //  if timeout regex doesnt match return Default too

    time_t svc_rsp_timeout = std::stoi(timeout);
    return svc_rsp_timeout;
}

svc_sequence_t
str_to_svc_sequence (std::string& svc_seq)
{
    if (svc_seq.empty()) {
        return fsm_services.svc_sequence();
    }
    std::vector<std::string> svcs;
    token_parse(svc_seq, svcs);
    svc_sequence_t svcs_sequence;
    for (std::string& name:svcs){
        svcs_sequence.push_back(svc_t(name));
    }
    return svcs_sequence;
};

event_sequence_t
str_to_event_sequence (std::string& evt_seq)
{
    event_sequence_t seq=PARALLEL;
    if (evt_seq.empty()) {
        seq = fsm_services.event_sequence();
    } else if (boost::iequals(evt_seq, "parallel")) {
        seq =  PARALLEL;
    } else if (boost::iequals(evt_seq, "serial")) {
        seq = SERIAL;
    } else {
        // erro
    }
    // TODO: more validation
    return seq;
};

stage_id_t
name_to_stage_id (std::string stage)
{
    // TODO: empty error
    stage_id_t id = STAGE_ID_COMPAT_CHECK;
    boost::algorithm::to_lower(stage);
    auto found = fsm_stagename_to_id.find(stage);
    if (found != fsm_stagename_to_id.end()) {
        id = fsm_stagename_to_id[stage];
    } else {
        fprintf (stderr, "TODO: error!\n");
    }
    return id;
};

std::string
id_to_stage_name (stage_id_t stage)
{
    // TODO: empty error
    std::string name;
    auto found = fsm_stageid_to_name.find(stage);
    if (found != fsm_stageid_to_name.end()) {
        name = fsm_stageid_to_name[stage];
    } else {
        fprintf (stderr, "TODO: error!\n");
    }
    return name;
};

scripts_t
str_to_scripts (std::string scripts)
{
    scripts_t script_list;
    if (scripts.empty()) {
        return script_list;
    }
    std::vector<std::string> filenames;
    token_parse(scripts, filenames);
    for (std::string& name: filenames){
        script_list.push_back(script_t(name));
    }
    return script_list;
};

#if 0
    //static sdk_ret_t (* send_event)(stage_id_t);
    static sdk_ret_t (* pre_hook_callbacks[STAGE_COUNT])(void) = { NULL };
    static sdk_ret_t (* post_hook_callbacks[STAGE_COUNT])(void) = { NULL };
#endif

#if 0
// NOTE:
// ASSUMPTIONS:
//  - It is a valid script and reviewd internally.
//  - This script hasn't been compromised with malicious code.
//  - Application using  this upgrade code lib doesnt have
//    set-user-ID or set-group-ID privilege.
//  - System() is a cross-platform call, so the script it is
//    executing needs to be tested on the target before.
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

}
