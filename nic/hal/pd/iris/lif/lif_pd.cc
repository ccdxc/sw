#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/hal/pd/iris/nw/enicif_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/eth.hpp"
#include "nic/sdk/asic/pd/scheduler.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "gen/proto/nicmgr/metrics.delphi.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

static void
pd_lif_copy_asicpd_params (asicpd_scheduler_lif_params_t *out, pd_lif_t *lif_pd)
{
    lif_t *lif = (lif_t *)lif_pd->pi_lif;

    out->lif_id = lif_get_lif_id(lif);
    out->tx_sched_table_offset = lif_pd->tx_sched_table_offset;
    out->tx_sched_num_table_entries = lif_pd->tx_sched_num_table_entries;

    if (lif->qstate_pgm_in_hal) {
        out->total_qcount = lif_get_total_qcount(lif_pd->hw_lif_id);
    } else {
        // TODO: The tx scheduler programming  should eventually be moved to nicmgr.
        // For hw take it from pi lif
        out->total_qcount = ((lif_t *)lif_pd->pi_lif)->qcount;
        HAL_TRACE_DEBUG("lif_hw_id: {} Lif's Qcount: {}",
                        lif_pd->hw_lif_id, out->total_qcount);
    }
    out->hw_lif_id = lif_pd->hw_lif_id;
    out->cos_bmp = ((lif_t *)lif_pd->pi_lif)->qos_info.cos_bmp;
    return;
}

static void
pd_lif_copy_asicpd_sched_params (pd_lif_t *lif_pd, asicpd_scheduler_lif_params_t *apd_lif)
{
    lif_pd->tx_sched_table_offset = apd_lif->tx_sched_table_offset;
    lif_pd->tx_sched_num_table_entries = apd_lif->tx_sched_num_table_entries;
    return;
}

//-----------------------------------------------------------------------------
// Lif Create in PD
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t            ret;
    pd_lif_create_args_t *args = pd_func_args->pd_lif_create;
    pd_lif_t             *pd_lif;

    // Create lif PD
    pd_lif = lif_pd_alloc_init();
    if (pd_lif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_lif, args->lif);

    // Allocate Resources
    ret = lif_pd_alloc_res(pd_lif, args);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        goto end;
    }

    // Program HW
    ret = lif_pd_program_hw(pd_lif);

end:
    if (ret != HAL_RET_OK) {
        lif_pd_cleanup(pd_lif);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// PD Lif Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_lif_update_args_t *args = pd_func_args->pd_lif_update;
    lif_t               *lif = args->lif;
    lif_t               *lif_clone = args->lif_clone;
    pd_lif_t            *pd_lif = (pd_lif_t *)args->lif->pd_lif;
    pd_lif_t            *pd_lif_clone = (pd_lif_t *)args->lif_clone->pd_lif;
    if_t                *enic_if = NULL;
    sdk_ret_t sdk_ret;


    if (args->rx_policer_changed) {
        ret = lif_pd_rx_policer_program_hw((pd_lif_t *)lif_clone->pd_lif, true);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }

    // Process VLAN offload config changes
    if (args->vlan_strip_en_changed) {
        // Program output mapping table
        ret = lif_pd_pgm_output_mapping_tbl(pd_lif,
                                            args, TABLE_OPER_UPDATE);
        if (ret != HAL_RET_OK) {
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    if (args->pkt_filter_prom_changed) {
        ret = pd_lif_handle_promiscous_filter_change(lif, args, false);
    }

    if (args->rdma_sniff_en_changed) {
        enic_if = pd_lif_get_enic(lif->lif_id);
        if (args->rdma_sniff_en) {
            ret = pd_lif_install_rdma_sniffer(pd_lif_clone, enic_if);
        } else {
            ret = pd_lif_uninstall_rdma_sniffer(pd_lif_clone);
        }
    }

    // Process ETH RSS configuration changes
    if (args->rss_config_changed) {
        ret = eth_rss_init(pd_lif->hw_lif_id, &lif_clone->rss,
            (lif_queue_info_t *)&lif->qinfo);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for RSS", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    asicpd_scheduler_lif_params_t apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    if (args->qstate_map_init_set) {
        sdk_ret = asicpd_tx_scheduler_map_alloc(&apd_lif);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            goto end;
        }
       // Copy scheduler info back to pd_lif.
       pd_lif_copy_asicpd_sched_params(pd_lif, &apd_lif);

       sdk_ret = asicpd_tx_scheduler_map_program(&apd_lif);
       ret = hal_sdk_ret_to_hal_ret(sdk_ret);
       if (ret != HAL_RET_OK) {
           goto end;
       }
    }

    if (args->tx_policer_changed || args->qstate_map_init_set) {
        HAL_TRACE_DEBUG("pd-lif:{}: tx policer changed ", __FUNCTION__);

        ret = lif_pd_tx_policer_program_hw((pd_lif_t *)lif_clone->pd_lif, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx policer", __FUNCTION__);
            goto end;
        }
    }


end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Lif Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_delete_args_t *args = pd_func_args->pd_lif_delete;
    pd_lif_t       *lif_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->lif != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->lif->pd_lif != NULL), HAL_RET_INVALID_ARG);
    lif_pd = (pd_lif_t *)args->lif->pd_lif;

    // Deprogram
    ret = lif_pd_deprogram_hw(lif_pd);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Cleanup
    ret = lif_pd_cleanup(lif_pd);
    if (ret != HAL_RET_OK) {
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources for PD Lif
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_alloc_res(pd_lif_t *pd_lif, pd_lif_create_args_t *args)
{
    sdk_ret_t           sdk_ret;
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;
    lif_t                *lif = (lif_t *)pd_lif->pi_lif;


#if 0
    if (args->with_hw_lif_id) {
        pd_lif->hw_lif_id = args->hw_lif_id;
    } else {
        // Allocate lif hwid
        rs = g_hal_state_pd->lif_hwid_idxr()->alloc((uint32_t *)&pd_lif->hw_lif_id);
        if (rs != indexer::SUCCESS) {
            return HAL_RET_NO_RESOURCE;
        }
    }
#endif

    if (args->with_hw_lif_id) {
        pd_lif->hw_lif_id = args->hw_lif_id;
    } else {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},hw_lif_id has to be allocated in PI",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
        SDK_ASSERT(0);
    }

    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{} allocated hw_lif_id:{}",
                    __FUNCTION__,
                    lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                    pd_lif->hw_lif_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_lif->
            lif_lport_id);
    if (rs != indexer::SUCCESS) {
        pd_lif->lif_lport_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }
    
    if (lif->type != types::LIF_TYPE_SWM) { 
        asicpd_scheduler_lif_params_t apd_lif;
        pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
        //Allocate tx scheduler resource for this lif if qstate-map init is done.
        if (args->lif->qstate_init_done) {
            sdk_ret = asicpd_tx_scheduler_map_alloc(&apd_lif);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                goto end;
            }
            // Copy scheduler-info back to pd_lif.
            pd_lif_copy_asicpd_sched_params(pd_lif, &apd_lif);
        }
    }

