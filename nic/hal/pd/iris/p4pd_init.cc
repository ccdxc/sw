// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "lib/table/tcam/tcam.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/p4/common/defines.h"
#include "nic/p4/rdma/include/rdma_defines.h"
#include "nic/hal/iris/datapath/p4/include/table_sizes.h"
#include "nic/hal/pd/iris/nw/rw_pd.hpp"
#include "nic/hal/pd/iris/nw/tnnl_rw_pd.hpp"
#include "nic/hal/pd/iris/p4pd_defaults.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "gen/proto/types.pb.h"
#include "nic/hal/pd/iris/aclqos/acl_pd.hpp"
#include "nic/hal/pd/hal_pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"

#define PAGE_SZ 4096

using sdk::table::tcam;
using hal::pd::utils::acl_tcam_entry_handle_t;
using hal::pd::utils::priority_t;

namespace hal {
namespace pd {

#if 0
static hal_ret_t
p4pd_ddos_policers_init (void)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    sdk_ret_t                                   sdk_ret;
    directmap                                   *dm;
    directmap                                   *dm_act;
    ddos_src_vf_policer_actiondata_t              d_svf = { 0 };
    ddos_service_policer_actiondata_t             d_service = { 0 };
    ddos_src_dst_policer_actiondata_t             d_srcdst = { 0 };
    ddos_src_vf_policer_action_actiondata_t       dact_svf = { 0 };
    ddos_service_policer_action_actiondata_t      dact_service = { 0 };
    ddos_src_dst_policer_action_actiondata_t      dact_srcdst = { 0 };

    /*
     * Invalidate the first four policers. Entry valid bit is set to
     * zero by default
     */
    dm = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SRC_VF_POLICER);
    dm_act = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SRC_VF_POLICER_ACTION);
    SDK_ASSERT(dm != NULL);
    for (int i = 0; i < 4; i++) {
        sdk_ret = dm->insert_withid(&d_svf, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos src_vf policer init failed, err : {}", ret);
            return ret;
        }
        sdk_ret = dm_act->insert_withid(&dact_svf, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos src_vf policeract init failed, err : {}", ret);
            return ret;
        }
    }

    dm = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SERVICE_POLICER);
    dm_act = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SERVICE_POLICER_ACTION);
    SDK_ASSERT(dm != NULL);
    for (int i = 0; i < 4; i++) {
        sdk_ret = dm->insert_withid(&d_service, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos service policer init failed, err : {}", ret);
            return ret;
        }
        sdk_ret = dm_act->insert_withid(&dact_service, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos service policeract init failed, err : {}", ret);
            return ret;
        }
    }

    dm = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SRC_DST_POLICER);
    dm_act = g_hal_state_pd->dm_table(P4TBL_ID_DDOS_SRC_DST_POLICER_ACTION);
    SDK_ASSERT(dm != NULL);
    for (int i = 0; i < 4; i++) {
        sdk_ret = dm->insert_withid(&d_srcdst, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos src_dst policer init failed, err : {}", ret);
            return ret;
        }
        sdk_ret = dm_act->insert_withid(&dact_srcdst, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ddos src_dst policeract init failed, err : {}", ret);
            return ret;
        }
    }

    return (ret);
}
#endif

static hal_ret_t
p4pd_input_mapping_native_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    uint32_t                             idx;
    input_mapping_native_swkey_t         key;
    input_mapping_native_swkey_mask_t    mask;
    input_mapping_native_actiondata_t      data;
    hal_ret_t                            ret;
    sdk_ret_t                            sdk_ret;
    tcam                                 *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_INPUT_MAPPING_NATIVE);
    SDK_ASSERT(tcam != NULL);

    // entry for IPv4 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_NATIVE_NATIVE_IPV4_PACKET_ID;

    // insert into the tcam now - default entries are inserted bottom-up
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for IPv6 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 1;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_NATIVE_NATIVE_IPV6_PACKET_ID;

    // insert into the tcam now - default entries are inserted bottom-up
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for non-IP native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_NATIVE_NATIVE_NON_IP_PACKET_ID;

    // insert into the tcam now
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_input_mapping_tunneled_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    uint32_t                             idx;
    input_mapping_tunneled_swkey_t       key;
    input_mapping_tunneled_swkey_mask_t  mask;
    input_mapping_tunneled_actiondata_t    data;
    hal_ret_t                            ret;
    sdk_ret_t                            sdk_ret;
    tcam                                 *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_INPUT_MAPPING_TUNNELED);
    SDK_ASSERT(tcam != NULL);

    // no-op entry for IPv4 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_TUNNELED_NOP_ID;
    // insert into the tcam now
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, idx : {}", idx);

    // no-op entry for IPv6 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 1;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_TUNNELED_NOP_ID;

    // insert into the tcam now
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // no-op entry for non-IP native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_inactive_input_mapping = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_inactive_input_mapping_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0x0;

    // set the action
    data.action_id = INPUT_MAPPING_TUNNELED_NOP_ID;

    // insert into the tcam now
    sdk_ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
