// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------


#include <iostream>
#include <ev.h>
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
std::string
svc_sequence_to_str (const svc_sequence_list svcs)
{
    std::string str = "svc_sequence_list : ";
    for (auto x: svcs) {
        str += x + "," ;
    }
    return str;
}

std::string
transition_to_str (const transition_stages& transitions)
{
    std::string str = "(from:rsp:to) :"  ;
    for (auto x: transitions) {
        str += "( " + std::string(upg_stage2str(x.from()));
        str += " : "  + std::to_string(x.svc_rsp_code());
        str += " : "  + std::string(upg_stage2str(x.to())) + "),";
    }

    return str;
}

std::string
script_to_str(const upg_scripts& scripts)
{
    std::string str;
    for (auto x: scripts) {
        str += x.path() + ", ";
    }
    return str;
}

void
dump(const fsm& fsm)
{
    std::string str;
    upg_stage_t start = fsm.start_stage();
    upg_stage_t curr  = fsm.current_stage();
    upg_stage_t end   = fsm.end_stage();

    UPG_TRACE_VERBOSE("Runtime state : \n"
                      "\tstart_stage   : %s\n"
                      "\tcurrent_stage : %s\n"
                      "\tend_stage     : %s\n"
                      "\tPending resp  : %d\n"
                      "\ttimeout       : %f\n"
                      "\tsvc sequence  : %s\n",
                      upg_stage2str(start),
                      upg_stage2str(curr),
                      upg_stage2str(end),
                      fsm.pending_response(),
                      fsm.timeout(),
                      svc_sequence_to_str(fsm.svc_sequence()).c_str());
}

void
dump (const transition_stages& transitions)
{
    std::string str ;
    for (auto x: transitions) {
        str += "(from:" + std::string(upg_stage2str(x.from()));
        str += " rsp:"  + std::to_string(x.svc_rsp_code());
        str += " to: "  + std::string(upg_stage2str(x.to()))+ "),";
    }

    UPG_TRACE_VERBOSE("%s", str.c_str());
}

void
dump (const upg_scripts& scripts)
{
    std::string str;
    for (auto x: scripts) {
        str += x.path() + ", ";
    }

    UPG_TRACE_VERBOSE("%s", str.c_str());
}

    void
dump (upg_stage& stage)
{
    UPG_TRACE_VERBOSE("\t upg_stage :"
                      "\n\t\t svc_rsp_timeout: %f"
                      "\n\t\t svc_sequence   : %s"
                      "\n\t\t event_sequence : %d"
                      "\n\t\t pre_hook       : %s"
                      "\n\t\t post_hook      : %s",
                      stage.svc_rsp_timeout(),
                      svc_sequence_to_str(stage.svc_sequence()).c_str(),
                      stage.event_sequence(),
                      script_to_str(stage.pre_hook_scripts()).c_str(),
                      script_to_str(stage.post_hook_scripts()).c_str());
}

void
dump (upg_stages_map& stages)
{
    std::string str = "upg_stages_map : ";
    UPG_TRACE_VERBOSE("%s", str.c_str());
    for (auto x: stages) {
        dump(x.second);
    }
}

void
dump (const upg_svc_map& svcs)
{
    std::string str = "services : ";
    for (std::pair<std::string, upg_svc> element : svcs) {
        upg_svc x = element.second;
        str += x.name() + "(ipd_id :";
        str += std::to_string(x.ipc_id()) + "), "  ;
    }
    UPG_TRACE_VERBOSE("%s", str.c_str());
}

void
dump (const svc_sequence_list& svcs)
{
    std::string str = "svc_sequence_list : ";
    for (auto x: svcs) {
        str += x + ", ";
    }
    UPG_TRACE_VERBOSE("%s", str.c_str());
}

void
dump (const stage_map& tran)
{
    std::string str = "stage_map : ";
    for (std::pair<std::string, upg_stage_t> element : tran) {
        str += "[ " + element.first;
        str += " ->" + std::to_string(element.second) + "], ";
    }
    UPG_TRACE_VERBOSE("%s", str.c_str());
}

void
token_parse (std::string& text, std::vector<std::string>& results)
{
    boost::split(results, text, [](char c){return c == ':';});
};

ev_tstamp
str_to_timeout (const std::string& timeout)
{
    if (timeout.empty()) {
        return (ev_tstamp) DEFAULT_SVC_RSP_TIMEOUT;
    }
    ev_tstamp svc_rsp_timeout = std::stoi(timeout);
    return svc_rsp_timeout;
}

upg_stage_t
name_to_stage_id (const std::string stage)
{
   upg_stage_t id;
   bool found = false;

    for (uint32_t i = UPG_STAGE_COMPAT_CHECK; i < UPG_STAGE_MAX; i++) {
        if(!strcmp (upg_stage2str((upg_stage_t)i), stage.c_str())) {
            id    = (upg_stage_t) i;
            found = true;
            break;
        }
    }

    SDK_ASSERT(found != false);
    return id;
};

std::string
id_to_stage_name (const upg_stage_t stage)
{
   std::string name;
   bool found = false;

   for (uint32_t i = UPG_STAGE_COMPAT_CHECK; i < UPG_STAGE_MAX; i++) {
       if(stage == (upg_stage_t)i) {
           name = std::string(upg_stage2str((upg_stage_t) i));
           found = true;
           break;
       }
   }

   SDK_ASSERT(found != false);
   return name;
};

upg_scripts
str_to_scripts (std::string scripts)
{
    upg_scripts script_list;
    if (scripts.empty()) {
        return script_list;
    }
    std::vector<std::string> filenames;
    token_parse(scripts, filenames);
    for (std::string& name: filenames){
        script_list.push_back(upg_script(name));
    }
    return script_list;
};


svc_rsp_code_t
svc_rsp_code (const upg_status_t id)
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
}