end:

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources.
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_dealloc_res(pd_lif_t *lif_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;
    asicpd_scheduler_lif_params_t   apd_lif;
    sdk_ret_t sdk_ret;


    pd_lif_copy_asicpd_params(&apd_lif, lif_pd);
    if (lif_pd->lif_lport_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->lport_idxr()->free(lif_pd->lif_lport_id);
        if (rs != indexer::SUCCESS) {
            ret = HAL_RET_INVALID_OP;
            goto end;
        }
    }

    if (lif_pd->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
        sdk_ret = asicpd_tx_scheduler_map_free(&apd_lif);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            ret = HAL_RET_INVALID_OP;
            goto end;
        }
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD lif Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD lif
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_cleanup(pd_lif_t *lif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!lif_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = lif_pd_dealloc_res(lif_pd);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(lif_pd, (lif_t *)lif_pd->pi_lif);

    // Freeing PD
    lif_pd_free(lif_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate and Initialize Lif PD Instance
//-----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_alloc_init (void)
{
    return lif_pd_init(lif_pd_alloc());
}

//-----------------------------------------------------------------------------
// Allocate LIF Instance
//-----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_alloc (void)
{
    pd_lif_t    *lif;

    lif = (pd_lif_t *)g_hal_state_pd->lif_pd_slab()->alloc();
    if (lif == NULL) {
        return NULL;
    }
    return lif;
}

//-----------------------------------------------------------------------------
// Initialize LIF PD instance
//-----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_init (pd_lif_t *lif)
{
    // Nothing to do currently
    if (!lif) {
        return NULL;
    }
    // Set here if you want to initialize any fields
    lif->tx_sched_table_offset = INVALID_INDEXER_INDEX;
    lif->host_mgmt_acl_handle = INVALID_INDEXER_INDEX;
    lif->rdma_sniff_mirr_idx = INVALID_INDEXER_INDEX;
    lif->tx_handle = INVALID_INDEXER_INDEX;
    lif->rx_handle = INVALID_INDEXER_INDEX;

    return lif;
}

//-----------------------------------------------------------------------------
// Program HW
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_program_hw (pd_lif_t *pd_lif)
{
    hal_ret_t   ret = HAL_RET_OK;
    lif_t       *lif = (lif_t *)pd_lif->pi_lif;
    sdk_ret_t   sdk_ret = SDK_RET_OK;
    if_t        *enic_if = NULL;


    if (lif->type == types::LIF_TYPE_SWM) {
        goto end;
    }

    // Program the rx-policer.
    ret = lif_pd_rx_policer_program_hw(pd_lif, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for rx policer");
        goto end;
    }

    // Program output mapping table
    ret = lif_pd_pgm_output_mapping_tbl(pd_lif, NULL, TABLE_OPER_INSERT);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw");
        goto end;
    }

    // Program RDMA sniffer
    enic_if = pd_lif_get_enic(lif->lif_id);
    if (lif->rdma_sniff_en) {
        ret = pd_lif_install_rdma_sniffer(pd_lif, enic_if);
    }

    // Program TX scheduler and Policer.
    asicpd_scheduler_lif_params_t   apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    sdk_ret = asicpd_tx_scheduler_map_program(&apd_lif);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for tx scheduler");
        goto end;
    }


    ret = lif_pd_tx_policer_program_hw(pd_lif, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for tx policer");
        goto end;
    }

    // ETH RSS configuration
    ret = eth_rss_init(pd_lif->hw_lif_id, &lif->rss,
                       (lif_queue_info_t *)&lif->qinfo);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for RSS. err: {}", ret);
        ret = HAL_RET_ERR;
        goto end;
    }

#if 0
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED &&
        lif->type == types::LIF_TYPE_HOST_MANAGEMENT) {
        ret = pd_lif_pgm_host_mgmt(pd_lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program NACL to prevent "
                          "non-classic traffic on host-mgmt. ret: {}", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }
#endif

end:
    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram HW
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    lif_t                           *lif = (lif_t *)pd_lif->pi_lif;
    asicpd_scheduler_lif_params_t   apd_lif;

    if (lif->type == types::LIF_TYPE_SWM) {
        goto end;
    }

    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    // Deprogram output mapping table
    ret = lif_pd_depgm_output_mapping_tbl(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
        goto end;
    }

    sdk_ret = asicpd_tx_scheduler_map_cleanup(&apd_lif);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw for tx scheduler");
        goto end;
    }

    ret = lif_pd_rx_policer_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw for rx policer");
        goto end;
    }

    ret = lif_pd_tx_policer_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw for tx policer");
        goto end;
    }

#if 0
    ret = pd_lif_depgm_host_mgmt(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram NACL to prevent "
                      "non-classic traffic on host-mgmt. ret: {}", ret);
        goto end;
    }
#endif

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Get vlan strip enable
//-----------------------------------------------------------------------------
bool
pd_lif_get_vlan_strip_en (lif_t *lif, pd_lif_update_args_t *args)
{
    if (args && args->vlan_strip_en_changed) {
        return args->vlan_strip_en;
    }
    return lif->vlan_strip_en;
}

if_t *
pd_lif_get_enic(lif_id_t lif_id)
{
    lif_t           *lif = NULL;
    if_t            *hal_if = NULL;
    dllist_ctxt_t   *lnode = NULL;
    hal_handle_id_list_entry_t  *entry  = NULL;

    lif = find_lif_by_id(lif_id);
    if (lif) {
        dllist_for_each(lnode, &lif->if_list_head) {
            entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
            hal_if = find_if_by_handle(entry->handle_id);
            return hal_if;
        }
    }

    return NULL;
}