//  NACL Stats init 
//  0: nop entry.
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_nacl_stats_init (void)
{
    uint32_t                    idx = NACL_STATS_NOP_ENTRY;
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    directmap                  *dm;
    nacl_stats_actiondata_t     data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_NACL_STATS);
    SDK_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.action_id = NACL_STATS_NOP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("NACL Stats table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_l4_profile_init (void)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    directmap                *dm;
    l4_profile_actiondata_t    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_L4_PROFILE);
    SDK_ASSERT(dm != NULL);
    sdk_ret = dm->insert_withid(&data, L4_PROF_DEFAULT_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("L4 profile table write failure, idx : 0, err : {}",
                      ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_flow_info_init (void)
{
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    flow_info_actiondata_t    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    SDK_ASSERT(dm != NULL);

    // "catch-all" flow miss entry
    data.action_id = FLOW_INFO_FLOW_MISS_ID;
    sdk_ret = dm->insert_withid(&data, FLOW_INFO_MISS_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow info table write failure for miss entry, err : {}",
                      ret);
        return ret;
    }

    // common flow hit & drop entry
    data.action_id = FLOW_INFO_FLOW_HIT_DROP_ID;
    data.action_u.flow_info_flow_hit_drop.start_timestamp = 0;
    sdk_ret = dm->insert_withid(&data, FLOW_INFO_DROP_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow info table write failure for drop entry, err : {}",
                      ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Session State
//  0: nop entry.
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_session_state_init (void)
{
    uint32_t                    idx = SESSION_STATE_NOP_ENTRY;
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm;
    session_state_actiondata_t    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    SDK_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.action_id = SESSION_STATE_NOP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session state table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Flow Stats: Default entries
//  0: nop entry.
//  1: Have to be in sync in flow stats.
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_flow_stats_init (void)
{
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    flow_stats_actiondata_t   data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.action_id = FLOW_STATS_FLOW_STATS_ID;
    sdk_ret = dm->insert_withid(&data, FLOW_STATS_NOP_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, idx : {}, err : {}",
                      FLOW_STATS_NOP_ENTRY, ret);
        return ret;
    }

    // claim one more entry to be in sync with flow info table
    sdk_ret = dm->insert_withid(&data, FLOW_STATS_RSVD_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, idx : {}, err : {}",
                      FLOW_STATS_RSVD_ENTRY, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Initializes drop stats table
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_drop_stats_init (void)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    tcam                     *tcam;
    drop_stats_swkey         key      = { 0 };
    drop_stats_swkey_mask    key_mask = { 0 };
    drop_stats_actiondata_t    data     = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    for (int i = DROP_MIN; i <= DROP_MAX; i++) {
        uint64_t drop_reason = ((uint64_t)1 << i);
        key.entry_inactive_drop_stats = 0;
        memcpy(key.control_metadata_drop_reason, &drop_reason,
               sizeof(key.control_metadata_drop_reason));
        key_mask.entry_inactive_drop_stats_mask = 0xFF;
        memcpy(key_mask.control_metadata_drop_reason_mask, &drop_reason,
               sizeof(key_mask.control_metadata_drop_reason_mask));

        data.action_id = DROP_STATS_DROP_STATS_ID;
        sdk_ret = tcam->insert_withid(&key, &key_mask, &data, i);
        if (sdk_ret != sdk::SDK_RET_OK) {
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            HAL_TRACE_ERR("drop stats table write failure, idx : {}, err : {}",
                          i, ret);
            return ret;
        }
    }

    // catch all entry for unknown drop reasons
    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));
    key.entry_inactive_drop_stats = 0;
    key_mask.entry_inactive_drop_stats_mask = 0xFF;
    data.action_id = DROP_STATS_DROP_STATS_ID;
    sdk_ret = tcam->insert_withid(&key, &key_mask, &data, DROP_MAX + 1);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("drop stats table write failure, idx : {}, err : {}",
                      DROP_MAX + 1, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_egress_drop_stats_init (void)
{
    hal_ret_t                       ret;
    sdk_ret_t                       sdk_ret;
    tcam                            *tcam;
    egress_drop_stats_swkey         key      = { 0 };
    egress_drop_stats_swkey_mask    key_mask = { 0 };
    egress_drop_stats_actiondata_t    data     = { 0 };

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_EGRESS_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    for (int i = EGRESS_DROP_MIN; i <= EGRESS_DROP_MAX; i++) {
        uint64_t drop_reason = ((uint64_t)1 << i);
        key.entry_inactive_egress_drop_stats = 0;
        memcpy(key.control_metadata_egress_drop_reason, &drop_reason,
               sizeof(key.control_metadata_egress_drop_reason));
        key_mask.entry_inactive_egress_drop_stats_mask = 0xFF;
        memcpy(key_mask.control_metadata_egress_drop_reason_mask, &drop_reason,
               sizeof(key_mask.control_metadata_egress_drop_reason_mask));

        data.action_id = EGRESS_DROP_STATS_EGRESS_DROP_STATS_ID;
        sdk_ret = tcam->insert_withid(&key, &key_mask, &data, i);
        if (sdk_ret != sdk::SDK_RET_OK) {
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            HAL_TRACE_ERR("egress drop stats table write failure, idx : {}, "
                          "err : {}", i, ret);
            return ret;
        }
    }

    // catch all entry for unknown drop reasons
    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));
    key.entry_inactive_egress_drop_stats = 0;
    key_mask.entry_inactive_egress_drop_stats_mask = 0xFF;
    data.action_id = EGRESS_DROP_STATS_EGRESS_DROP_STATS_ID;
    sdk_ret = tcam->insert_withid(&key, &key_mask, &data, EGRESS_DROP_MAX + 1);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("egress drop stats table write failure, idx : {}, "
                      "err : {}", EGRESS_DROP_MAX + 1, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_qos_init (uint32_t admin_cos)
{
    hal_ret_t      ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    directmap      *qos_tbl = NULL;
    qos_actiondata_t d = {0};

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    SDK_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    for (int i = 0; i < QOS_TABLE_SIZE; i++) {
        sdk_ret = qos_tbl->insert_withid(&d, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: qos table write failure ret {}",
                          __func__, ret);
            return ret;
        }
    }

    // Reserve the IQ needed for the ADMIN class
    g_hal_state_pd->qos_txdma_iq_idxr()->alloc_withid(admin_cos);
#if 0
    pd_qos_class_get_admin_cos_args_t args = {0};
    pd_qos_class_get_admin_cos(&args);
    g_hal_state_pd->qos_txdma_iq_idxr()->alloc_withid(args.cos);
    // g_hal_state_pd->qos_txdma_iq_idxr()->alloc_withid(qos_class_get_admin_cos());
#endif
    return ret;
}

static hal_ret_t
p4pd_p4plus_app_init (void)
{
    hal_ret_t                ret = HAL_RET_OK;
    sdk_ret_t                sdk_ret;
    directmap                *dm;
    p4plus_app_actiondata_t data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_P4PLUS_APP);
    SDK_ASSERT(dm != NULL);

    for (int i = P4PLUS_APPTYPE_MIN; i <= P4PLUS_APPTYPE_MAX; i++) {
        switch(i) {
#ifndef P4_16
            case P4PLUS_APPTYPE_DEFAULT:
                data.action_id = P4PLUS_APP_P4PLUS_APP_DEFAULT_ID;
                break;
#endif
            case P4PLUS_APPTYPE_CLASSIC_NIC:
                data.action_id = P4PLUS_APP_P4PLUS_APP_CLASSIC_NIC_ID;
                break;
            case P4PLUS_APPTYPE_RDMA:
                data.action_id = P4PLUS_APP_P4PLUS_APP_RDMA_ID;
                break;
            case P4PLUS_APPTYPE_TCPTLS:
                data.action_id = P4PLUS_APP_P4PLUS_APP_TCP_PROXY_ID;
                break;
            case P4PLUS_APPTYPE_IPSEC:
                data.action_id = P4PLUS_APP_P4PLUS_APP_IPSEC_ID;
                break;
            case P4PLUS_APPTYPE_STORAGE:
                break;
            case P4PLUS_APPTYPE_TELEMETRY:
                break;
            case P4PLUS_APPTYPE_CPU:
                data.action_id = P4PLUS_APP_P4PLUS_APP_CPU_ID;
                break;
            case P4PLUS_APPTYPE_RAW_REDIR:
                data.action_id = P4PLUS_APP_P4PLUS_APP_RAW_REDIR_ID;
                break;
            case P4PLUS_APPTYPE_P4PT:
                data.action_id = P4PLUS_APP_P4PLUS_APP_P4PT_ID;
                break;
            default:
                HAL_TRACE_ERR("Unknown app_type: {}", i);
                SDK_ASSERT(0);
        }

        sdk_ret = dm->insert_withid(&data, i);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("p4plus app table write failure, idx : {}, err : {}",
                    i, ret);
            return ret;
        }
    }

    return ret;
}

typedef struct tunnel_decap_copy_inner_key_t_ {
    union {
        struct {
            uint8_t inner_ethernet_valid : 1;
            uint8_t inner_ipv6_valid : 1;
            uint8_t inner_ipv4_valid : 1;
            uint8_t inner_udp_valid : 1;
            uint8_t pad : 4;
        } __PACK__;
        uint8_t val;
    } __PACK__;
} tunnel_decap_copy_inner_key_t;

/*
 * TODO: Temporary function to return the index given the key bits
 * for the direct index table. This will be replaced by the
 * generated P4PD function once it is available
 */
static uint32_t
p4pd_get_tunnel_decap_copy_inner_tbl_idx (bool inner_udp_valid,
                                          bool inner_ipv4_valid,
                                          bool inner_ipv6_valid,
                                          bool inner_ethernet_valid)
{
    tunnel_decap_copy_inner_key_t key = {0};

    key.inner_udp_valid = inner_udp_valid;
    key.inner_ipv4_valid = inner_ipv4_valid;
    key.inner_ipv6_valid = inner_ipv6_valid;
    key.inner_ethernet_valid = inner_ethernet_valid;

    uint32_t ret_val = key.val;
    return (ret_val);
}

static hal_ret_t
p4pd_tunnel_decap_copy_inner_init (void)
{
    uint32_t                              idx = 0;
    hal_ret_t                             ret;
    sdk_ret_t                             sdk_ret;
    directmap                             *dm;
    tunnel_decap_copy_inner_actiondata_t    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_DECAP_COPY_INNER);
    SDK_ASSERT(dm != NULL);

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, true, false, false);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV4_UDP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, true, false, false);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV4_OTHER_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, false, true, false);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV6_UDP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, true, false);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV6_OTHER_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, true, false, true);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV4_UDP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, true, false, true);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV4_OTHER_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, false, true, true);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV6_UDP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, true, true);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV6_OTHER_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, false, true);
    data.action_id = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_NON_IP_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Twice NAT:
