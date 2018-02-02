#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/lif_pd.hpp"
#include "nic/hal/pd/iris/scheduler_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/eth.hpp"


namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Lif Create in PD
// ----------------------------------------------------------------------------
EXTC hal_ret_t
pd_lif_create (pd_lif_create_args_t *args)
{
    hal_ret_t            ret;
    pd_lif_t             *pd_lif;

    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{} Creating pd state for Lif", 
            __FUNCTION__, lif_get_lif_id(args->lif));

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
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{} Unable to alloc. resources",
                __FUNCTION__, lif_get_lif_id(args->lif));
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
EXTC hal_ret_t
pd_lif_update (pd_lif_update_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif = args->lif;
    pd_lif_t            *pd_lif = (pd_lif_t *)args->lif->pd_lif;

    if (args->rx_policer_changed) {
        HAL_TRACE_DEBUG("pd-lif:{}: rx policer changed ", __FUNCTION__);

        ret = lif_pd_rx_policer_program_hw(pd_lif, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for rx policer", __FUNCTION__);
            goto end;
        }
    }

    if (args->tx_policer_changed) {
        HAL_TRACE_DEBUG("pd-lif:{}: tx policer changed ", __FUNCTION__);

        ret = lif_pd_tx_policer_program_hw(pd_lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx policer", __FUNCTION__);
            goto end;
        }
    }

    // Process VLAN offload config changes
    if (args->vlan_strip_en_changed) {
        HAL_TRACE_DEBUG("pd-lif:{}: vlan_strip_en changed. ", __FUNCTION__);

        // Program output mapping table
        ret = lif_pd_pgm_output_mapping_tbl(pd_lif,
                                            args, TABLE_OPER_UPDATE);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw", __FUNCTION__);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    // Process ETH RSS configuration changes
    if (args->rss_config_changed) {
        HAL_TRACE_DEBUG("pd-lif:{}: rss config changed. ", __FUNCTION__);

        ret = eth_rss_init(pd_lif->hw_lif_id, &lif->rss,
            (lif_queue_info_t *)&lif->qinfo);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for RSS", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    if (args->qstate_map_init_set) {
        HAL_TRACE_DEBUG("pd-lif:{}: qstate map init . ", __FUNCTION__);
        ret = scheduler_tx_pd_alloc((pd_lif_t *)args->lif->pd_lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to scheduler resource",
                          __FUNCTION__, args->lif->lif_id);
            goto end;
        }
        ret = scheduler_tx_pd_program_hw((pd_lif_t *)args->lif->pd_lif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx scheduler", __FUNCTION__);
            goto end;
        }
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Lif Delete
//-----------------------------------------------------------------------------
EXTC hal_ret_t
pd_lif_delete (pd_lif_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_t       *lif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->lif != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->lif->pd_lif != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{},deleting pd state",
                    __FUNCTION__, args->lif->lif_id);
    lif_pd = (pd_lif_t *)args->lif->pd_lif;

    // Deprogram
    ret = lif_pd_deprogram_hw(lif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to deprogram hw tables. ret:{}",
                      __FUNCTION__, args->lif->lif_id, ret);
        goto end;
    }

    // Cleanup
    ret = lif_pd_cleanup(lif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed pd lif delete",
                      __FUNCTION__, args->lif->lif_id);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Lif
// ----------------------------------------------------------------------------
hal_ret_t 
lif_pd_alloc_res(pd_lif_t *pd_lif, pd_lif_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;
    
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
        HAL_ASSERT(0);
    }

    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{} allocated hw_lif_id:{}", 
                    __FUNCTION__, 
                    lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                    pd_lif->hw_lif_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_lif->
            lif_lport_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to alloc lport. err:{}",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                      rs);
        pd_lif->lif_lport_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    //Allocate tx scheduler resource for this lif if qstate-map init is done.
    if (args->lif->qstate_init_done) {
       ret = scheduler_tx_pd_alloc(pd_lif);
       if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to scheduler resource",
                          __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                          rs);
            goto end;
       }
    }

    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{},allocated lport_id:{}", 
                    __FUNCTION__, 
                    lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                    pd_lif->lif_lport_id);

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

    if (lif_pd->lif_lport_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->lport_idxr()->free(lif_pd->lif_lport_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-lif:{}:failed to free lport err: {}", 
                          __FUNCTION__, lif_pd->lif_lport_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-lif:{}:freed lport: {}", 
                        __FUNCTION__, lif_pd->lif_lport_id);
    }
   
    if (lif_pd->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
        ret = scheduler_tx_pd_dealloc(lif_pd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:failed to free sched table res at offset: {}",
                          __FUNCTION__, lif_pd->tx_sched_table_offset);
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
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{}, unable to dealloc res", 
                      __FUNCTION__, 
                      lif_get_lif_id((lif_t *)lif_pd->pi_lif));
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(lif_pd, (lif_t *)lif_pd->pi_lif);

    // Freeing PD
    lif_pd_free(lif_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Lif PD Instance
// ----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_alloc_init (void)
{
    return lif_pd_init(lif_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate LIF Instance
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// Initialize LIF PD instance
// ----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_init (pd_lif_t *lif)
{
    // Nothing to do currently
    if (!lif) {
        return NULL;
    }
    // Set here if you want to initialize any fields
    lif->tx_sched_table_offset = INVALID_INDEXER_INDEX;

    return lif;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_program_hw (pd_lif_t *pd_lif)
{
    hal_ret_t            ret;
    lif_t                *lif = (lif_t *)pd_lif->pi_lif;

    // Program the policers
    ret = lif_pd_rx_policer_program_hw(pd_lif, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw for rx policer", __FUNCTION__);
        goto end;
    }
    
    ret = lif_pd_tx_policer_program_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx policer", __FUNCTION__);
        goto end;
    }

    // Program output mapping table
    ret = lif_pd_pgm_output_mapping_tbl(pd_lif, NULL, TABLE_OPER_INSERT);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw", __FUNCTION__);
        goto end;
    }

    ret = scheduler_tx_pd_program_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw for tx scheduler", __FUNCTION__);
        goto end;
    }

    // ETH RSS configuration
    ret = eth_rss_init(pd_lif->hw_lif_id, &lif->rss,
        (lif_queue_info_t *)&lif->qinfo);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to program hw for RSS", ret);
        ret = HAL_RET_ERR;
        goto end;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Deprogram output mapping table
    ret = lif_pd_depgm_output_mapping_tbl(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to deprogram hw", __FUNCTION__);
        goto end;
    }

    ret = scheduler_tx_pd_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to deprogram hw for tx scheduler", __FUNCTION__);
        goto end;
    }

    ret = lif_pd_rx_policer_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to deprogram hw for rx policer", __FUNCTION__);
        goto end;
    }

    ret = lif_pd_tx_policer_deprogram_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:unable to deprogram hw for tx policer", __FUNCTION__);
        goto end;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Get vlan strip enable