uint32_t
pd_lif_get_enic_lport(lif_id_t lif_id)
{
    lif_t           *lif = NULL;
    if_t            *hal_if = NULL;
    dllist_ctxt_t   *lnode = NULL;
    hal_handle_id_list_entry_t  *entry  = NULL;

    lif = find_lif_by_id(lif_id);
    if (lif) {
        SDK_ASSERT(dllist_count(&lif->if_list_head) == 1);
        dllist_for_each(lnode, &lif->if_list_head) {
            entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
            hal_if = find_if_by_handle(entry->handle_id);
            return if_get_lport_id(hal_if);
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Handles promiscous filter change
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_handle_promiscous_filter_change (lif_t *lif,
                                        pd_lif_update_args_t *args,
                                        bool skip_hw_pgm)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry  = NULL;
    if_t                        *hal_if = NULL;
    dllist_ctxt_t               *lnode = NULL;

    dllist_for_each(lnode, &lif->if_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_if = find_if_by_handle(entry->handle_id);

        ret = pd_enicif_update_num_prom_lifs(hal_if,
                                             args->receive_promiscous ? true : false,
                                             skip_hw_pgm);

    }

    return ret;
}



#define TX_POLICER_ACTION(_d, _arg) _d.action_u.tx_table_s5_t4_lif_rate_limiter_table_tx_stage5_lif_egress_rl_params._arg
hal_ret_t
lif_pd_tx_policer_program_hw (pd_lif_t *pd_lif, bool update)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    lif_t                 *pi_lif = (lif_t *)pd_lif->pi_lif;
    directmap             *tx_policer_tbl = NULL;
    tx_table_s5_t4_lif_rate_limiter_table_actiondata_t d = {0};
    tx_table_s5_t4_lif_rate_limiter_table_actiondata_t d_mask = {0};
    uint64_t refresh_interval_us = HAL_DEFAULT_POLICER_REFRESH_INTERVAL;
    uint64_t rate_tokens = 0;
    uint64_t burst_tokens = 0;
    uint64_t rate;

    tx_policer_tbl = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE);
    SDK_ASSERT_RETURN((tx_policer_tbl != NULL), HAL_RET_ERR);

    d.action_id = TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE_TX_STAGE5_LIF_EGRESS_RL_PARAMS_ID;

    rate = pi_lif->qos_info.tx_policer.rate;

    if (rate == 0) {
        TX_POLICER_ACTION(d, entry_valid) = 0;
    } else {
        TX_POLICER_ACTION(d, entry_valid) = 1;
        TX_POLICER_ACTION(d, pkt_rate) =
            pi_lif->qos_info.tx_policer.type == POLICER_TYPE_PPS ? 1 : 0;
        TX_POLICER_ACTION(d, rlimit_en) = 1;

        ret = policer_to_token_rate(&pi_lif->qos_info.tx_policer,
                                    refresh_interval_us,
                                    &rate_tokens, &burst_tokens);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error converting rate to token rate ret {}", ret);
            return ret;
        }

        memcpy(TX_POLICER_ACTION(d, burst), &burst_tokens,
               std::min(sizeof(TX_POLICER_ACTION(d, burst)), sizeof(burst_tokens)));
        memcpy(TX_POLICER_ACTION(d, rate), &rate_tokens,
               std::min(sizeof(TX_POLICER_ACTION(d, rate)), sizeof(rate_tokens)));
    }

    memset(&d_mask.action_u.tx_table_s5_t4_lif_rate_limiter_table_tx_stage5_lif_egress_rl_params,
           0xff,
           sizeof(tx_table_s5_t4_lif_rate_limiter_table_tx_stage5_lif_egress_rl_params_t));
    TX_POLICER_ACTION(d_mask, rsvd) = 0;
    TX_POLICER_ACTION(d_mask, axi_wr_pend) = 0;
    if (update) {
        memset(TX_POLICER_ACTION(d_mask, tbkt), 0,
               sizeof(TX_POLICER_ACTION(d_mask, tbkt)));
    }

    if (update) {
        sdk_ret = tx_policer_tbl->update(pd_lif->hw_lif_id, &d, &d_mask);
    } else {
        sdk_ret = tx_policer_tbl->insert_withid(&d, pd_lif->hw_lif_id, &d_mask);
    }

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}: tx policer table write failure, lif {}, ret {}",
                      __FUNCTION__, lif_get_lif_id(pi_lif), ret);
        return ret;
    }

    asicpd_scheduler_lif_params_t apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    // Program mapping from rate-limiter-table to scheduler-table rate-limiter-group (RLG) for pausing.
    sdk_ret = asicpd_tx_policer_program(&apd_lif);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx policer", __FUNCTION__);
        goto end;
    }

    HAL_TRACE_DEBUG("Lif: {} hw_lif_id: {} policer: {} programmed",
                    lif_get_lif_id(pi_lif),
                    pd_lif->hw_lif_id,
                    pi_lif->qos_info.tx_policer);
end:
    return ret;
}

hal_ret_t
lif_pd_tx_policer_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    directmap             *tx_policer_tbl = NULL;

    tx_policer_tbl = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S5_T4_LIF_RATE_LIMITER_TABLE);
    SDK_ASSERT_RETURN((tx_policer_tbl != NULL), HAL_RET_ERR);

    sdk_ret = tx_policer_tbl->remove(pd_lif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},unable to deprogram tx policer table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},deprogrammed tx policer table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }

    return ret;
}

