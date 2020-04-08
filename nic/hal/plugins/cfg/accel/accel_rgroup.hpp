//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ACCEL_RGROUP_HPP__
#define __ACCEL_RGROUP_HPP__

#include "nic/include/base.hpp"
#include "nic/include/accel_ring.h"
#include "gen/proto/accel_rgroup.pb.h"
#include "nic/include/pd.hpp"

using accelRGroup::AccelRGroupAddRequest;
using accelRGroup::AccelRGroupAddResponse;
using accelRGroup::AccelRGroupDelRequest;
using accelRGroup::AccelRGroupDelResponse;
using accelRGroup::AccelRGroupRingAddRequest;
using accelRGroup::AccelRGroupRingAddResponse;
using accelRGroup::AccelRGroupRingDelRequest;
using accelRGroup::AccelRGroupRingDelResponse;
using accelRGroup::AccelRGroupResetSetRequest;
using accelRGroup::AccelRGroupResetSetResponse;
using accelRGroup::AccelRGroupEnableSetRequest;
using accelRGroup::AccelRGroupEnableSetResponse;
using accelRGroup::AccelRGroupPndxSetRequest;
using accelRGroup::AccelRGroupPndxSetResponse;
using accelRGroup::AccelRGroupInfoGetRequest;
using accelRGroup::AccelRGroupInfoGetResponse;
using accelRGroup::AccelRGroupIndicesGetRequest;
using accelRGroup::AccelRGroupIndicesGetResponse;
using accelRGroup::AccelRGroupMetricsGetRequest;
using accelRGroup::AccelRGroupMetricsGetResponse;
using accelRGroup::AccelRGroupMiscGetRequest;
using accelRGroup::AccelRGroupMiscGetResponse;

namespace hal {

hal_ret_t accel_rgroup_init(int tid, uint32_t accel_total_rings);
hal_ret_t accel_rgroup_fini(int tid);
hal_ret_t accel_rgroup_add(const AccelRGroupAddRequest& request,
                           AccelRGroupAddResponse *response);
hal_ret_t accel_rgroup_del(const AccelRGroupDelRequest& request,
                           AccelRGroupDelResponse *response);
hal_ret_t accel_rgroup_ring_add(const AccelRGroupRingAddRequest& request,
                                AccelRGroupRingAddResponse *response);
hal_ret_t accel_rgroup_ring_del(const AccelRGroupRingDelRequest& request,
                                AccelRGroupRingDelResponse *response);
hal_ret_t accel_rgroup_reset_set(const AccelRGroupResetSetRequest& request,
                                 AccelRGroupResetSetResponse *response);
hal_ret_t accel_rgroup_enable_set(const AccelRGroupEnableSetRequest& request,
                                  AccelRGroupEnableSetResponse *response);
hal_ret_t accel_rgroup_pndx_set(const AccelRGroupPndxSetRequest& request,
                                AccelRGroupPndxSetResponse *response);
hal_ret_t accel_rgroup_info_get(const AccelRGroupInfoGetRequest& request,
                                AccelRGroupInfoGetResponse *response);
hal_ret_t accel_rgroup_indices_get(const AccelRGroupIndicesGetRequest& request,
                                   AccelRGroupIndicesGetResponse *response);
hal_ret_t accel_rgroup_metrics_get(const AccelRGroupMetricsGetRequest& request,
                                   AccelRGroupMetricsGetResponse *response);
hal_ret_t accel_rgroup_misc_get(const AccelRGroupMiscGetRequest& request,
                                AccelRGroupMiscGetResponse *response);
}    // namespace hal

#endif // __ACCEL_RGROUP_HPP__
