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
#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg
{

void           token_parse(std::string& text,
                           std::vector<std::string>& results);
ev_tstamp      str_to_timeout(const std::string& timeout);
stage_id_t     name_to_stage_id(const std::string stage);
std::string    id_to_stage_name(const stage_id_t stage);
std::string    svc_sequence_to_str(const svc_sequence_t svcs);
std::string    transition_to_str(const transition_t& transitions);
std::string    script_to_str(const scripts_t& scripts);
scripts_t      str_to_scripts(std::string scripts);
upg_stage_t    upg_stage_id(const stage_id_t id);
svc_rsp_code_t svc_rsp_code(const upg_status_t id);
void           dump(const transition_t& transitions);
void           dump(const scripts_t& scripts);
void           dump(stage_t& stage);
void           dump(stages_t& stages);
void           dump(const services_t& svcs);
void           dump(const svc_sequence_t& svcs);
void           dump(const stage_map_t& tran);
void           dump(const fsm& fsm);

}