#define RX_POLICER_ACTION(_d, _arg) _d.action_u.rx_policer_execute_rx_policer._arg
static hal_ret_t
lif_pd_rx_policer_tbl_program_hw (pd_lif_t *pd_lif, bool update)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    lif_t                 *pi_lif = (lif_t *)pd_lif->pi_lif;
    directmap             *rx_policer_tbl = NULL;
    rx_policer_actiondata_t d = {0};
    rx_policer_actiondata_t d_mask = {0};
    uint64_t              refresh_interval_us =
                                        HAL_DEFAULT_POLICER_REFRESH_INTERVAL;
    uint64_t              rate_tokens = 0;
    uint64_t              burst_tokens = 0;
    uint64_t              rate;

    rx_policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER);
    SDK_ASSERT_RETURN((rx_policer_tbl != NULL), HAL_RET_ERR);

    d.action_id = RX_POLICER_EXECUTE_RX_POLICER_ID;

    rate = pi_lif->qos_info.rx_policer.rate;

    if (rate == 0) {
        RX_POLICER_ACTION(d, entry_valid) = 0;
    } else {
        RX_POLICER_ACTION(d, entry_valid) = 1;
        RX_POLICER_ACTION(d, pkt_rate) =
            pi_lif->qos_info.rx_policer.type == POLICER_TYPE_PPS ? 1 : 0;

        ret = policer_to_token_rate(&pi_lif->qos_info.rx_policer,
                                    refresh_interval_us,
                                    &rate_tokens, &burst_tokens);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error converting rate to token rate ret {}", ret);
            return ret;
        }

        memcpy(RX_POLICER_ACTION(d, burst), &burst_tokens,
               std::min(sizeof(RX_POLICER_ACTION(d, burst)),
                        sizeof(burst_tokens)));
        memcpy(RX_POLICER_ACTION(d, rate), &rate_tokens,
               std::min(sizeof(RX_POLICER_ACTION(d, rate)), sizeof(rate_tokens)));

        uint64_t tbkt = burst_tokens;
        memcpy(RX_POLICER_ACTION(d, tbkt), &tbkt,
               std::min(sizeof(RX_POLICER_ACTION(d, tbkt)),
                        sizeof(burst_tokens)));
    }

    memset(&d_mask.action_u.rx_policer_execute_rx_policer, 0xff,
           sizeof(rx_policer_execute_rx_policer_t));
    RX_POLICER_ACTION(d_mask, rsvd) = 0;
    RX_POLICER_ACTION(d_mask, axi_wr_pend) = 0;
    if (update) {
        memset(RX_POLICER_ACTION(d_mask, tbkt), 0,
               sizeof(RX_POLICER_ACTION(d_mask, tbkt)));
    }

    HAL_TRACE_DEBUG("lif {} hw_lif_id {} rx_policer {}"
                    "rate_tokens {} burst_tokens {} programmed",
                    lif_get_lif_id(pi_lif),
                    pd_lif->hw_lif_id,
                    pi_lif->qos_info.rx_policer,
                    rate_tokens, burst_tokens);
    if (update) {
        sdk_ret = rx_policer_tbl->update(pd_lif->hw_lif_id, &d, &d_mask);
    } else {
        sdk_ret = rx_policer_tbl->insert_withid(&d, pd_lif->hw_lif_id, &d_mask);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rx policer table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }

    return ret;
}
#undef RX_POLICER_ACTION

static hal_ret_t
lif_pd_rx_policer_action_tbl_reset_hw (pd_lif_t *pd_lif, bool insert)
{
    hal_ret_t                    ret = HAL_RET_OK;
    sdk_ret_t                    sdk_ret;
    lif_t                        *pi_lif = (lif_t *)pd_lif->pi_lif;
    directmap                    *rx_policer_action_tbl = NULL;
    rx_policer_action_actiondata_t d = {0};

    rx_policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER_ACTION);
    SDK_ASSERT_RETURN((rx_policer_action_tbl != NULL), HAL_RET_ERR);

    d.action_id = RX_POLICER_ACTION_RX_POLICER_ACTION_ID;

    if (insert) {
        sdk_ret = rx_policer_action_tbl->insert_withid(&d, pd_lif->hw_lif_id);
    } else {
        sdk_ret = rx_policer_action_tbl->remove(pd_lif->hw_lif_id);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("rx policer action table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }
    HAL_TRACE_DEBUG("lif {} hw_lif_id {} rx policer action programmed ",
                    lif_get_lif_id(pi_lif),
                    pd_lif->hw_lif_id);
    return ret;
}

hal_ret_t
lif_pd_rx_policer_program_hw (pd_lif_t *pd_lif, bool update)
{
    hal_ret_t ret;
    lif_t     *pi_lif = (lif_t *)pd_lif->pi_lif;

    ret = lif_pd_rx_policer_tbl_program_hw(pd_lif, update);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Rx policer table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }

    if (!update) {
        ret = lif_pd_rx_policer_action_tbl_reset_hw(pd_lif, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Rx policer action table write failure, lif {}, ret {}",
                          lif_get_lif_id(pi_lif), ret);
            return ret;
        }
    }

    return ret;
}

static hal_ret_t
lif_pd_rx_policer_tbl_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    directmap             *rx_policer_tbl = NULL;

    rx_policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER);
    SDK_ASSERT_RETURN((rx_policer_tbl != NULL), HAL_RET_ERR);

    sdk_ret = rx_policer_tbl->remove(pd_lif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("lif_id:{},unable to deprogram rx policer table",
                      lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("lif_id:{},deprogrammed rx policer table",
                      lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }

    return ret;
}

hal_ret_t
lif_pd_rx_policer_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t ret;
    lif_t     *pi_lif = (lif_t *)pd_lif->pi_lif;

    ret = lif_pd_rx_policer_tbl_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Rx policer table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }

    ret = lif_pd_rx_policer_action_tbl_reset_hw(pd_lif, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Rx policer action table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }

    return ret;
}

static hal_ret_t
lif_pd_stats_read (intf::LifRxStats *rx_stats,
                   intf::LifTxStats *tx_stats,
                   pd_lif_t *pd_lif)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    lif_t                           *pi_lif = (lif_t *)pd_lif->pi_lif;
    delphi::objects::lifmetrics_t   lif_metrics = {0};

    sdk::types::mem_addr_t stats_mem_addr =
        get_mem_addr(CAPRI_HBM_REG_LIF_STATS);

    stats_mem_addr += pd_lif->hw_lif_id << LIF_STATS_SIZE_SHIFT;

    HAL_TRACE_VERBOSE("lif:{}, stats_mem_addr: {:x}",
                    pd_lif->hw_lif_id, stats_mem_addr);

    ret = sdk::asic::asic_mem_read(stats_mem_addr, (uint8_t *)&lif_metrics,
                                   sizeof(delphi::objects::lifmetrics_t));
    if (ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Error reading stats for lif {} hw-id {}, ret {}",
                      lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, ret);
        return hal_sdk_ret_to_hal_ret(ret);
    }

    rx_stats->set_unicast_frames_ok(lif_metrics.rx_unicast_packets);
    rx_stats->set_multicast_frames_ok(lif_metrics.rx_multicast_packets);
    rx_stats->set_broadcast_frames_ok(lif_metrics.rx_broadcast_packets);
    rx_stats->set_unicast_frames_drop(lif_metrics.rx_drop_unicast_packets);
    rx_stats->set_multicast_frames_drop(lif_metrics.rx_drop_multicast_packets);
    rx_stats->set_broadcast_frames_drop(lif_metrics.rx_drop_broadcast_packets);
    rx_stats->set_unicast_bytes_ok(lif_metrics.rx_unicast_bytes);
    rx_stats->set_multicast_bytes_ok(lif_metrics.rx_multicast_bytes);
    rx_stats->set_broadcast_bytes_ok(lif_metrics.rx_broadcast_bytes);
    rx_stats->set_drops(lif_metrics.rx_queue_empty_drops +
                        lif_metrics.rx_queue_disabled_drops);
