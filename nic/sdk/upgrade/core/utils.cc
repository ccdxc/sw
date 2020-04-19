// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation for core upgrade data structures, methods, and APIs
///
//----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <ev.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "include/sdk/base.hpp"
#include "stage.hpp"
#include "service.hpp"
#include "fsm.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "upgrade/include/upgrade.hpp"

namespace sdk {
namespace upg {

// create a map for upgrade stages for json stage comparison
SDK_DEFINE_MAP(upg_stage_t, UPG_STAGE_ENTRIES)

// Overloaded dump functions can be used for debugging
std::string
svc_sequence_to_str (const svc_sequence_list svcs)
{
    std::string str = "svc sequence list is ";
    for (auto x: svcs) {
        str += x + "," ;
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
dump (const fsm& fsm)
{
    std::string str;
    upg_stage_t start = fsm.start_stage();
    upg_stage_t curr  = fsm.current_stage();
    upg_stage_t end   = fsm.end_stage();

    UPG_TRACE_VERBOSE("Runtime state : \n"
                      "\tstart stage   : %s\n"
                      "\tcurrent stage : %s\n"
                      "\tend stage     : %s\n"
                      "\tPending resp  : %u\n"
                      "\ttimeout       : %f\n"
                      "\tevent seq     : %s\n"
                      "\tdiscovery     : %u\n"
                      "\tsvc sequence  : %s\n",
                      upg_stage2str(start),
                      upg_stage2str(curr),
                      upg_stage2str(end),
                      fsm.pending_response(),
                      fsm.timeout(),
                      (fsm.is_serial_event_sequence() ? "serial":"parallel"),
                      fsm.is_discovery(),
                      svc_sequence_to_str(fsm.svc_sequence()).c_str());
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
    UPG_TRACE_VERBOSE("\t upg stage :"
                      "\n\t\t svc rsp timeout: %f"
                      "\n\t\t svc sequence   : %s"
                      "\n\t\t event sequence : %s"
                      "\n\t\t discovery      : %u"
                      "\n\t\t pre hook       : %s"
                      "\n\t\t post hook      : %s",
                      stage.svc_rsp_timeout(),
                      svc_sequence_to_str(stage.svc_sequence()).c_str(),
                      (stage.event_sequence() == SERIAL? "serial":"parallel"),
                      stage.is_discovery(),
                      script_to_str(stage.pre_hook_scripts()).c_str(),
                      script_to_str(stage.post_hook_scripts()).c_str());
}

void
dump (upg_stages_map& stages)
{
    std::string str = "upg stages map : ";
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
        str += x.name() + "(ipd id :";
        str += std::to_string(x.ipc_id()) + "), "  ;
    }
    UPG_TRACE_VERBOSE("%s", str.c_str());
}

void
dump (const svc_sequence_list& svcs)
{
    std::string str = "svc sequence list : ";
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
    try
    {
        return UPG_STAGE_ENTRIES_map.at(stage.c_str());
    }
    catch (std::exception const& ex)
    {
        UPG_TRACE_VERBOSE("Stage %s doesn't exist\n", stage.c_str());
        UPG_TRACE_ERR("Stage %s doesn't exist", stage.c_str());
        SDK_ASSERT(0);
    }
};

std::string
id_to_stage_name (const upg_stage_t stage)
{
    std::map<std::string, upg_stage_t>::iterator it =
        UPG_STAGE_ENTRIES_map.begin();

    while (it != UPG_STAGE_ENTRIES_map.end()) {
        if (it->second == stage) {
            return it->first;
        }
        it++;
    }
    UPG_TRACE_VERBOSE("Stage %u doesn't exist\n", stage);
    UPG_TRACE_ERR("Stage %u doesn't exist", stage);
    SDK_ASSERT(0);
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
svc_rsp_code_name_to_id (std::string name) {
    svc_rsp_code_t code;
    bool found = false;

    for (uint32_t i = UPG_STATUS_OK; i < SVC_RSP_MAX; i++) {
        if(!strcmp (svc_rsp_code_name[(svc_rsp_code_t)i], name.c_str())) {
            code    = (svc_rsp_code_t) i;
            found = true;
            break;
        }
    }

    SDK_ASSERT(found != false);
    return code;
}

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

static std::string
get_script_path (std::string script)
{
    std::string file_path;
    try {
        std::string conf_path = std::string(std::getenv("CONFIG_PATH"));
        std::string pipeline = std::string(std::getenv("PIPELINE"));
        conf_path = conf_path + pipeline + "/";
        file_path = conf_path + script ;

    } catch (std::exception const& ex) {
        UPG_TRACE_ERR("Failed to execute hook  %s", ex.what());
    }
    return file_path;
}

bool
is_valid_script (const std::string script)
{
    std::string file;
    file = get_script_path(script);
    if (access(file.c_str(), F_OK) != 0) {
        UPG_TRACE_ERR("File %s doesn't exist !", file.c_str());
        return false;
    }

    if (access(file.c_str(), X_OK) != 0) {
        UPG_TRACE_ERR("File %s doesn't have execute permission !",
                      file.c_str());
        return false;
    }
    return true;
}

static bool
execute (const char *cmd)
{
    bool result=true;
    int status = system(cmd);

    if (status < 0) {
        UPG_TRACE_ERR("Failed to execute script, return error %d", errno);
        result = false;
    } else {
        if (WIFEXITED(status)) {
            UPG_TRACE_INFO("Successfully executed script, return status  %d",
                           WEXITSTATUS(status));
            result = true;
        } else {
            UPG_TRACE_ERR("Failed to execute script, exited abnormaly");
            result = false;
        }
    }
    return result;
}

bool
execute_hook (const std::string script, const std::string stage_name,
               hook_execution_t hook_type, svc_rsp_code_t status)
{
    bool result=true;
    std::string cmd;
    try {
        cmd = get_script_path(script);

        if (hook_type == PRE_STAGE) {
            UPG_TRACE_INFO("Executing pre-hook %s , in stage %s",
                           script.c_str(), stage_name.c_str());
            cmd = cmd + " -f" + stage_name;
            cmd = cmd + " -t pre" ;
            execute(cmd.c_str());
        } else {
            SDK_ASSERT(status != SVC_RSP_MAX);
            UPG_TRACE_INFO("Executing post-hook %s , in stage %s, with stage "
                           "status %d", script.c_str(), stage_name.c_str(),
                           status);
            cmd = cmd + " -f" + stage_name;
            cmd = cmd + " -t post";
            cmd = cmd + " -r " + std::string(svc_rsp_code_name[status]);

            SDK_ASSERT(status != SVC_RSP_MAX);
            execute(cmd.c_str());
        }
    } catch (std::exception const& ex) {
        UPG_TRACE_ERR("Failed to execute hook  %s", ex.what());
    }

    return result;
}

}    // namespace upg
}    // namespace sdk
