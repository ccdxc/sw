#include <base.h>
#include <p4pd.h>
#include <p4pd_api.hpp>
#include <tcam.hpp>
#include <hal_state_pd.hpp>
#include <defines.h>
#include <common_defines.h>
#include <rdma_defines.h>
#include <table_sizes.h>

using hal::pd::utils::Tcam;

namespace hal {
namespace pd {

static hal_ret_t
p4pd_input_mapping_native_init (void)
{
    uint32_t                             idx;
    input_mapping_native_swkey_t         key;
    input_mapping_native_swkey_mask_t    mask;
    input_mapping_native_actiondata      data;
    hal_ret_t                            ret;
    Tcam                                 *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_INPUT_MAPPING_NATIVE);
    HAL_ASSERT(tcam != NULL);

    /* TODO: TEMP HACK FOR EXTRA ENTRY SINCE MODEL IS NOT LOOKING AT LAST INDEX!
     * WILL BE REMOVED ONCE MODEL FIX IS IN
     */
    // Temp extra catch-all entry
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NATIVE_IPV4_PACKET_ID;

    // insert into the tcam now - default entries are inserted bottom-up
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for IPv4 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.mpls_0_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NATIVE_IPV4_PACKET_ID;

    // insert into the tcam now - default entries are inserted bottom-up
    ret = tcam->insert(&key, &mask, &data, &idx, false);
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
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 1;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NATIVE_IPV6_PACKET_ID;

    // insert into the tcam now - default entries are inserted bottom-up
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
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
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NATIVE_NON_IP_PACKET_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // no-op entry for IPv4 transit packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NOP_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // no-op entry for IPv6 transit packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 1;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set the appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.mpls_0_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NOP_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping native tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping native tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // no-op entry for non-IP tunnel packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set the appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_NATIVE_NOP_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
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
p4pd_input_mapping_tunneled_init (void)
{
    uint32_t                             idx;
    input_mapping_tunneled_swkey_t       key;
    input_mapping_tunneled_swkey_mask_t  mask;
    input_mapping_tunneled_actiondata    data;
    hal_ret_t                            ret;
    Tcam                                 *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_INPUT_MAPPING_TUNNELED);
    HAL_ASSERT(tcam != NULL);

    /* TODO: TEMP HACK SINCE MODEL IS NOT LOOKING AT LAST INDEX!!
     * WILL BE REMOVED ONCE MODEL FIX IS IN
     */
    // extra catch-all entry for IPv4 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_NOP_ID;
    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);


    // no-op entry for IPv4 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0x1;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;
    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_NOP_ID;
    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // no-op entry for IPv6 native packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 1;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_NOP_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
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
    key.entry_status_inactive = 0;
    key.ipv4_valid = 0;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = 0;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_NOP_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for IPv4 transit packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    // TODO: Temp fix. Remove inner_ipv4_valid key
    key.inner_ipv4_valid = 1;
    key.inner_ipv6_valid = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0xFF;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_TUNNELED_IPV4_PACKET_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for IPv6 transit packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.inner_ipv4_valid = 0;
    key.inner_ipv6_valid = 1;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set appropriate mask for them
    mask.entry_status_inactive_mask = 0xFF;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_TUNNELED_IPV6_PACKET_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);

    // entry for non-IP tunnel packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // set the key bits that we care
    key.entry_status_inactive = 0;
    key.inner_ipv4_valid = 0;
    key.inner_ipv6_valid = 0;
    key.ipv4_valid = 1;
    key.ipv6_valid = 0;
    key.mpls_0_valid = 0;
    key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;

    // and set the appropriate mask for them
    mask.entry_status_inactive_mask = 0xFF;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.ipv4_valid_mask = 0xFF;
    mask.ipv6_valid_mask = 0xFF;
    mask.mpls_0_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    // set the action
    data.actionid = INPUT_MAPPING_TUNNELED_TUNNELED_NON_IP_PACKET_ID;

    // insert into the tcam now
    ret = tcam->insert(&key, &mask, &data, &idx, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Input mapping tunneled tcam write failure, "
                      "idx : {}, err : {}", idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Input mapping tunneled tcam write, "
                  "idx : {}, ret: {}", idx, ret);
    return HAL_RET_OK;
}