#if 0
    rx_stats->set_frames_ok(lif_metrics.);
    rx_stats->set_bytes_ok(lif_metrics.);
    rx_stats->set_no_bufs(lif_metrics.);
    rx_stats->set_errors(lif_metrics.);
    rx_stats->set_rss(lif_metrics.);
    rx_stats->set_crc_errors(lif_metrics.);
    rx_stats->set_frames_64(lif_metrics.);
    rx_stats->set_frames_127(lif_metrics.);
    rx_stats->set_frames_255(lif_metrics.);
    rx_stats->set_frames_511(lif_metrics.);
    rx_stats->set_frames_1024(lif_metrics.);
    rx_stats->set_frames_1518(lif_metrics.);
    rx_stats->set_frames_to_max(lif_metrics.);
#endif
    tx_stats->set_unicast_frames_ok(lif_metrics.tx_unicast_packets);
    tx_stats->set_multicast_frames_ok(lif_metrics.tx_multicast_packets);
    tx_stats->set_broadcast_frames_ok(lif_metrics.tx_broadcast_packets);
    tx_stats->set_unicast_frames_drop(lif_metrics.tx_drop_unicast_packets);
    tx_stats->set_multicast_frames_drop(lif_metrics.tx_drop_multicast_packets);
    tx_stats->set_broadcast_frames_drop(lif_metrics.tx_drop_broadcast_packets);
    tx_stats->set_unicast_bytes_ok(lif_metrics.tx_unicast_bytes);
    tx_stats->set_multicast_bytes_ok(lif_metrics.tx_multicast_bytes);
    tx_stats->set_broadcast_bytes_ok(lif_metrics.tx_broadcast_bytes);
#if 0
    tx_stats->set_drops(lif_metrics.tx_queue_empty_drops +
                        lif_metrics.tx_queue_disabled_drops);
#endif

    HAL_TRACE_VERBOSE("lif: hw_id: {}, Rx: Uc: {}, Mc: {}, Bc: {}, "
                    "Bytes: Uc: {}, Mc: {}, Bc: {}, Drops: {}",
                    pd_lif->hw_lif_id,
                    lif_metrics.rx_unicast_packets,
                    lif_metrics.rx_multicast_packets,
                    lif_metrics.rx_broadcast_packets,
                    lif_metrics.rx_unicast_bytes,
                    lif_metrics.rx_multicast_bytes,
                    lif_metrics.rx_broadcast_bytes,
                    lif_metrics.rx_queue_empty_drops +
                    lif_metrics.rx_queue_disabled_drops);
    HAL_TRACE_VERBOSE("lif: hw_id: {}, Tx: Uc: {}, Mc: {}, Bc: {}, "
                    "Bytes: Uc: {}, Mc: {}, Bc: {}, "
                    "Drops: UC: {}, MC: {}, BC: {}, "
                    "Bytes: UC: {}, MC: {}, BC: {}",
                    pd_lif->hw_lif_id,
                    lif_metrics.tx_unicast_packets,
                    lif_metrics.tx_multicast_packets,
                    lif_metrics.tx_broadcast_packets,
                    lif_metrics.tx_unicast_bytes,
                    lif_metrics.tx_multicast_bytes,
                    lif_metrics.tx_broadcast_bytes,
                    lif_metrics.tx_drop_unicast_packets,
                    lif_metrics.tx_drop_multicast_packets,
                    lif_metrics.tx_drop_broadcast_packets,
                    lif_metrics.tx_drop_unicast_bytes,
                    lif_metrics.tx_drop_multicast_bytes,
                    lif_metrics.tx_drop_broadcast_bytes);

    return HAL_RET_OK;
}

typedef struct lif_pd_rx_policer_stats_s {
    uint64_t permitted_bytes;
    uint64_t permitted_packets;
    uint64_t denied_bytes;
    uint64_t denied_packets;
} __PACK__ lif_pd_rx_policer_stats_t;

#define RX_POLICER_STATS(_d, _arg) _d.action_u.rx_policer_action_rx_policer_action._arg
static hal_ret_t
lif_pd_populate_rx_policer_stats (qos::PolicerStats *stats_rsp, pd_lif_t *pd_lif)
{
    hal_ret_t                    ret = HAL_RET_OK;
    lif_pd_rx_policer_stats_t    stats_0 = {0};
    lif_pd_rx_policer_stats_t    stats_1 = {0};
    mem_addr_t                   stats_addr = 0;
    sdk_ret_t                    sdk_ret;
    lif_t                        *pi_lif = (lif_t *)pd_lif->pi_lif;
    directmap                    *rx_policer_action_tbl= NULL;
    rx_policer_action_actiondata_t d;

    rx_policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER_ACTION);
    SDK_ASSERT_RETURN((rx_policer_action_tbl != NULL), HAL_RET_ERR);

    ret = hal_pd_stats_addr_get(P4TBL_ID_RX_POLICER_ACTION,
                                pd_lif->hw_lif_id, &stats_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting stats address for lif {} hw-id {}, ret {}",
                      lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, ret);
        return ret;
    }

    sdk_ret = sdk::asic::asic_mem_read(stats_addr, (uint8_t *)&stats_0,
                                       sizeof(stats_0));
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Error reading stats for lif {} hw-id {}, ret {}",
                      lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, ret);
        return hal_sdk_ret_to_hal_ret(sdk_ret);
    }

    // read the d-vector
    sdk_ret = rx_policer_action_tbl->retrieve(pd_lif->hw_lif_id, &d);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading lif action entry lif {} hw-id {}, ret {}",
                      lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, ret);
        return ret;
    }
    sdk_ret = sdk::asic::asic_mem_read(stats_addr, (uint8_t *)&stats_1,
                                       sizeof(stats_1));
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Error reading stats for lif {} hw-id {}, ret {}",
                      lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, ret);
        return hal_sdk_ret_to_hal_ret(sdk_ret);
    }

    HAL_TRACE_VERBOSE("Policer stat read lif {} hw_lif_id {} stats_addr {:#x} "
                    "stats_0.permitted_packets {} "
                    "stats_0.permitted_bytes {} "
                    "stats_0.denied_packets {} "
                    "stats_0.denied_bytes {} "
                    "stats_1.permitted_packets {} "
                    "stats_1.permitted_bytes {} "
                    "stats_1.denied_packets {} "
                    "stats_1.denied_bytes {} "
                    "d.permitted_packets {} "
                    "d.permitted_bytes {} "
                    "d.denied_packets {} "
                    "d.denied_bytes {} ",
                    lif_get_lif_id(pi_lif), pd_lif->hw_lif_id, stats_addr,
                    stats_0.permitted_packets ,
                    stats_0.permitted_bytes ,
                    stats_0.denied_packets ,
                    stats_0.denied_bytes ,
                    stats_1.permitted_packets ,
                    stats_1.permitted_bytes ,
                    stats_1.denied_packets ,
                    stats_1.denied_bytes ,
                    RX_POLICER_STATS(d,permitted_packets),
                    RX_POLICER_STATS(d,permitted_bytes),
                    RX_POLICER_STATS(d,denied_packets),
                    RX_POLICER_STATS(d,denied_bytes));

    if (stats_1.permitted_packets == stats_0.permitted_packets) {
        stats_1.permitted_packets += RX_POLICER_STATS(d, permitted_packets);
        stats_1.permitted_bytes += RX_POLICER_STATS(d, permitted_bytes);
    }
    if (stats_1.denied_packets == stats_0.denied_packets) {
        stats_1.denied_packets += RX_POLICER_STATS(d, denied_packets);
        stats_1.denied_bytes += RX_POLICER_STATS(d, denied_bytes);
    }

    stats_rsp->set_permitted_packets(stats_1.permitted_packets);
    stats_rsp->set_permitted_bytes(stats_1.permitted_bytes);
    stats_rsp->set_dropped_packets(stats_1.denied_packets);
    stats_rsp->set_dropped_bytes(stats_1.denied_bytes);
    return HAL_RET_OK;
}
#undef RX_POLICER_STATS