//  0: nop entry
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_twice_nat_init (void)
{
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    twice_nat_actiondata_t    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    SDK_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.action_id = TWICE_NAT_NOP_ID;
    sdk_ret = dm->insert_withid(&data, TWICE_NAT_NOP_ENTRY);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("twice nat table write failure, idx : {}, err : {}",
                      TWICE_NAT_NOP_ENTRY, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Rewrite table:
//  0: nop entry
//  1: decap vlan entry
//     - L2 Mcast packets
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_rewrite_init (void)
{
    uint32_t              idx = 0;
    hal_ret_t             ret;
    directmap             *dm;
    pd_rw_entry_key_t     rw_key{};
    pd_rw_entry_info_t    rw_info{};


    dm = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    SDK_ASSERT(dm != NULL);

    // "catch-all" nop entry
    rw_key.rw_act = REWRITE_NOP_ID;
    rw_info.with_id = true;
    rw_info.rw_idx = REWRITE_NOP_ENTRY;
    ret = rw_entry_alloc(&rw_key, &rw_info, &idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rewrite table write failure, idx : {}, err : {}",
                      REWRITE_NOP_ENTRY, ret);
        return ret;
    }

    // "decap vlan" entry -
    rw_key.rw_act = REWRITE_REWRITE_ID;
    rw_info.rw_idx = REWRITE_DECAP_VLAN_ENTRY;
    ret = rw_entry_alloc(&rw_key, &rw_info, &idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rewrite table write failure, idx : {}, err : {}",
                      REWRITE_DECAP_VLAN_ENTRY, ret);
        return ret;
    }

    g_hal_state_pd->set_rwr_tbl_decap_vlan_idx(REWRITE_DECAP_VLAN_ENTRY);
    return HAL_RET_OK;
}