// ----------------------------------------------------------------------------
bool
pd_lif_get_vlan_strip_en (lif_t *lif, pd_lif_update_args_t *args)
{
    if (args && args->vlan_strip_en_changed) {
        return args->vlan_strip_en;
    }
    return lif->vlan_strip_en;
}

hal_ret_t
lif_pd_tx_policer_program_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
lif_pd_tx_policer_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;

    return ret;
}

#define RX_POLICER_ACTION(_arg) d.rx_policer_action_u.rx_policer_execute_rx_policer._arg
hal_ret_t
lif_pd_rx_policer_program_hw (pd_lif_t *pd_lif, bool update)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    lif_t                 *pi_lif = (lif_t *)pd_lif->pi_lif;
    directmap             *rx_policer_tbl = NULL;
    rx_policer_actiondata d = {0};

    rx_policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER);
    HAL_ASSERT_RETURN((rx_policer_tbl != NULL), HAL_RET_ERR);

    d.actionid = RX_POLICER_EXECUTE_RX_POLICER_ID;
    if (pi_lif->qos_info.rx_policer.bps_rate == 0) {
        RX_POLICER_ACTION(entry_valid) = 0;
    } else {
        RX_POLICER_ACTION(entry_valid) = 1;
        RX_POLICER_ACTION(pkt_rate) = 0;
        //TODO does this need memrev ?
        memcpy(RX_POLICER_ACTION(burst), &pi_lif->qos_info.rx_policer.burst_size, sizeof(uint32_t));
        // TODO convert the rate into token-rate 
        memcpy(RX_POLICER_ACTION(rate), &pi_lif->qos_info.rx_policer.bps_rate, sizeof(uint32_t));
    }

    if (update) {
        sdk_ret = rx_policer_tbl->update(pd_lif->hw_lif_id, &d);
    } else {
        sdk_ret = rx_policer_tbl->insert_withid(&d, pd_lif->hw_lif_id);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}: rx policer table write failure, lif {}, ret {}",
                      __FUNCTION__, lif_get_lif_id(pi_lif), ret);
        return ret;
    }
    HAL_TRACE_DEBUG("pd-lif:{}: lif {} hw_lif_id {} rate {} burst {} programmed",
                    __FUNCTION__, lif_get_lif_id(pi_lif), 
                    pd_lif->hw_lif_id, pi_lif->qos_info.rx_policer.bps_rate,
                    pi_lif->qos_info.rx_policer.burst_size);
    return ret;
}
#undef RX_POLICER_ACTION