//-----------------------------------------------------------------------------
// Program Output Mapping Table
//-----------------------------------------------------------------------------
#define om_tmoport data.action_u.output_mapping_set_tm_oport
#define om_cpu data.action_u.output_mapping_redirect_to_cpu
hal_ret_t
lif_pd_pgm_output_mapping_tbl(pd_lif_t *pd_lif, pd_lif_update_args_t *args,
                              table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     p4plus_app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    output_mapping_actiondata_t   data;
    directmap                   *dm_omap = NULL;

    memset(&data, 0, sizeof(data));

    /*
     * For Service lifs,
     *      hw_lif_id:      passed to PI with hw_lif_if filled.
     *      lif_lport_id:   internal to P4 and allocated in PD
     * For Regular lifs,
     *      hw_lif_id:      allocated in PI
     *      lif_lport_id:   internal to P4 and allocated in PD
     */
    if (((lif_t *)pd_lif->pi_lif)->lif_id == SERVICE_LIF_APP_REDIR) {
        p4plus_app_id = P4PLUS_APPTYPE_RAW_REDIR;
    } else if (((lif_t *)pd_lif->pi_lif)->lif_id == SERVICE_LIF_TCP_PROXY) {
        p4plus_app_id = P4PLUS_APPTYPE_TCPTLS;
    } else if (((lif_t *)pd_lif->pi_lif)->lif_id == SERVICE_LIF_IPSEC_ESP) {
        p4plus_app_id = P4PLUS_APPTYPE_IPSEC;
    }

    if (((lif_t *)pd_lif->pi_lif)->lif_id == SERVICE_LIF_P4PT) {
      /*
       * P4PT goes to a separate monitoring service lif
       */
       p4plus_app_id = P4PLUS_APPTYPE_P4PT;
       pd_lif->lif_lport_id = SERVICE_LIF_P4PT;
       pd_lif->hw_lif_id = SERVICE_LIF_P4PT;
       HAL_TRACE_ERR("setting p4pt tm_port {}", SERVICE_LIF_P4PT);
    }


    data.action_id = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_mirror_en = 1;
    om_tmoport.egress_port1 = TM_PORT_DMA;
    om_tmoport.p4plus_app_id = p4plus_app_id;
    om_tmoport.rdma_enabled = lif_get_enable_rdma((lif_t *)pd_lif->pi_lif);
    om_tmoport.dst_lif = pd_lif->hw_lif_id;
    om_tmoport.vlan_strip = pd_lif_get_vlan_strip_en((lif_t *)pd_lif->pi_lif, args);


    // Program OutputMapping table
    //  - Get tmoport from PI
    //  - Get vlan_tagid_in_skb from the fwding mode:
    //      - Classic: TRUE
    //      - Switch : FALSE

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = dm_omap->insert_withid(&data, pd_lif->lif_lport_id);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{} {} unable to program",
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                          oper);
        } else {
            HAL_TRACE_DEBUG("lif_id:{}, {} programmed output "
                            "mapping at:{}",
                            lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                            oper, pd_lif->lif_lport_id);
        }
    } else {
        sdk_ret = dm_omap->update(pd_lif->lif_lport_id, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{},{} unable to program",
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif), oper);
        } else {
            HAL_TRACE_DEBUG("lif_id:{},{} programmed output "
                            "mapping at:{}",
                            lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                            oper, pd_lif->lif_lport_id);
        }
    }
    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram output mapping table
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_depgm_output_mapping_tbl (pd_lif_t *pd_lif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->remove(pd_lif->lif_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("lif_id:{},unable to deprogram output "
                      "mapping table", lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("lif_id:{},deprogrammed output "
                      "mapping table", lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Freeing LIF PD. Frees PD memory and all other memories allocated for PD.
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_free (pd_lif_t *lif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_LIF_PD, lif);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Freeing LIF PD memory. Just frees the memory of PD structure.
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_mem_free (pd_lif_t *lif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_LIF_PD, lif);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Linking PI <-> PD
//-----------------------------------------------------------------------------
void
link_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = pi_lif;
    lif_set_pd_lif(pi_lif, pd_lif);
}

//-----------------------------------------------------------------------------
// Un-Linking PI <-> PD
//-----------------------------------------------------------------------------
void
delink_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = NULL;
    lif_set_pd_lif(pi_lif, NULL);
}

//-----------------------------------------------------------------------------
// Makes a clone
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_lif_make_clone_args_t *args = pd_func_args->pd_lif_make_clone;
    pd_lif_t            *pd_lif_clone = NULL;

    lif_t *lif = args->lif;
    lif_t *clone = args->clone;

    pd_lif_clone = lif_pd_alloc_init();
    if (pd_lif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_lif_clone, lif->pd_lif, sizeof(pd_lif_t));

    link_pi_pd(pd_lif_clone, clone);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Frees PD memory without indexer free.
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_mem_free (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_mem_free_args_t *args = pd_func_args->pd_lif_mem_free;
    pd_lif_t    *lif_pd;

    lif_pd = (pd_lif_t *)args->lif->pd_lif;
    lif_pd_mem_free(lif_pd);

    return ret;
}

//-----------------------------------------------------------------------------
// Get PD hw_lif_id from lif.
//-------------------------------------------------------------------------

// uint32_t pd_lif_get(lif_t *lif)
hal_ret_t
pd_lif_get (pd_func_args_t *pd_func_args)
{
    pd_lif_get_args_t *args = pd_func_args->pd_lif_get;
    lif_t *lif = args->lif;

    pd_lif_t  *lif_pd = (pd_lif_t *)lif->pd_lif;

    args->hw_lif_id = lif_pd->hw_lif_id;

    return HAL_RET_OK;
}

hal_ret_t
pd_lif_stats_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_lif_stats_get_args_t *args = pd_func_args->pd_lif_stats_get;
    lif_t                   *lif = args->lif;
    pd_lif_t                *pd_lif = (pd_lif_t *)lif->pd_lif;
    LifGetResponse          *rsp = args->rsp;

    ret = lif_pd_populate_rx_policer_stats(
                        rsp->mutable_stats()->mutable_data_lif_stats()->mutable_rx_stats()->mutable_policer_stats(),
                        pd_lif);

    ret = lif_pd_stats_read(
                        rsp->mutable_stats()->mutable_data_lif_stats()->mutable_rx_stats(),
                        rsp->mutable_stats()->mutable_data_lif_stats()->mutable_tx_stats(),
                        pd_lif);


    return ret;
}

//-----------------------------------------------------------------------------
// Control TX Scheduler for Lif
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_sched_control (pd_func_args_t *pd_func_args)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    pd_lif_sched_control_args_t     *args = pd_func_args->pd_lif_sched_control;
    pd_lif_t                        *lif_pd = (pd_lif_t *)args->lif->pd_lif;
    asicpd_scheduler_lif_params_t   apd_lif;

    pd_lif_copy_asicpd_params(&apd_lif, lif_pd);
    if (!args->en) {
        if (lif_pd->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
            sdk_ret = asicpd_tx_scheduler_map_free(&apd_lif);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                ret = HAL_RET_INVALID_OP;
                goto end;
            }
        }
        HAL_TRACE_DEBUG("Successfully disabled TX scheduler for lif: {}",
                        args->lif->lif_id);
    }