typedef struct tunnel_encap_update_inner_key_t_ {
    union {
        struct {
            uint8_t ipv6_valid : 1;
            uint8_t ipv4_valid : 1;
            uint8_t udp_valid : 1;
            uint8_t tcp_valid : 1;
            uint8_t icmp_valid : 1;
            uint8_t pad : 3;
        } __PACK__;
        uint8_t val;
    } __PACK__;
} tunnel_encap_update_inner_key_t;

/*
 * TODO: Temporary function to return the index given the key bits
 * for the direct index table. This will be replaced by the
 * generated P4PD function once it is available
 */
static uint32_t
p4pd_get_tunnel_encap_update_inner_tbl_idx (bool ipv6_valid,
                                          bool ipv4_valid,
                                          bool udp_valid,
                                          bool tcp_valid,
                                          bool icmp_valid)
{
    tunnel_encap_update_inner_key_t key = {0};

    key.ipv6_valid = ipv6_valid;
    key.ipv4_valid = ipv4_valid;
    key.udp_valid = udp_valid;
    key.tcp_valid = tcp_valid;
    key.icmp_valid = icmp_valid;

    uint32_t ret_val = key.val;
    return (ret_val);
}

static hal_ret_t
p4pd_tunnel_encap_update_inner (void)
{
    uint32_t                              idx = 0;
    hal_ret_t                             ret;
    sdk_ret_t                             sdk_ret;
    directmap                             *dm;
    tunnel_encap_update_inner_actiondata_t  data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_ENCAP_UPDATE_INNER);
    SDK_ASSERT(dm != NULL);

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, true, false, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_UDP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, true, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_TCP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, false, true);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_ICMP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, false, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_UNKNOWN_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, true, false, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_UDP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, true, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_TCP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, false, true);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_ICMP_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, false, false);
    data.action_id = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_UNKNOWN_REWRITE_ID;
    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Init entries for EGRESS tunnel rewrite table
