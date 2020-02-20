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

namespace upg
{
    void      token_parse(std::string& text, std::vector<std::string>& results);
    time_t              str_to_timeout(std::string& timeout);
    svc_sequence_t      str_to_svc_sequence(std::string& svc_seq);
    event_sequence_t    str_to_event_sequence(std::string& evt_seq);
    stage_id_t          name_to_stage_id(std::string stage);
    std::string         id_to_stage_name(stage_id_t stage);
    scripts_t           str_to_scripts (std::string scripts);
}
