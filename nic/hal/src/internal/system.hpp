//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/system.pb.h"

using sys::ApiCounter;
using sys::ApiStatsResponse;
using sys::ApiStatsEntry;
using sys::SystemGetRequest;
using sys::SystemResponse;
using sys::FeatureProfileResponse;
using sys::ForwardingModeResponse;
using sys::MicroSegSpec;
using sys::MicroSegResponse;
using sys::MicroSegEvent;
using sys::MicroSegUpdateRequest;
using sys::MicroSegUpdateResponse;
using sys::SysSpec;
using sys::SysSpecResponse;
using sys::SysSpecGetResponse;
using sys::SysSpecGetRequest;

namespace hal {

hal_ret_t api_stats_get(ApiStatsResponse *rsp);
hal_ret_t system_get(const SystemGetRequest *req, SystemResponse *rsp);
hal_ret_t system_uuid_get(SystemResponse *rsp);
hal_ret_t clear_pb_stats(void);
hal_ret_t clear_ingress_drop_stats(void);
hal_ret_t clear_egress_drop_stats(void);
hal_ret_t upgrade_table_reset(void);
hal_ret_t feature_profile_get(FeatureProfileResponse *rsp);
hal_ret_t micro_seg_update(MicroSegUpdateRequest &req, 
                           MicroSegUpdateResponse *rsp);
hal_ret_t micro_seg_status_update(MicroSegSpec &req, 
                                  MicroSegResponse *rsp);
hal_ret_t system_handle_fwd_policy_updates(const SysSpec *spec,
                                           SysSpecResponse *rsp);
hal_ret_t system_get_fwd_policy_mode(SysSpecGetResponse *rsp);
hal_ret_t system_handle_a_to_b(void);

}    // namespace hal

#endif    // __SYSTEM_HPP__