//
//  0: No-op Entry
//  1: To encap vlan entry. Flow will drive this whenever a flow needs to add
//     or modify a vlan encap.
//
//  Bridging:
//      Flow: mac_sa_rw:0, mac_da_rw:0, ttl_dec:0
//      -> Untag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 0]
//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
//          -> tnnl_rwr_table[0] (nop)
//      -> Tag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 1]
//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
//          -> tnnl_rwr_table[1] (encap with tnnl_vnid if eif is Uplink,
//                                encap from outpu_mapping, cos rwr)
//
//  Routing:
//     Flow: mac_sa_rw:1, mac_da_rw:1, ttl_dec:1
//      -> Untag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 0]
//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
//          -> tnnl_rwr_table[0] (nop)
//      -> Tag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 1]
//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
//          -> tnnl_rwr_table[1] (encap with tnnl_vnid if eif is Uplink,
//                                encap from outpu_mapping, cos rwr)
//
// ----------------------------------------------------------------------------
static hal_ret_t
p4pd_tunnel_rewrite_init (void)
{
    uint32_t                     idx;
    hal_ret_t                    ret;
    directmap                    *dm;
    pd_tnnl_rw_entry_key_t       rw_key{};
    pd_tnnl_rw_entry_info_t      rw_info{};

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    SDK_ASSERT(dm != NULL);

    rw_info.with_id = true;

    // "catch-all" nop entry
    rw_key.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
    rw_info.tnnl_rw_idx = TUNNEL_REWRITE_NOP_ENTRY;
    ret = tnnl_rw_entry_alloc(&rw_key, &rw_info, &idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, idx : {}, err : {}",
                      TUNNEL_REWRITE_NOP_ENTRY, ret);
        return ret;
    }

    // "encap_vlan" entry
    rw_key.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VLAN_ID;
    rw_info.tnnl_rw_idx = TUNNEL_REWRITE_ENCP_VLAN_ENTRY;
    ret = tnnl_rw_entry_alloc(&rw_key, &rw_info, &idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, idx : {}, err : {}",
                      TUNNEL_REWRITE_ENCP_VLAN_ENTRY, ret);
        return ret;
    }
    g_hal_state_pd->set_tnnl_rwr_tbl_encap_vlan_idx(TUNNEL_REWRITE_ENCP_VLAN_ENTRY);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Mirror table: Initialization
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_mirror_table_init (void)
{
    uint32_t                     idx = 0;
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    mirror_actiondata_t          data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);
    SDK_ASSERT(dm != NULL);

    // Initialize for usable span session.
    data.action_id = MIRROR_DROP_MIRROR_ID;
    for (idx = 0; idx < MIRROR_RESERVED_INDICES; idx++) {
        sdk_ret = dm->insert_withid(&data, idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("mirror table initialization failed for idx : {}, err : {}",
                          idx, ret);
            return ret;
        }
    }

    // Entry for FTE Span
    data.action_id = MIRROR_LOCAL_SPAN_ID;
    data.action_u.mirror_local_span.dst_lport = CPU_LPORT;
    data.action_u.mirror_local_span.qid_en = 1;
    data.action_u.mirror_local_span.qid = types::CPUCB_ID_FTE_SPAN;
    sdk_ret = dm->insert_withid(&data, MIRROR_FTE_SPAN_IDX);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("mirror table initialization failed for idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

typedef struct roce_opcode_info_t {
    uint32_t valid:1;
    uint32_t roce_hdr_length: 8; //in bytes
    uint32_t type: 4; //LIF sub-type
    uint32_t raw_flags:16;
} roce_opcode_info_t;

roce_opcode_info_t opc_to_info[DECODE_ROCE_OPCODE_TABLE_SIZE] = {
    //Reliable-Connect opcodes
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_FIRST | RESP_RX_FLAG_SEND)}, //0 - send-first
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_SEND)}, //1 - send-middle
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION)}, //2 - send-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //3 - send-last-with-immediate
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION)}, //4 - send-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //5 - send-only-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_FIRST | RESP_RX_FLAG_WRITE)}, //6 - write-first
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_WRITE)}, //7 - write-middle
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_WRITE)}, //8 - write-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_immeth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)},//9 - write-last-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_WRITE)}, //10 - write-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t)+sizeof(rdma_immeth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION)}, //11 - write-only-with-immediate
    {1, sizeof(rdma_bth_t)+sizeof(rdma_reth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_READ_REQ)}, //12 - read-request
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_FIRST | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //13 - read-response-first
    {1, sizeof(rdma_bth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_MIDDLE | REQ_RX_FLAG_READ_RESP)}, //14 - read-response-middle
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_LAST | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //15 - read-response-last
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_ONLY | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH | REQ_RX_FLAG_COMPLETION)}, //16 - read-response-only
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_AETH | REQ_RX_FLAG_ACK | REQ_RX_FLAG_COMPLETION)}, //17 - ack
    {1, sizeof(rdma_bth_t)+sizeof(rdma_aeth_t)+sizeof(rdma_atomicaeth_t), Q_TYPE_RDMA_SQ,
     (REQ_RX_FLAG_AETH | REQ_RX_FLAG_ATOMIC_AETH | REQ_RX_FLAG_COMPLETION)}, //18 - atomic-ack
    {1, sizeof(rdma_bth_t)+sizeof(rdma_atomiceth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ATOMIC_CSWAP)}, //19 - compare-and-swap
    {1, sizeof(rdma_bth_t)+sizeof(rdma_atomiceth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ATOMIC_FNA)}, //20 - fetch-and-add
    {0, 0, 0}, //21 - Reserved
    {1, sizeof(rdma_bth_t)+sizeof(rdma_ieth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_LAST | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY)}, //22 - send-last-with-inv-rkey
    {1, sizeof(rdma_bth_t)+sizeof(rdma_ieth_t), Q_TYPE_RDMA_RQ,
     (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY)}, //23 - send-only-with-inv-rkey
};

