// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/iris/debug/debug_pd.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/p4pd_defaults.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_debug_cli_read(pd_func_args_t *pd_func_args)
{
    hal_ret_t    ret    = HAL_RET_OK;
    pd_debug_cli_read_args_t *args = pd_func_args->pd_debug_cli_read;
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint32_t tableid = args->tableid;
    uint32_t index = args->index;
    void     *swkey = args->swkey;
    void     *swkey_mask = args->swkey_mask;
    void     *actiondata = args->actiondata;

    pd_err = p4pd_global_entry_read(tableid,
                                    index,
                                    swkey,
                                    swkey_mask,
                                    actiondata);
    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware read fail",
                        __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    return ret;
}

hal_ret_t
pd_debug_cli_write(pd_func_args_t *pd_func_args)
{
    hal_ret_t    ret              = HAL_RET_OK;
    pd_debug_cli_write_args_t *args = pd_func_args->pd_debug_cli_write;
    p4pd_error_t pd_err           = P4PD_SUCCESS;
    uint32_t     hwkey_len        = 0;
    uint32_t     hwkeymask_len    = 0;
    uint32_t     hwactiondata_len = 0;
    void         *hwkey           = NULL;
    void         *hwkeymask       = NULL;
    uint32_t tableid = args->tableid;
    uint32_t index = args->index;
    void     *swkey = args->swkey;
    void     *swkey_mask = args->swkey_mask;
    void     *actiondata = args->actiondata;

    p4pd_hwentry_query(tableid, &hwkey_len,
                       &hwkeymask_len, &hwactiondata_len);

    HAL_TRACE_DEBUG("hwkeylen: {}, hwkeymask_len: {}",
                    hwkey_len, hwkeymask_len);

    // build hw key & mask
    hwkey      = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                            (hwkey_len     + 7)/8);

    hwkeymask  = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                            (hwkeymask_len + 7)/8);

    memset(hwkey,     0, (hwkey_len     + 7)/8);
    memset(hwkeymask, 0, (hwkeymask_len + 7)/8);

    pd_err = p4pd_hwkey_hwmask_build(tableid,
                                     swkey,
                                     swkey_mask,
                                     (uint8_t*)hwkey,
                                     (uint8_t*)hwkeymask);
    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware mask build fail", __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    pd_err = p4pd_global_entry_write(tableid,
                                     index,
                                     (uint8_t*)hwkey,
                                     (uint8_t*)hwkeymask,
                                     actiondata);

    if (pd_err != P4PD_SUCCESS) {
        HAL_TRACE_DEBUG("{}: Hardware write fail", __FUNCTION__);
        ret = HAL_RET_ERR;
    }

    if (hwkey)     HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkey);
    if (hwkeymask) HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkeymask);

    return ret;
}