end:
    return ret;
}

#if 0
hal_ret_t
pd_lif_pgm_host_mgmt (pd_lif_t *pd_lif)
{
    hal_ret_t           ret = HAL_RET_OK;
    nacl_swkey_t        key;
    nacl_swkey_mask_t   mask;
    nacl_actiondata_t   data;
    acl_tcam            *acl_tbl = NULL;

    acl_tbl = g_hal_state_pd->acl_table();
    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    key.entry_inactive_nacl = 0;
    mask.entry_inactive_nacl_mask = 0x1;

    key.capri_intrinsic_lif = pd_lif->hw_lif_id;
    mask.capri_intrinsic_lif_mask =
        ~(mask.capri_intrinsic_lif_mask & 0);
    key.control_metadata_nic_mode = NIC_MODE_SMART;
    mask.control_metadata_nic_mode_mask =
        ~(mask.control_metadata_nic_mode_mask & 0);
    data.action_id = NACL_NACL_DENY_ID;

    ret = acl_tbl->insert(&key, &mask, &data,
                          ACL_HOSTPIN_HOST_MGMT_DROP, &pd_lif->host_mgmt_acl_handle);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Programmed nacl to prevent non-classic traffic "
                        "on host-mgmt.");
    } else {
        HAL_TRACE_ERR("Unable to program nacl to prevent non-classic traffic "
                      "on host-mgmt. ret: {}", ret);
    }

    return ret;
}

hal_ret_t
pd_lif_depgm_host_mgmt (pd_lif_t *pd_lif)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_tcam  *acl_tbl = NULL;

    acl_tbl = g_hal_state_pd->acl_table();
    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    if (pd_lif->host_mgmt_acl_handle != INVALID_INDEXER_INDEX) {
        ret = acl_tbl->remove(pd_lif->host_mgmt_acl_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to cleanup for acl: {}",
                          pd_lif->host_mgmt_acl_handle);
        } else {
            HAL_TRACE_DEBUG("Programmed cleanup acl: {}",
                            pd_lif->host_mgmt_acl_handle);
        }
    }
    return ret;
}
#endif