static hal_ret_t
p4pd_l4_profile_init (void)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    l4_profile_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_L4_PROFILE);
    HAL_ASSERT(dm != NULL);
    ret = dm->insert_withid(&data, 0);
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
    uint32_t                idx = 0;
    hal_ret_t               ret;
    DirectMap               *dm;
    flow_info_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    HAL_ASSERT(dm != NULL);

    // "catch-all" flow miss entry
    data.actionid = FLOW_INFO_FLOW_MISS_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow info table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    // common flow hit & drop entry
    ++idx;
    data.actionid = FLOW_INFO_FLOW_HIT_DROP_ID;
    data.flow_info_action_u.flow_info_flow_hit_drop.flow_index = 0;
    data.flow_info_action_u.flow_info_flow_hit_drop.start_timestamp = 0;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow info table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_session_state_init (void)
{
    uint32_t                 idx = 0;
    hal_ret_t                ret;
    DirectMap                *dm;
    session_state_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    HAL_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.actionid = SESSION_STATE_NOP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow state table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_flow_stats_init (void)
{
    uint32_t                 idx = 0;
    hal_ret_t                ret;
    DirectMap                *dm;
    flow_stats_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    HAL_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.actionid = FLOW_STATS_FLOW_STATS_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    // claim one more entry to be in sync with flow info table
    ++idx;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_p4plus_app_init (void)
{
    hal_ret_t                ret = HAL_RET_OK;
    DirectMap                *dm;
    p4plus_app_actiondata data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_P4PLUS_APP);
    HAL_ASSERT(dm != NULL);

    for (int i = P4PLUS_APP_TYPE_MIN; i <= P4PLUS_APP_TYPE_MAX; i++) {
        switch(i) {
            case P4PLUS_APPTYPE_CLASSIC_NIC:
                data.actionid = P4PLUS_APP_P4PLUS_APP_CLASSIC_NIC_ID;
                break;
            case P4PLUS_APPTYPE_RDMA:
                break;
            case P4PLUS_APPTYPE_TCPTLS:
                data.actionid = P4PLUS_APP_P4PLUS_APP_TCP_PROXY_ID;
                break;
            case P4PLUS_APPTYPE_IPSEC:
                data.actionid = P4PLUS_APP_P4PLUS_APP_IPSEC_ID;
                break;
            case P4PLUS_APPTYPE_NDE:
                break;
            case P4PLUS_APPTYPE_STORAGE:
                break;
            case P4PLUS_APPTYPE_TELEMETRY:
                break;
            case P4PLUS_APPTYPE_CPU:
                data.actionid = P4PLUS_APP_P4PLUS_APP_CPU_ID;
                break;
            default:
                HAL_TRACE_ERR("Unknown app_type: {}", i);
                HAL_ASSERT(0);
        }

        ret = dm->insert_withid(&data, i);
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
    DirectMap                             *dm;
    tunnel_decap_copy_inner_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_DECAP_COPY_INNER);
    HAL_ASSERT(dm != NULL);
    
    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, true, false, false);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV4_UDP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, true, false, false);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV4_OTHER_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, false, true, false);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV6_UDP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, true, false);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_IPV6_OTHER_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, true, false, true);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV4_UDP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, true, false, true);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV4_OTHER_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(true, false, true, true);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV6_UDP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, true, true);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_IPV6_OTHER_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_decap_copy_inner_tbl_idx(false, false, false, true);
    data.actionid = TUNNEL_DECAP_COPY_INNER_COPY_INNER_ETH_NON_IP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel decap copy inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_twice_nat_init (void)
{
    uint32_t                idx = 0;
    hal_ret_t               ret;
    DirectMap               *dm;
    twice_nat_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    HAL_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.actionid = TWICE_NAT_NOP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("twice nat table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_rewrite_init (void)
{
    uint32_t              idx = 0, decap_vlan_idx = 1;
    hal_ret_t             ret;
    DirectMap             *dm;
    rewrite_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    HAL_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.actionid = REWRITE_NOP_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rewrite table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    // "decap vlan" entry - 
    // - For L2 Mcast packets
    data.actionid = REWRITE_REWRITE_ID;
    ret = dm->insert_withid(&data, decap_vlan_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rewrite table write failure, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }

    g_hal_state_pd->set_rwr_tbl_decap_vlan_idx(decap_vlan_idx);
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
    DirectMap                             *dm;
    tunnel_encap_update_inner_actiondata  data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_ENCAP_UPDATE_INNER);
    HAL_ASSERT(dm != NULL);
    
    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, true, false, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_UDP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, true, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_TCP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, false, true);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_ICMP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(false, true, false, false, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV4_UNKNOWN_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, true, false, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_UDP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, true, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_TCP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, false, true);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_ICMP_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel encap update inner table write failure, "
                      "idx {}, err : {}", idx, ret);
        return ret;
    }

    idx = p4pd_get_tunnel_encap_update_inner_tbl_idx(true, false, false, false, false);
    data.actionid = TUNNEL_ENCAP_UPDATE_INNER_ENCAP_INNER_IPV6_UNKNOWN_REWRITE_ID;
    ret = dm->insert_withid(&data, idx);
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
    uint32_t                     noop_idx = 0, enc_vlan_idx = 1;
    hal_ret_t                    ret;
    DirectMap                    *dm;
    tunnel_rewrite_actiondata    data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TUNNEL_REWRITE);
    HAL_ASSERT(dm != NULL);

    // "catch-all" nop entry
    data.actionid = TUNNEL_REWRITE_NOP_ID;
    ret = dm->insert_withid(&data, noop_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, idx : {}, err : {}",
                      noop_idx, ret);
        return ret;
    }

    // "encap_vlan" entry
    data.actionid = TUNNEL_REWRITE_ENCAP_VLAN_ID;
    ret = dm->insert_withid(&data, enc_vlan_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tunnel rewrite table write failure, idx : {}, err : {}",
                      enc_vlan_idx, ret);
        return ret;
    }
    g_hal_state_pd->set_tnnl_rwr_tbl_encap_vlan_idx(enc_vlan_idx);
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
     (REQ_RX_FLAG_FIRST | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_AETH)}, //13 - read-response-first
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
    DirectMap                    *dm;
    decode_roce_opcode_actiondata data = { 0 };

    dm = g_hal_state_pd->dm_table(P4TBL_ID_DECODE_ROCE_OPCODE);
    HAL_ASSERT(dm != NULL);

    for (idx = 0; idx < DECODE_ROCE_OPCODE_TABLE_SIZE; idx++) {

        if (opc_to_info[idx].valid == 1) {

            // valid entry
            data.actionid = DECODE_ROCE_OPCODE_DECODE_ROCE_OPCODE_ID;
            data.decode_roce_opcode_action_u.decode_roce_opcode_decode_roce_opcode.qtype =
                opc_to_info[idx].type;
            data.decode_roce_opcode_action_u.decode_roce_opcode_decode_roce_opcode.len =
                opc_to_info[idx].roce_hdr_length;
            data.decode_roce_opcode_action_u.decode_roce_opcode_decode_roce_opcode.raw_flags =
                opc_to_info[idx].raw_flags;
        } else {
            // nop entry
            data.actionid = DECODE_ROCE_OPCODE_NOP_ID;
        }

        ret = dm->insert_withid(&data, idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("decode roce opcode table write failure, idx : {}, err : {}",
                          idx, ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
p4pd_table_defaults_init (void)
{
    // initialize all P4 ingress tables with default entries, if any
    HAL_ASSERT(p4pd_input_mapping_native_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_input_mapping_tunneled_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_l4_profile_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_flow_info_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_session_state_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_flow_stats_init() == HAL_RET_OK);

    // initialize all P4 egress tables with default entries, if any
    HAL_ASSERT(p4pd_tunnel_decap_copy_inner_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_twice_nat_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_rewrite_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_tunnel_encap_update_inner() == HAL_RET_OK);
    HAL_ASSERT(p4pd_tunnel_rewrite_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_decode_roce_opcode_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_p4plus_app_init() == HAL_RET_OK);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