hal_ret_t
lif_pd_rx_policer_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    directmap             *rx_policer_tbl = NULL;

    rx_policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER);
    HAL_ASSERT_RETURN((rx_policer_tbl != NULL), HAL_RET_ERR);

    sdk_ret = rx_policer_tbl->remove(pd_lif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},unable to deprogram rx policer table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},deprogrammed rx policer table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.output_mapping_action_u.output_mapping_set_tm_oport
#define om_cpu data.output_mapping_action_u.output_mapping_redirect_to_cpu
hal_ret_t
lif_pd_pgm_output_mapping_tbl(pd_lif_t *pd_lif, pd_lif_update_args_t *args, 
                              table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     p4plus_app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
    output_mapping_actiondata   data;
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
       HAL_TRACE_ERR("pd-lif:{}:setting p4pt tm_port = %d", __FUNCTION__, SERVICE_LIF_P4PT);
    }


    data.actionid = OUTPUT_MAPPING_SET_TM_OPORT_ID;
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
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = dm_omap->insert_withid(&data, pd_lif->lif_lport_id);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif::{}:lif_id:{} {} unable to program",
                          __FUNCTION__, 
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                          oper);
        } else {
            HAL_TRACE_DEBUG("pd-lif::{}:lif_id:{},{} programmed output "
                            "mapping at:{}",
                            __FUNCTION__, 
                            lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                            oper, pd_lif->lif_lport_id);
        }
    } else {
        sdk_ret = dm_omap->update(pd_lif->lif_lport_id, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif::{}:lif_id:{},{} unable to program",
                          __FUNCTION__, 
                          lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                          oper);
        } else {
            HAL_TRACE_DEBUG("pd-lif::{}:lif_id:{},{} programmed output "
                            "mapping at:{}",
                            __FUNCTION__, 
                            lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                            oper, pd_lif->lif_lport_id);
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram output mapping table
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_depgm_output_mapping_tbl (pd_lif_t *pd_lif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);
    
    sdk_ret = dm_omap->remove(pd_lif->lif_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},unable to deprogram output "
                      "mapping table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},deprogrammed output "
                      "mapping table",
                      __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing LIF PD. Frees PD memory and all other memories allocated for PD.
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_free (pd_lif_t *lif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_LIF_PD, lif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Freeing LIF PD memory. Just frees the memory of PD structure.
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_mem_free (pd_lif_t *lif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_LIF_PD, lif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = pi_lif;
    lif_set_pd_lif(pi_lif, pd_lif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
delink_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = NULL;
    lif_set_pd_lif(pi_lif, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
EXTC hal_ret_t
// pd_lif_make_clone(lif_t *ten, lif_t *clone)
pd_lif_make_clone(pd_lif_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
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

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
EXTC hal_ret_t
pd_lif_mem_free(pd_lif_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_t    *lif_pd;

    lif_pd = (pd_lif_t *)args->lif->pd_lif;
    lif_pd_mem_free(lif_pd);

    return ret;
}


// ----------------------------------------------------------------------------
// Get PD hw_lif_id from lif.
// ------------------------------------------------------------------------

// uint32_t pd_get_hw_lif_id(lif_t *lif)
EXTC hal_ret_t
pd_get_hw_lif_id (pd_get_hw_lif_id_args_t *args)
{
    lif_t *lif = args->lif;

    pd_lif_t  *lif_pd = (pd_lif_t *)lif->pd_lif;

    args->hw_lifid = lif_pd->hw_lif_id;

    return HAL_RET_OK;
}
}    // namespace pd
}    // namespace hal
