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
#include "nic/apollo/upgrade/core/fsm.hpp"
#include "nic/apollo/upgrade/core/utils.hpp"
#include "nic/apollo/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg
{

 // Overloaded dump functions can be used for debugging
void
dump(fsm& fsm)
{
    std::string str;
    stage_id_t start = fsm.start_stage();
    stage_id_t curr  = fsm.current_stage();
    stage_id_t end   = fsm.end_stage();

    str =  "Runtime state : \n";
    str += "\tstart_stage   : " + id_to_stage_name(start);
    str += " -> " + std::string(upg_stage_name[upg_stage_id(start)]) + "\n";
    str += "\tcurrent_stage : " + id_to_stage_name(curr);
    str += " -> " + std::string(upg_stage_name[upg_stage_id(curr)]) + "\n";
    str += "\tend_stage     : " + id_to_stage_name(end);
    str += " -> " + std::string(upg_stage_name[upg_stage_id(end)]) + "\n";
    str += "\tPending resp  : " + std::to_string(fsm.pending_response()) + "\n";
    str += "\ttimeout       : " + std::to_string(fsm.timeout());

    UPG_TRACE_PRINT(str);
}

void
dump (transition_t transitions)
{
    std::string str ;
    for (auto x: transitions) {
        str += "(from:" + id_to_stage_name(x.from());
        str += " rsp:"  + x.svc_rsp_code();
        str += " to: "  + id_to_stage_name(x.to())+ "),";
    }

    UPG_TRACE_PRINT(str);
}

void
dump (scripts_t scripts)
{
    std::string str;
    for (auto x: scripts) {
        str += x.path() + ", ";
    }

    UPG_TRACE_PRINT(str);
}

void
dump (stage_t stage)
{
    std::string str;
    str = "\t stage_t : ";
    str += " \t\t svc_rsp_timeout : ["+ std::to_string(stage.svc_rsp_timeout());
    str += " ] \n";
    str += " \t\t svc_sequence    : " ;
    dump(stage.svc_sequence());
    str += " \t\t event_sequence  : [" + std::to_string(stage.event_sequence());
    str += " ]\n";
    str += " \t\t transitions     : [" ;
    dump(stage.transitions());
    str += " ]\n";
    str += " \t\t pre_hook        : [";
    dump(stage.pre_hook_scripts());
    str += " ]\n";
    str += " \t\t post_hook       : [";
    dump(stage.post_hook_scripts());
    str += " ]\n";

    UPG_TRACE_PRINT(str);
}

void
dump (stages_t stages)
{
    std::string str = "stages_t : ";
    for (auto x: stages){
        dump(x.second);
    }
    UPG_TRACE_PRINT(str);
}

void
dump (services_t svcs)
{
    std::string str = "services_t : ";
    svc_sequence_t svc_list = svcs.svc_sequence();
    for (auto x: svc_list) {
         str += x.name() + ", ";
    }
    UPG_TRACE_PRINT(str);
    UPG_TRACE_PRINT("[" + std::to_string(svcs.event_sequence()) + "]");
}

void
dump (svc_sequence_t svcs)
{
    std::string str = "svc_sequence_t : ";
    svc_sequence_t svc_list = svcs;
    for (auto x: svc_list) {
         str += x.name() + ", ";
    }
    UPG_TRACE_PRINT(str);
}

void
dump (stage_map_t tran)
{
    std::string str = "stage_map_t : ";
    for (std::pair<std::string, stage_id_t> element : tran) {
         str += "[ " + element.first;
         str += " ->" + std::to_string(element.second) + "], ";
    }
    UPG_TRACE_PRINT(str);
}

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

stage_id_t
name_to_stage_id (std::string stage)
{
    // TODO: empty error
   stage_id_t id;
   bool found = false;

    for (uint32_t i = STAGE_ID_COMPAT_CHECK; i<= STAGE_ID_EXIT; i++) {
        if(!strcmp (fsm_stageid_to_name[(stage_id_t)i], stage.c_str())) {
            id    = (stage_id_t) i;
            found = true;
            break;
        }
    }

    SDK_ASSERT(found != false);
    return id;
};

std::string
id_to_stage_name (stage_id_t stage)
{
    // TODO: empty error
   std::string name;
   bool found = false;

   for (uint32_t i = STAGE_ID_COMPAT_CHECK; i<= STAGE_ID_EXIT; i++) {
       if(stage == (stage_id_t)i) {
           name = std::string(fsm_stageid_to_name[(stage_id_t) i]);
           found = true;
           break;
       }
   }

   SDK_ASSERT(found != false);
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


svc_rsp_code_t
svc_rsp_code (upg_status_t id)
{
    svc_rsp_code_t svc_rsp_id = SVC_RSP_NONE;
    switch (id) {
    case UPG_STATUS_OK:
        svc_rsp_id = SVC_RSP_OK;
        break;
    case UPG_STATUS_FAIL:
        svc_rsp_id = SVC_RSP_FAIL;
        break;
    case UPG_STATUS_CRITICAL:
        svc_rsp_id = SVC_RSP_CRIT;
        break;
    default:
        svc_rsp_id = SVC_RSP_NONE;
        break;
    }
    return svc_rsp_id;
};

upg_stage_t
upg_stage_id (stage_id_t id)
{

    upg_stage_t upg_stage_id = UPG_STAGE_NONE;

    switch(id) {
    case STAGE_ID_COMPAT_CHECK:
        upg_stage_id = UPG_STAGE_COMPAT_CHECK;
        break;
    case STAGE_ID_START:
        upg_stage_id = UPG_STAGE_START;
        break;
    case STAGE_ID_BACKUP :
    case STAGE_ID_PREPARE:
        upg_stage_id = UPG_STAGE_PREPARE;
        break;
    case STAGE_ID_SWITCHOVER:
        upg_stage_id = UPG_STAGE_SWITCHOVER;
        break;
    case STAGE_ID_FINISH:
    case STAGE_ID_VERIFY:
        upg_stage_id = UPG_STAGE_INIT;
        break;
    case STAGE_ID_ABORT:
        upg_stage_id = UPG_STAGE_ABORT;
        break;
    case STAGE_ID_ROLLBACK:
        upg_stage_id = UPG_STAGE_ROLLBACK;
        break;
    case STAGE_ID_CRITICAL:
    case STAGE_ID_EXIT:
        upg_stage_id = UPG_STAGE_EXIT;
        break;
    default:
        upg_stage_id = UPG_STAGE_NONE;
        break;
    }
    return upg_stage_id;
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