hal_ret_t
pd_table_properties_get (pd_func_args_t *pd_func_args)
{
    p4pd_error_t            pd_err = P4PD_SUCCESS;
    hal::pd::pd_table_properties_get_args_t *args = pd_func_args->pd_table_properties_get;
    p4pd_table_properties_t tbl_ctx;

    memset(&tbl_ctx, 0 , sizeof(p4pd_table_properties_t));

    pd_err = p4pd_global_table_properties_get(args->tableid, &tbl_ctx);
    if (pd_err != P4PD_SUCCESS) {
        return HAL_RET_ERR;
    }

    args->tabledepth = tbl_ctx.tabledepth;
    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// linking PI <-> PD
//----------------------------------------------------------------------------
void
link_pi_pd(pd_fte_span_t *pd_fte_span, fte_span_t *pi_fte_span)
{
    pd_fte_span->fte_span = pi_fte_span;
    pi_fte_span->pd = pd_fte_span;
}

// ----------------------------------------------------------------------------
// un-linking PI <-> PD
// ----------------------------------------------------------------------------
void
delink_pi_pd(pd_fte_span_t *pd_fte_span, fte_span_t *pi_fte_span)
{
    if (pd_fte_span) {
        pd_fte_span->fte_span = NULL;
    }
    if (pi_fte_span) {
        pi_fte_span->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_fte_span_make_clone(pd_func_args_t *pd_func_args)
{
    hal_ret_t     ret = HAL_RET_OK;
    pd_fte_span_make_clone_args_t *args = pd_func_args->pd_fte_span_make_clone;
    pd_fte_span_t      *pd_fte_span_clone = NULL;
    fte_span_t         *fte_span, *clone;

    fte_span = args->fte_span;
    clone = args->clone;

    pd_fte_span_clone = fte_span_pd_alloc_init();
    if (pd_fte_span_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_fte_span_clone, fte_span->pd, sizeof(pd_fte_span_t));

    link_pi_pd(pd_fte_span_clone, clone);

end:
    return ret;
}

hal_ret_t
fte_span_pd_alloc_res (pd_fte_span_t *fte_span_pd)
{
    return HAL_RET_OK;
}

static hal_ret_t
nacl_stats_program_tbl (pd_fte_span_t *fte_span_pd, bool insert)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    directmap              *nacl_stats_tbl = NULL;
    nacl_stats_actiondata_t d = {0};

    nacl_stats_tbl = g_hal_state_pd->dm_table(P4TBL_ID_NACL_STATS);
    SDK_ASSERT_RETURN((nacl_stats_tbl != NULL), HAL_RET_ERR);

    // populate the action information
    d.action_id = NACL_STATS_NACL_STATS_ID; 

    // Initialize the stats
    d.action_u.nacl_stats_nacl_stats.stats_packets = 0;

    if (insert) {
        sdk_ret = nacl_stats_tbl->insert(&d, &fte_span_pd->stats_index);
    } else {
        sdk_ret = nacl_stats_tbl->remove(fte_span_pd->stats_index);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nacl stats table write failure, ret {}",
                      ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Nacl Stats : Stats idx: {} programmed",
                    fte_span_pd->stats_index);
    return HAL_RET_OK;
}

hal_ret_t
fte_span_pd_program_hw (pd_fte_span_t *fte_span_pd, table_oper_t oper)
{
    sdk_ret_t           sdk_ret = SDK_RET_OK;
    hal_ret_t           ret = HAL_RET_OK;
    nacl_swkey_t        key;
    nacl_swkey_mask_t   mask;
    nacl_actiondata_t   data;
    fte_span_t          *fte_span = fte_span_pd->fte_span;
    acl_tcam            *acl_tbl = NULL;
    uint64_t            drop_reason_mask = ~0;
    mirror_actiondata_t mirr_data = { 0 };
    directmap           *mirr_dm;

    acl_tbl = g_hal_state_pd->acl_table();
    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    mirr_dm = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);
    SDK_ASSERT(mirr_dm != NULL);

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));


    if (!fte_span->sel) {
        // Nothing to match. Eff. disable the entry
        key.entry_inactive_nacl = 1;
        mask.entry_inactive_nacl_mask = 0x1;

        // Remove mirror session
        if (fte_span_pd->mirr_sess_id != INVALID_INDEXER_INDEX) {
            sdk_ret = mirr_dm->remove(fte_span_pd->mirr_sess_id);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to deprogram mirror session. id: {}, ret: {}",
                              fte_span_pd->mirr_sess_id, ret);
            } else {
                HAL_TRACE_ERR("Deprogrammed mirror session. id: {}",
                              fte_span_pd->mirr_sess_id);
            }
            fte_span_pd->mirr_sess_id = INVALID_INDEXER_INDEX;
        }
#if 0
        // Go back to original mirror session
        mirr_data.action_id = MIRROR_LOCAL_SPAN_ID;
        mirr_data.action_u.mirror_local_span.dst_lport = CPU_LPORT;
        mirr_data.action_u.mirror_local_span.qid_en = 1;
        mirr_data.action_u.mirror_local_span.qid = types::CPUCB_ID_FTE_SPAN;
#endif

        if (fte_span_pd->stats_index) {
            // Clean up the stats programming
            nacl_stats_program_tbl(fte_span_pd, false);
            fte_span_pd->stats_index = 0; 
        }


        // Only for Testing, uncomment so that every packet is spanned to FTE
#if 0
        key.entry_inactive_nacl = 0;
        mask.entry_inactive_nacl_mask = 0x1;

        data.action_id = NACL_NACL_PERMIT_ID;
        data.action_u.nacl_nacl_permit.ingress_mirror_en = 1;
        data.action_u.nacl_nacl_permit.ingress_mirror_session_id = 0x1 << 7;