//-----------------------------------------------------------------------------
// Program mirror session
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_pgm_mirror_session (pd_lif_t *pd_lif, if_t *hal_if,
                           table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret = SDK_RET_OK;
    mirror_actiondata_t         mirr_data = { 0 };
    directmap                   *mirr_dm;
    lif_t                       *lif = (lif_t *)pd_lif->pi_lif;
    uint32_t                    lport = 0;

    mirr_dm = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);

    lport = if_get_lport_id(hal_if);
    mirr_data.action_id = MIRROR_LOCAL_SPAN_ID;
    mirr_data.action_u.mirror_local_span.dst_lport = lport;
    mirr_data.action_u.mirror_local_span.qid_en = 0;
    mirr_data.action_u.mirror_local_span.qid = 0; 

    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = mirr_dm->insert(&mirr_data, &pd_lif->rdma_sniff_mirr_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{},{} unable to program mirror table",
                          lif_get_lif_id(lif), ret);
        } else {
            HAL_TRACE_DEBUG("lif_id:{}, programmed mirror table at: {}",
                            lif_get_lif_id(lif), pd_lif->rdma_sniff_mirr_idx);
        }
    } else {
        if (pd_lif->rdma_sniff_mirr_idx != INVALID_INDEXER_INDEX) {
            sdk_ret = mirr_dm->update(pd_lif->rdma_sniff_mirr_idx, &mirr_data);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("lif_id:{},{} unable to program mirror table",
                              lif_get_lif_id(lif), ret);
            } else {
                HAL_TRACE_DEBUG("lif_id:{}, programmed mirror table at: {}",
                                lif_get_lif_id(lif), 
                                pd_lif->rdma_sniff_mirr_idx);
            }
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram mirror session
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_depgm_mirror_session (pd_lif_t *pd_lif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *mirr_dm = NULL;

    mirr_dm = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);

    if (pd_lif->rdma_sniff_mirr_idx != INVALID_INDEXER_INDEX) {
        sdk_ret = mirr_dm->remove(pd_lif->rdma_sniff_mirr_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{},unable to deprogram mirror table at: {}",
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif), 
                          pd_lif->rdma_sniff_mirr_idx);
        } else {
            HAL_TRACE_ERR("lif_id:{}, deprogrammed mirror table at: {} ",
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                          pd_lif->rdma_sniff_mirr_idx);
        }
        pd_lif->rdma_sniff_mirr_idx = INVALID_INDEXER_INDEX;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Program TX NACL to drive egress mirror
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_pgm_tx_nacl (pd_lif_t *pd_lif, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    nacl_swkey_t        key;
    nacl_swkey_mask_t   mask;
    nacl_actiondata_t   data;
    uint32_t            lport = 0;
    acl_tcam            *acl_tbl = NULL;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    acl_tbl = g_hal_state_pd->acl_table();

    lport = if_get_lport_id(hal_if);

    // (src_lport, RDMA's dport) => mirr_idx
    key.entry_inactive_nacl = 0;
    mask.entry_inactive_nacl_mask = 0x1;
    key.control_metadata_src_lport = lport;
    mask.control_metadata_src_lport_mask =
        ~(mask.control_metadata_dst_lport_mask & 0);
    key.flow_lkp_metadata_lkp_dport = UDP_PORT_ROCE_V2;
    mask.flow_lkp_metadata_lkp_dport_mask =
        ~(mask.flow_lkp_metadata_lkp_dport_mask & 0);

    data.action_id = NACL_NACL_PERMIT_ID;
    data.action_u.nacl_nacl_permit.egress_mirror_en = 1;
    data.action_u.nacl_nacl_permit.egress_mirror_session_id = 0x1 << pd_lif->rdma_sniff_mirr_idx;

    ret = acl_tbl->insert(&key, &mask, &data, ACL_RDMA_SNIFFER_PRIORITY, 
                          &pd_lif->tx_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program TX NACL. ret: {}", ret);
    } else {
        HAL_TRACE_DEBUG("Programmed TX NACL at: {}", pd_lif->tx_handle);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram TX NACL to drive egress mirror
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_depgm_tx_nacl (pd_lif_t *pd_lif)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_tcam  *acl_tbl = NULL;

    acl_tbl = g_hal_state_pd->acl_table();

    if (pd_lif->tx_handle != INVALID_INDEXER_INDEX) {
        ret = acl_tbl->remove(pd_lif->tx_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram NACL. {} ret: {}", 
                          pd_lif->tx_handle, ret);
        } else {
            HAL_TRACE_DEBUG("Deprogrammed NACL at: {}", pd_lif->tx_handle);
        }
        pd_lif->tx_handle = INVALID_INDEXER_INDEX;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Program RX NACL to drive egress mirror
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_pgm_rx_nacl (pd_lif_t *pd_lif, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    nacl_swkey_t        key;
    nacl_swkey_mask_t   mask;
    nacl_actiondata_t   data;
    uint32_t            lport = 0;
    acl_tcam            *acl_tbl = NULL;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    acl_tbl = g_hal_state_pd->acl_table();

    lport = if_get_lport_id(hal_if);

    // (dst_lport, RDMA's dport) => mirr_idx
    key.entry_inactive_nacl = 0;
    mask.entry_inactive_nacl_mask = 0x1;
    key.control_metadata_dst_lport = lport;
    mask.control_metadata_dst_lport_mask =
        ~(mask.control_metadata_dst_lport_mask & 0);
    key.flow_lkp_metadata_lkp_dport = UDP_PORT_ROCE_V2;
    mask.flow_lkp_metadata_lkp_dport_mask =
        ~(mask.flow_lkp_metadata_lkp_dport_mask & 0);

    data.action_id = NACL_NACL_PERMIT_ID;
    data.action_u.nacl_nacl_permit.ingress_mirror_en = 1;
    data.action_u.nacl_nacl_permit.ingress_mirror_session_id = 0x1 << pd_lif->rdma_sniff_mirr_idx;

    ret = acl_tbl->insert(&key, &mask, &data, ACL_RDMA_SNIFFER_PRIORITY, 
                          &pd_lif->rx_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program RX NACL. ret: {}", ret);
    } else {
        HAL_TRACE_DEBUG("Programmed RX NACL at: {}", pd_lif->rx_handle);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram RX NACL to drive ingress mirror
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_depgm_rx_nacl (pd_lif_t *pd_lif)
{
    hal_ret_t ret = HAL_RET_OK;
    acl_tcam  *acl_tbl = NULL;

    acl_tbl = g_hal_state_pd->acl_table();

    if (pd_lif->rx_handle != INVALID_INDEXER_INDEX) {
        ret = acl_tbl->remove(pd_lif->rx_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram NACL. {} ret: {}", 
                          pd_lif->rx_handle, ret);
        } else {
            HAL_TRACE_DEBUG("Deprogrammed NACL at: {}", pd_lif->rx_handle);
        }
        pd_lif->rx_handle = INVALID_INDEXER_INDEX;
    }

    return ret;
}

hal_ret_t
pd_lif_install_rdma_sniffer (pd_lif_t *pd_lif, if_t *hal_if)
{
    hal_ret_t ret = HAL_RET_OK;
    lif_t *lif = (lif_t *)pd_lif->pi_lif;

    if ((lif && lif->rdma_sniff_en) &&
        (hal_if && hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC)) {

        HAL_TRACE_DEBUG("Installing RDMA Sniffer");

        // Install Mirror session
        ret = pd_lif_pgm_mirror_session(pd_lif, hal_if, TABLE_OPER_INSERT);

        // Install TX and RX NACLs
        ret = pd_lif_pgm_tx_nacl(pd_lif, hal_if);
        ret = pd_lif_pgm_rx_nacl(pd_lif, hal_if);
    }

    return ret;
}

hal_ret_t
pd_lif_uninstall_rdma_sniffer (pd_lif_t *pd_lif)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Un-Installing RDMA Sniffer");

    // Un-Install TX and RX NACLs
    ret = pd_lif_depgm_tx_nacl(pd_lif);
    ret = pd_lif_depgm_rx_nacl(pd_lif);

    // Un-Install Mirror session
    ret = pd_lif_depgm_mirror_session(pd_lif);

    return ret;
}

}    // namespace pd
}    // namespace hal
