// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/gft/lif_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/p4/gft/include/defines.h"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/eth.hpp"
#include "nic/hal/pd/asicpd/asic_pd_scheduler.hpp"

namespace hal {
namespace pd {

static void
pd_lif_copy_asicpd_params (asicpd_scheduler_lif_params_t *out, pd_lif_t *lif_pd)
{
    out->lif_id = lif_get_lif_id((lif_t *)lif_pd->pi_lif),
    out->tx_sched_table_offset = lif_pd->tx_sched_table_offset;
    out->tx_sched_num_table_entries = lif_pd->tx_sched_num_table_entries;
    out->total_qcount = lif_get_total_qcount(lif_pd->hw_lif_id);
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
pd_lif_create (pd_lif_create_args_t *args)
{
    hal_ret_t            ret;
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
pd_lif_update (pd_lif_update_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif = args->lif;
    pd_lif_t            *pd_lif = (pd_lif_t *)args->lif->pd_lif;

    if (args->rx_policer_changed) {
        ret = lif_pd_rx_policer_program_hw(pd_lif, true);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }

    if (args->tx_policer_changed) {
        ret = lif_pd_tx_policer_program_hw(pd_lif);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }

    // Process ETH RSS configuration changes
    if (args->rss_config_changed) {
        ret = eth_rss_init(pd_lif->hw_lif_id, &lif->rss,
            (lif_queue_info_t *)&lif->qinfo);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:unable to program hw for RSS", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    asicpd_scheduler_lif_params_t apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, (pd_lif_t *)args->lif->pd_lif);
    if (args->qstate_map_init_set) {
        ret = asicpd_scheduler_tx_pd_alloc(&apd_lif);
        if (ret != HAL_RET_OK) {
            goto end;
        }
       // Copy scheduler info back to pd_lif.
	pd_lif_copy_asicpd_sched_params(pd_lif, &apd_lif);

       ret = asicpd_scheduler_tx_pd_program_hw(&apd_lif);
       if (ret != HAL_RET_OK) {
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
pd_lif_delete (pd_lif_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_t       *lif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->lif != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->lif->pd_lif != NULL), HAL_RET_INVALID_ARG);
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
    hal_ret_t            ret = HAL_RET_OK;
    
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

    asicpd_scheduler_lif_params_t apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    //Allocate tx scheduler resource for this lif if qstate-map init is done.
    if (args->lif->qstate_init_done) {
       ret = asicpd_scheduler_tx_pd_alloc(&apd_lif);
       if (ret != HAL_RET_OK) {
            goto end;
       }
       //Copy scheduler info back to pd_lif.
       pd_lif_copy_asicpd_sched_params(pd_lif, &apd_lif);
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
    hal_ret_t                       ret = HAL_RET_OK;
    asicpd_scheduler_lif_params_t   apd_lif;
    
    pd_lif_copy_asicpd_params(&apd_lif, lif_pd);
    if (lif_pd->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
        ret = asicpd_scheduler_tx_pd_dealloc(&apd_lif);
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

    return lif;
}

//-----------------------------------------------------------------------------
// Program HW
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_program_hw (pd_lif_t *pd_lif)
{
    hal_ret_t            ret;
    lif_t                *lif = (lif_t *)pd_lif->pi_lif;

    // Program the policers
    ret = lif_pd_rx_policer_program_hw(pd_lif, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for rx policer");
        goto end;
    }
    
    ret = lif_pd_tx_policer_program_hw(pd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for tx policer");
        goto end;
    }

    asicpd_scheduler_lif_params_t   apd_lif;
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    ret = asicpd_scheduler_tx_pd_program_hw(&apd_lif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for tx scheduler");
        goto end;
    }

    // ETH RSS configuration
    ret = eth_rss_init(pd_lif->hw_lif_id, &lif->rss,
                       (lif_queue_info_t *)&lif->qinfo);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program hw for RSS", ret);
        ret = HAL_RET_ERR;
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram HW
//-----------------------------------------------------------------------------
hal_ret_t
lif_pd_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t            ret = HAL_RET_OK;
    asicpd_scheduler_lif_params_t   apd_lif;
    
    pd_lif_copy_asicpd_params(&apd_lif, pd_lif);
    ret = asicpd_scheduler_tx_pd_deprogram_hw(&apd_lif);
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
#if 0
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
        HAL_TRACE_ERR("rx policer table write failure, lif {}, ret {}",
                      lif_get_lif_id(pi_lif), ret);
        return ret;
    }
    HAL_TRACE_DEBUG("lif {} hw_lif_id {} rate {} burst {} programmed",
                    lif_get_lif_id(pi_lif), 
                    pd_lif->hw_lif_id, pi_lif->qos_info.rx_policer.bps_rate,
                    pi_lif->qos_info.rx_policer.burst_size);
#endif
    return ret;
}
#undef RX_POLICER_ACTION

hal_ret_t
lif_pd_rx_policer_deprogram_hw (pd_lif_t *pd_lif)
{
    hal_ret_t             ret = HAL_RET_OK;
#if 0
    sdk_ret_t             sdk_ret;
    directmap             *rx_policer_tbl = NULL;

    rx_policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_RX_POLICER);
    HAL_ASSERT_RETURN((rx_policer_tbl != NULL), HAL_RET_ERR);

    sdk_ret = rx_policer_tbl->remove(pd_lif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("lif_id:{},unable to deprogram rx policer table",
                      lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_ERR("lif_id:{},deprogrammed rx policer table",
                      lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }
#endif
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

//-----------------------------------------------------------------------------
// Frees PD memory without indexer free.
//-----------------------------------------------------------------------------
hal_ret_t
pd_lif_mem_free(pd_lif_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_lif_t    *lif_pd;

    lif_pd = (pd_lif_t *)args->lif->pd_lif;
    lif_pd_mem_free(lif_pd);

    return ret;
}

//-----------------------------------------------------------------------------
// Get PD hw_lif_id from lif.
//-------------------------------------------------------------------------

// uint32_t pd_get_hw_lif_id(lif_t *lif)
hal_ret_t
pd_get_hw_lif_id (pd_get_hw_lif_id_args_t *args)
{
    lif_t *lif = args->lif;

    pd_lif_t  *lif_pd = (pd_lif_t *)lif->pd_lif;

    args->hw_lifid = lif_pd->hw_lif_id;

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