#endif
    } else {
        key.entry_inactive_nacl = 0;
        mask.entry_inactive_nacl_mask = 0x1;

        // If MATCH_ANY, dont set anything in the mask
        if (!(fte_span->sel & (1 << types::MATCH_ANY))) {
            if (fte_span->sel & (1 << types::SRC_LIF)) {
                key.capri_intrinsic_lif = fte_span->src_lif;
                mask.capri_intrinsic_lif_mask =
                    ~(mask.capri_intrinsic_lif_mask & 0);
            }

            if (fte_span->sel & (1 << types::SRC_LPORT)) {
                key.control_metadata_src_lport = fte_span->src_lport;
                mask.control_metadata_src_lport_mask =
                    ~(mask.control_metadata_src_lport_mask & 0);
            }

            if (fte_span->sel & (1 << types::DST_LPORT)) {
                key.control_metadata_dst_lport = fte_span->dst_lport;
                mask.control_metadata_dst_lport_mask =
                    ~(mask.control_metadata_dst_lport_mask & 0);
            }

            if (fte_span->sel & (1 << types::DROP_REASON)) {
                memcpy(key.control_metadata_drop_reason, &fte_span->drop_reason,
                       sizeof(key.control_metadata_drop_reason));
                memcpy(mask.control_metadata_drop_reason_mask, &drop_reason_mask,
                       sizeof(mask.control_metadata_drop_reason_mask));
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_DIR)) {
                key.flow_lkp_metadata_lkp_dir = fte_span->flow_lkup_dir;
                mask.flow_lkp_metadata_lkp_dir_mask =
                    ~(mask.flow_lkp_metadata_lkp_dir_mask & 0);
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_TYPE)) {
                key.flow_lkp_metadata_lkp_type = fte_span->flow_lkup_type;
                mask.flow_lkp_metadata_lkp_type_mask =
                    ~(mask.flow_lkp_metadata_lkp_type_mask & 0);
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_VRF)) {
                key.flow_lkp_metadata_lkp_vrf = fte_span->flow_lkup_vrf;
                mask.flow_lkp_metadata_lkp_vrf_mask =
                    ~(mask.flow_lkp_metadata_lkp_vrf_mask & 0);
            }

            // TODO: Fix for Ipv6. Check acl_pd.cc. Also check session_pd.cc p4pd_add_flow_hash_table_entry
            if (fte_span->sel & (1 << types::FLOW_LKUP_SRC)) {
                memcpy(key.flow_lkp_metadata_lkp_src, fte_span->flow_lkup_src.v6_addr.addr8,
                       IP6_ADDR8_LEN);
                memset(mask.flow_lkp_metadata_lkp_src_mask, 0xFF, IP6_ADDR8_LEN);
            }

            // TODO: Fix for Ipv6. Check acl_pd.cc. Also check session_pd.cc p4pd_add_flow_hash_table_entry
            if (fte_span->sel & (1 << types::FLOW_LKUP_DST)) {
                memcpy(key.flow_lkp_metadata_lkp_dst, fte_span->flow_lkup_dst.v6_addr.addr8,
                       IP6_ADDR8_LEN);
                memset(mask.flow_lkp_metadata_lkp_dst_mask, 0xFF, IP6_ADDR8_LEN);
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_PROTO)) {
                key.flow_lkp_metadata_lkp_proto = fte_span->flow_lkup_proto;
                mask.flow_lkp_metadata_lkp_proto_mask =
                    ~(mask.flow_lkp_metadata_lkp_proto_mask & 0);
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_SPORT)) {
                key.flow_lkp_metadata_lkp_sport = fte_span->flow_lkup_sport;
                mask.flow_lkp_metadata_lkp_sport_mask =
                    ~(mask.flow_lkp_metadata_lkp_sport_mask & 0);
            }

            if (fte_span->sel & (1 << types::FLOW_LKUP_DPORT)) {
                key.flow_lkp_metadata_lkp_dport = fte_span->flow_lkup_dport;
                mask.flow_lkp_metadata_lkp_dport_mask =
                    ~(mask.flow_lkp_metadata_lkp_dport_mask & 0);
            }

            if (fte_span->sel & (1 << types::ETH_DMAC)) {
                memcpy(key.ethernet_dstAddr,
                       &fte_span->eth_dmac, sizeof(mac_addr_t));
                memcpy(mask.ethernet_dstAddr_mask, &drop_reason_mask,
                       sizeof(mask.ethernet_dstAddr_mask));
            }

            if (fte_span->sel & (1 << types::FROM_CPU)) {
                key.control_metadata_from_cpu = fte_span->from_cpu;
                mask.control_metadata_from_cpu_mask =
                    ~(mask.control_metadata_from_cpu_mask & 0);
            }

        }

        if (fte_span_pd->stats_index) {
            // Clean up the stats programming
            nacl_stats_program_tbl(fte_span_pd, false);
            fte_span_pd->stats_index = 0;
        }
 
        if (fte_span->attach_stats) {
            // Program nacl_stats table
            nacl_stats_program_tbl(fte_span_pd, true);
            data.action_u.nacl_nacl_permit.stats_idx = (uint16_t)fte_span_pd->stats_index;
        } 

        data.action_id = NACL_NACL_PERMIT_ID;
        if (fte_span->span_lport) {
            mirr_data.action_id = MIRROR_LOCAL_SPAN_ID;
            mirr_data.action_u.mirror_local_span.dst_lport = fte_span->span_lport;
            mirr_data.action_u.mirror_local_span.qid_en = 0;
            mirr_data.action_u.mirror_local_span.qid = 0;
        } else {
            mirr_data.action_id = MIRROR_LOCAL_SPAN_ID;
            mirr_data.action_u.mirror_local_span.dst_lport = CPU_LPORT;
            mirr_data.action_u.mirror_local_span.qid_en = 1;
            mirr_data.action_u.mirror_local_span.qid = types::CPUCB_ID_FTE_SPAN;
        }

        if (fte_span_pd->mirr_sess_id != INVALID_INDEXER_INDEX) {
            // Updating mirror session
            sdk_ret = mirr_dm->update(fte_span_pd->mirr_sess_id, &mirr_data);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("mirror table initialization failed for idx : {}, err : {}",
                              fte_span_pd->mirr_sess_id, ret);
                return ret;
            } else {
                HAL_TRACE_DEBUG("mirror session updated for id: {}",
                                fte_span_pd->mirr_sess_id);
            }
        } else {
            sdk_ret = mirr_dm->insert(&mirr_data, &fte_span_pd->mirr_sess_id);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("mirror table initialization failed err : {}", ret);
                return ret;
            } else {
                HAL_TRACE_DEBUG("mirror session installed for id: {}",
                                fte_span_pd->mirr_sess_id);
            }
        }
        if (fte_span->is_egress) {
            data.action_u.nacl_nacl_permit.egress_mirror_en = 1;
            data.action_u.nacl_nacl_permit.egress_mirror_session_id = 0x1 << fte_span_pd->mirr_sess_id;
        } else {
            data.action_u.nacl_nacl_permit.ingress_mirror_en = 1;
            data.action_u.nacl_nacl_permit.ingress_mirror_session_id = 0x1 << fte_span_pd->mirr_sess_id;
        }
    }


    if (oper == TABLE_OPER_INSERT) {
        ret = acl_tbl->insert(&key, &mask, &data, ACL_FTE_SPAN_PRIORITY, 
                              &fte_span_pd->handle);
    } else {
        // Update
        ret = acl_tbl->update(fte_span_pd->handle, &key, &mask, &data, ACL_FTE_SPAN_PRIORITY);
    }

    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Programmed nacl for FTE Span.");
    } else {
        HAL_TRACE_ERR("Unable to program nacl for FTE Span. ret: {}", ret);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_fte_span_mem_free(pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_fte_span_mem_free_args_t *args = pd_func_args->pd_fte_span_mem_free;
    pd_fte_span_t    *fte_span_pd;

    fte_span_pd = (pd_fte_span_t *)args->fte_span->pd;
    fte_span_pd_mem_free(fte_span_pd);

    return ret;
}

hal_ret_t
fte_span_pd_cleanup(pd_fte_span_t *fte_span_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!fte_span_pd) {
        goto end;
    }

    // delinking PI<->PD
    delink_pi_pd(fte_span_pd, (fte_span_t *)fte_span_pd->fte_span);

    // freeing PD
    fte_span_pd_free(fte_span_pd);
end:
    return ret;
}

