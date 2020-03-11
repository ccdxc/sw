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
#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg
{

void      token_parse(std::string& text, std::vector<std::string>& results);
time_t              str_to_timeout(std::string& timeout);
stage_id_t           name_to_stage_id(std::string stage);
std::string         id_to_stage_name(stage_id_t stage);
scripts_t           str_to_scripts (std::string scripts);
upg_stage_t         upg_stage_id(stage_id_t id);
svc_rsp_code_t      svc_rsp_code( upg_status_t id);
void                dump(transition_t transitions);
void                dump(scripts_t scripts);
void                dump(stage_t stage);
void                dump(stages_t stages);
void                dump(services_t svcs);
void                dump(svc_sequence_t svcs);
void                dump(stage_map_t tran);
void                dump(fsm& fsm);

}
