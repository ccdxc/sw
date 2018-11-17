//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ACCEL_RGROUP_HPP__
#define __ACCEL_RGROUP_HPP__

#include "nic/include/base.hpp"
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

namespace hal {

/*
 * Ring config info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint64_t    base_pa;
    uint64_t    pndx_pa;
    uint64_t    shadow_pndx_pa;
    uint64_t    opaque_tag_pa;
    uint32_t    opaque_tag_size;
    uint32_t    ring_size;
    uint32_t    desc_size;
    uint32_t    pndx_size;
    bool        sw_reset_capable;
    bool        sw_enable_capable;
} accel_rgroup_ring_info_t;

typedef void (*accel_rgroup_ring_info_cb_t)(void *user_ctx,
                                            const accel_rgroup_ring_info_t& info);
/*
 * Ring indices info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint32_t    pndx;
    uint32_t    cndx;
} accel_rgroup_ring_indices_t;

typedef void (*accel_rgroup_ring_indices_cb_t)(void *user_ctx,
                                               const accel_rgroup_ring_indices_t& indices);

/*
 * Ring metrics info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint64_t    input_bytes;
    uint64_t    output_bytes;
    uint64_t    soft_resets;
} accel_rgroup_ring_metrics_t;

typedef void (*accel_rgroup_ring_metrics_cb_t)(void *user_ctx,
                                               const accel_rgroup_ring_metrics_t& metrics);


hal_ret_t AccelRGroupAdd(const AccelRGroupAddRequest& request,
                         AccelRGroupAddResponse *response);
hal_ret_t AccelRGroupDel(const AccelRGroupDelRequest& request,
                         AccelRGroupDelResponse *response);
hal_ret_t AccelRGroupRingAdd(const AccelRGroupRingAddRequest& request,
                             AccelRGroupRingAddResponse *response);
hal_ret_t AccelRGroupRingDel(const AccelRGroupRingDelRequest& request,
                             AccelRGroupRingDelResponse *response);
hal_ret_t AccelRGroupResetSet(const AccelRGroupResetSetRequest& request,
                              AccelRGroupResetSetResponse *response);
hal_ret_t AccelRGroupEnableSet(const AccelRGroupEnableSetRequest& request,
                               AccelRGroupEnableSetResponse *response);
hal_ret_t AccelRGroupPndxSet(const AccelRGroupPndxSetRequest& request,
                             AccelRGroupPndxSetResponse *response);
hal_ret_t AccelRGroupInfoGet(const AccelRGroupInfoGetRequest& request,
                             AccelRGroupInfoGetResponse *response);
hal_ret_t AccelRGroupIndicesGet(const AccelRGroupIndicesGetRequest& request,
                                AccelRGroupIndicesGetResponse *response);
hal_ret_t AccelRGroupMetricsGet(const AccelRGroupMetricsGetRequest& request,
                                AccelRGroupMetricsGetResponse *response);
}    // namespace hal

#endif // __ACCEL_RGROUP_HPP__