hal_ret_t
pd_fte_span_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                  ret;
    pd_fte_span_create_args_t *args = pd_func_args->pd_fte_span_create;
    pd_fte_span_t             *fte_span_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);

    // allocate PD fte_span state
    fte_span_pd = fte_span_pd_alloc_init();
    if (fte_span_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    link_pi_pd(fte_span_pd, args->fte_span);

    // allocate resources
    ret = fte_span_pd_alloc_res(fte_span_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocated resources");
        goto end;
    }

    // program hw
    ret = fte_span_pd_program_hw(fte_span_pd, TABLE_OPER_INSERT);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw");
        goto end;
    }

    *args->stats_index = fte_span_pd->stats_index;

end:
    if (ret != HAL_RET_OK) {
        fte_span_pd_cleanup(fte_span_pd);
    }

    return ret;
}

hal_ret_t
pd_fte_span_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t                   ret;
    pd_fte_span_update_args_t   *args = pd_func_args->pd_fte_span_update;
    pd_fte_span_t               *fte_span_pd_clone;

    fte_span_pd_clone = (pd_fte_span_t *)args->fte_span_clone->pd;

    ret = fte_span_pd_program_hw(fte_span_pd_clone, TABLE_OPER_UPDATE);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update hw");
        goto end;
    }

    *args->stats_index = fte_span_pd_clone->stats_index;

end:
    if (ret != HAL_RET_OK) {
        fte_span_pd_cleanup(fte_span_pd_clone);
    }

    return ret;
}

hal_ret_t
pd_fte_span_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd_fte_span_get_args_t     *args = pd_func_args->pd_fte_span_get;
    pd_fte_span_t               *fte_span_pd;

    fte_span_pd = (pd_fte_span_t *)args->fte_span->pd;

    *args->stats_index = fte_span_pd->stats_index;

    return ret;
}

}    // namespace pd
}    // namespace hal