static hal_ret_t
p4pd_decode_roce_opcode_init (void)
{
    uint32_t                     idx = 0;
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    decode_roce_opcode_actiondata_t data = { 0 };

    // C++ compiler did not allow sparse initialization. compiler must be old.
    // So lets initialize the for UD entries here.

    opc_to_info[100].valid = 1;
    opc_to_info[100].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_deth_t);
    opc_to_info[100].type = Q_TYPE_RDMA_RQ;
    opc_to_info[100].raw_flags = (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND |
                                  RESP_RX_FLAG_COMPLETION|RESP_RX_FLAG_UD);

    opc_to_info[101].valid = 1;
    opc_to_info[101].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_deth_t)+sizeof(rdma_immeth_t);
    opc_to_info[101].type = Q_TYPE_RDMA_RQ;
    opc_to_info[101].raw_flags = (RESP_RX_FLAG_ONLY | RESP_RX_FLAG_SEND |
                                  RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_COMPLETION |
                                  RESP_RX_FLAG_UD);

    // CNP packet entry.
    opc_to_info[129].valid = 1;
    opc_to_info[129].roce_hdr_length = sizeof(rdma_bth_t)+sizeof(rdma_reth_t);
    opc_to_info[129].type = Q_TYPE_RDMA_RQ;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_DECODE_ROCE_OPCODE);
    SDK_ASSERT(dm != NULL);

    for (idx = 0; idx < DECODE_ROCE_OPCODE_TABLE_SIZE; idx++) {

        if (opc_to_info[idx].valid == 1) {

            // valid entry
            data.action_id = DECODE_ROCE_OPCODE_DECODE_ROCE_OPCODE_ID;
            data.action_u.decode_roce_opcode_decode_roce_opcode.qtype =
                opc_to_info[idx].type;
            data.action_u.decode_roce_opcode_decode_roce_opcode.len =
                opc_to_info[idx].roce_hdr_length;
            data.action_u.decode_roce_opcode_decode_roce_opcode.raw_flags =
                opc_to_info[idx].raw_flags;
        } else {
            //Except for invalid opcode 32(error test case), skip all others for now, to save HAL bootup time
            if (idx != 32) {
                continue;
            }
            data.action_id = DECODE_ROCE_OPCODE_DECODE_ROCE_OPCODE_ID;
            data.action_u.decode_roce_opcode_decode_roce_opcode.qtype = Q_TYPE_RDMA_RQ;
            data.action_u.decode_roce_opcode_decode_roce_opcode.len = sizeof(rdma_bth_t);
            data.action_u.decode_roce_opcode_decode_roce_opcode.raw_flags = 0;
        }

        sdk_ret = dm->insert_withid(&data, idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("decode roce opcode table write failure, idx : {}, err : {}",
                          idx, ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}

typedef struct compute_checksum_table_ {
    uint16_t ipv4_valid       : 1;
    uint16_t ipv6_valid       : 1;
    uint16_t inner_ipv4_valid : 1;
    uint16_t inner_ipv6_valid : 1;
    uint16_t tcp_valid        : 1;
    uint16_t udp_valid        : 1;
    uint16_t inner_udp_valid  : 1;
    uint16_t icmp_valid       : 1;
    uint8_t  actionid;
} compute_checksum_table_t;

compute_checksum_table_t compute_checksum_table[] = {
    /*****************************************
     v4, v6, iv4, iv6, tcp, udp, iudp, icmp, action
     *****************************************/
    { 0,  0,   0,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_NOP_ID}, // app = rdma
    { 0,  0,   0,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_NOP_ID}, // app = tcptls
    { 0,  0,   0,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_NOP_ID}, // app = p4pt
    { 1,  0,   0,   0,   1,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM1_ID},
    { 1,  0,   0,   0,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM2_ID},
    { 1,  0,   0,   1,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM2_ID},
    { 1,  0,   0,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM3_ID},
    { 1,  0,   1,   0,   1,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM4_ID},
    { 1,  0,   1,   0,   0,   0,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM5_ID},
    { 1,  0,   1,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM6_ID},
    { 1,  0,   1,   0,   1,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM7_ID},
    { 1,  0,   1,   0,   0,   1,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM8_ID},
    { 1,  0,   1,   0,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM9_ID},
    { 1,  0,   0,   1,   1,   0,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM10_ID},
    { 1,  0,   0,   1,   0,   0,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM11_ID},
    { 1,  0,   0,   1,   1,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM12_ID},
    { 1,  0,   0,   1,   0,   1,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM13_ID},
    { 0,  1,   0,   0,   1,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM14_ID},
    { 0,  1,   0,   0,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM15_ID},
    { 0,  1,   0,   1,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM15_ID},
    { 0,  1,   1,   0,   1,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM16_ID},
    { 0,  1,   1,   0,   0,   0,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM17_ID},
    { 0,  1,   1,   0,   0,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM18_ID},
    { 0,  1,   1,   0,   1,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM19_ID},
    { 0,  1,   1,   0,   0,   1,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM20_ID},
    { 0,  1,   1,   0,   0,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM21_ID},
    { 0,  1,   0,   1,   1,   1,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM22_ID},
    { 0,  1,   0,   1,   0,   1,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM23_ID},
    { 0,  1,   0,   1,   1,   0,    0,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM24_ID},
    { 0,  1,   0,   1,   0,   0,    1,    0, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM25_ID},
    { 1,  0,   1,   0,   0,   1,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM26_ID},
    { 1,  0,   0,   1,   0,   1,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM27_ID},
    { 1,  0,   1,   0,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM28_ID},
    { 1,  0,   0,   1,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM29_ID},
    { 1,  0,   0,   0,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM30_ID},
    { 0,  1,   1,   0,   0,   1,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM31_ID},
    { 0,  1,   0,   1,   0,   1,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM32_ID},
    { 0,  1,   1,   0,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM33_ID},
    { 0,  1,   0,   1,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM34_ID},
    { 0,  1,   0,   0,   0,   0,    0,    1, COMPUTE_CHECKSUM_COMPUTE_CHECKSUM35_ID},
};

static hal_ret_t
p4pd_compute_checksum_init(void)
{
    uint32_t                        idx;
    compute_checksum_swkey_t        key;
    compute_checksum_swkey_mask_t   mask;
    compute_checksum_actiondata_t     data;
    hal_ret_t                       ret;
    sdk_ret_t                       sdk_ret;
    tcam                            *tcam;

    idx = 0;
    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_COMPUTE_CHECKSUM);
    SDK_ASSERT(tcam != NULL);

    // don't compute checksum for rdma apptype
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));
    key.entry_inactive_compute_checksum = 0;
    key.control_metadata_p4plus_app_id = P4PLUS_APPTYPE_RDMA;
    mask.entry_inactive_compute_checksum_mask = 0xFF;
    mask.control_metadata_p4plus_app_id_mask = 0xFF;
    data.action_id = COMPUTE_CHECKSUM_NOP_ID;
    sdk_ret = tcam->insert_withid(&key, &mask, &data, idx);
    SDK_ASSERT(sdk_ret == sdk::SDK_RET_OK);
    idx++;

    // don't compute checksum for tcptls apptype
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));
    key.entry_inactive_compute_checksum = 0;
    key.control_metadata_p4plus_app_id = P4PLUS_APPTYPE_TCPTLS;
    mask.entry_inactive_compute_checksum_mask = 0xFF;
    mask.control_metadata_p4plus_app_id_mask = 0xFF;
    data.action_id = COMPUTE_CHECKSUM_NOP_ID;
    sdk_ret = tcam->insert_withid(&key, &mask, &data, idx);
    SDK_ASSERT(sdk_ret == sdk::SDK_RET_OK);
    idx++;

    // don't compute checksum for p4pt apptype
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));
    key.entry_inactive_compute_checksum = 0;
    key.control_metadata_p4plus_app_id = P4PLUS_APPTYPE_P4PT;
    mask.entry_inactive_compute_checksum_mask = 0xFF;
    mask.control_metadata_p4plus_app_id_mask = 0xFF;
    data.action_id = COMPUTE_CHECKSUM_NOP_ID;
    sdk_ret = tcam->insert_withid(&key, &mask, &data, idx);
    SDK_ASSERT(sdk_ret == sdk::SDK_RET_OK);
    idx++;

    // p4plus_app_id is don't care for all the entries below
    memset(&mask, 0xFF, sizeof(mask));
    mask.control_metadata_p4plus_app_id_mask = 0x0;
    for (;idx < sizeof(compute_checksum_table)/sizeof(compute_checksum_table_t);
         idx++) {
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));

        // key
        key.entry_inactive_compute_checksum = 0;
        key.ipv4_valid = compute_checksum_table[idx].ipv4_valid;
        key.ipv6_valid = compute_checksum_table[idx].ipv6_valid;
        key.inner_ipv4_valid = compute_checksum_table[idx].inner_ipv4_valid;
        key.inner_ipv6_valid = compute_checksum_table[idx].inner_ipv6_valid;
        key.tcp_valid = compute_checksum_table[idx].tcp_valid;
        key.udp_valid = compute_checksum_table[idx].udp_valid;
        key.inner_udp_valid = compute_checksum_table[idx].inner_udp_valid;
        key.icmp_valid = compute_checksum_table[idx].icmp_valid;

        // action
        data.action_id = compute_checksum_table[idx].actionid;

        // insert into TCAM at idx
        sdk_ret = tcam->insert_withid(&key, &mask, &data, idx);
        if (sdk_ret != sdk::SDK_RET_OK) {
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            HAL_TRACE_ERR("Compute checksum tcam write failure, "
                          "idx : {}, err : {}", idx, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Replication Lists:
//  - P4_NW_MCAST_INDEX_FIN_COPY & P4_NW_MCAST_INDEX_RST_COPY
//    - P4 drives these repl. lists for the respected packets.
//    - Lport driven from flow will be honored and a CPU copy will be sent.
//    - Each of them have different qids
//  - P4_NW_MCAST_INDEX_FLOW_REL_COPY
//    - Same as above but this is not inherently driven by P4 code and have
//      a different qid than above two.
//-----------------------------------------------------------------------------
// TODO: Ajeer -- why is this called capri_xxx ??
static hal_ret_t
capri_repl_pgm_def_entries (void)
{
    p4_replication_data_t data;

    /*Create as many Lists as required*/
    hal::pd::g_hal_state_pd->met_table()->create_repl_list_with_id(P4_NW_MCAST_INDEX_FIN_COPY);
    hal::pd::g_hal_state_pd->met_table()->create_repl_list_with_id(P4_NW_MCAST_INDEX_RST_COPY);
    hal::pd::g_hal_state_pd->met_table()->create_repl_list_with_id(P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    hal::pd::g_hal_state_pd->met_table()->create_repl_list_with_id(P4_NW_MCAST_INDEX_P4PT_COPY);

    /* Add 1st repication copy for list 1*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_FIN_COPY, &data);

    /* Add 2nd repication copy for list 1*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_TO_CPU_REL_COPY;
    data.lport = CPU_LPORT;
    data.is_qid = 1;
    data.qid_or_vnid = types::CPUCB_ID_TCP_CLOSE;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_FIN_COPY, &data);


    /* Add 1st repication copy for list 2*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_RST_COPY, &data);

    /* Add 2nd repication copy for list 2*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_TO_CPU_REL_COPY;
    data.lport = CPU_LPORT;
    data.is_qid = 1;
    data.qid_or_vnid = types::CPUCB_ID_TCP_CLOSE;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_RST_COPY, &data);

    /* Add 1st repication copy for list 3*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_FLOW_REL_COPY, &data);

    /* Add 2nd repication copy for list 3*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_TO_CPU_REL_COPY;
    data.lport = CPU_LPORT;
    data.is_qid = 1;
    data.qid_or_vnid = types::CPUCB_ID_RELIABLE_COPY;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_FLOW_REL_COPY, &data);

    /* Add 1st repication copy for list 1*/
    memset(&data, 0, sizeof(data));
    data.repl_type = TM_REPL_TYPE_HONOR_INGRESS;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_P4PT_COPY, &data);

    /* Add 2nd repication copy for list 1*/
    memset(&data, 0, sizeof(data));
    data.lport = 1008;  // SERVICE_LIF_P4PT as defined in nic/hal/src/proxy.hpp
    data.is_qid = 0;
    data.qid_or_vnid = 0;
    hal::pd::g_hal_state_pd->met_table()->add_replication(P4_NW_MCAST_INDEX_P4PT_COPY, &data);

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Chip Forwarding Mode Initialization
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_forwarding_mode_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
#if 0
    if (getenv("CAPRI_MOCK_MODE")) {
        return HAL_RET_OK;
    }
#endif
    uint64_t val, nic_mode = NIC_MODE_SMART;
    p4pd_table_properties_t       tbl_ctx;
    p4pd_table_properties_get(P4TBL_ID_INPUT_PROPERTIES, &tbl_ctx);
    sdk::platform::capri::capri_table_constant_read(&val, tbl_ctx.stage, tbl_ctx.stage_tableid,
                              (tbl_ctx.gress == P4_GRESS_INGRESS));
    val = be64toh(val);

    if (p4pd_def_cfg->hal_cfg->device_cfg.forwarding_mode == HAL_FORWARDING_MODE_CLASSIC) {
        nic_mode = NIC_MODE_CLASSIC;
    } else {
        // host-pinned & default
        nic_mode = NIC_MODE_SMART;
    }

    if (nic_mode == NIC_MODE_CLASSIC) {
        //val &= (uint64_t)0x1;
        val |= (uint64_t)~0;
        HAL_TRACE_DEBUG("Nic forwarding mode CLASSIC");
    } else {
        //val |= (uint64_t)~0x1;
        val = 0;
        HAL_TRACE_DEBUG("Nic forwarding mode SMART");
    }
    HAL_TRACE_DEBUG("Nic specific forwarding mode: {}",
                    p4pd_def_cfg->hal_cfg->device_cfg.forwarding_mode);
    val = htobe64(val);
    sdk::platform::capri::capri_table_constant_write(val, tbl_ctx.stage, tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    // Flow_info table is split into multiple threads in Capri so that
    // we can limit the # of phvwr to less than 8. Internally
    // multiple tables lookups will be launched with the same key to the
    // to same table and multiple MPU will be invoked. Table constant
    // value programmed for each thread will determine which chunk of
    // code to execute and this branching is done in ASM Code.
    // Here we need to program a different table constant for each
    // thread.
    uint8_t tid = 0;
    p4pd_table_properties_get(P4TBL_ID_FLOW_INFO, &tbl_ctx);
    for (int i = 0; i < tbl_ctx.table_thread_count; i++) {
        if (i != 0) {
            tid = tbl_ctx.thread_table_id[i];
        } else {
            tid = tbl_ctx.stage_tableid;
        }
        sdk::platform::capri::capri_table_constant_write(i, tbl_ctx.stage, tid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS));
        HAL_TRACE_DEBUG("setting flow_info table constant, tid = {}, constant = {}",
                        tid, i);
    }
 
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Flow hash table Initialization
//-----------------------------------------------------------------------------
static hal_ret_t
p4pd_flow_hash_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    p4pd_table_properties_t       tbl_ctx;
    uint64_t                      size = 0;

    p4pd_table_properties_get(P4TBL_ID_FLOW_HASH, &tbl_ctx);
    // Get the table size from hbm json to reset to 0
    size = get_mem_size_kb(tbl_ctx.tablename) << 10;
    HAL_TRACE_DEBUG("Table name: {} depth: {} Base virtual memory: {} Base Physical Memory: {} table_sz: {}",
            tbl_ctx.tablename, tbl_ctx.tabledepth, tbl_ctx.base_mem_va, tbl_ctx.base_mem_pa, size);
    sdk::lib::pal_mem_set(tbl_ctx.base_mem_pa, 0, size);
    p4pd_table_properties_get(tbl_ctx.oflow_table_id, &tbl_ctx);
    // Get the table size from hbm json to reset to 0
    size = get_mem_size_kb(tbl_ctx.tablename) << 10;
    HAL_TRACE_DEBUG("Table name: {} depth: {} Base virtual memory: {} Base Physical Memory: {} table_sz: {}",
          tbl_ctx.tablename, tbl_ctx.tabledepth, tbl_ctx.base_mem_va, tbl_ctx.base_mem_pa, size);
    sdk::lib::pal_mem_set(tbl_ctx.base_mem_pa, 0, size);

    return HAL_RET_OK;
}

/* *
 * Capri specific init functions
 * */

hal_ret_t
p4pd_table_defaults_init (p4pd_def_cfg_t *p4pd_def_cfg)
{
    // initialize all P4 ingress tables with default entries, if any
    SDK_ASSERT(p4pd_input_mapping_native_init(p4pd_def_cfg) == HAL_RET_OK);
    SDK_ASSERT(p4pd_input_mapping_tunneled_init(p4pd_def_cfg) == HAL_RET_OK);
    SDK_ASSERT(p4pd_l4_profile_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_flow_info_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_session_state_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_flow_stats_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_drop_stats_init() == HAL_RET_OK);
    //SDK_ASSERT(p4pd_ddos_policers_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_qos_init(p4pd_def_cfg->admin_cos) == HAL_RET_OK);

    // initialize all P4 egress tables with default entries, if any
    SDK_ASSERT(p4pd_tunnel_decap_copy_inner_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_twice_nat_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_rewrite_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_tunnel_encap_update_inner() == HAL_RET_OK);
    SDK_ASSERT(p4pd_tunnel_rewrite_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_decode_roce_opcode_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_p4plus_app_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_mirror_table_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_compute_checksum_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_egress_drop_stats_init() == HAL_RET_OK);
    SDK_ASSERT(p4pd_nacl_stats_init() == HAL_RET_OK);

    // initialize all PB/TM tables with default entries, if any
    // Even though this is not really a P4 Table it is very
    // tightly coupled with our P4 Program and after discussing
    // we put this call here conciously.
    SDK_ASSERT(capri_repl_pgm_def_entries() == HAL_RET_OK);

    // Setting NIC's forwarding mode
    SDK_ASSERT(p4pd_forwarding_mode_init(p4pd_def_cfg) == HAL_RET_OK);
    if (p4pd_def_cfg->hal_cfg->platform == platform_type_t::PLATFORM_TYPE_HW && 
        p4pd_def_cfg->hal_cfg->device_cfg.forwarding_mode == HAL_FORWARDING_MODE_SMART_HOST_PINNED) {
        SDK_ASSERT(p4pd_flow_hash_init(p4pd_def_cfg) == HAL_RET_OK);
    }

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

