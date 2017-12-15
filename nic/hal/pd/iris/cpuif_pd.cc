#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/pd/iris/lif_pd.hpp"
#include "nic/hal/pd/iris/cpuif_pd.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// CPU If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_cpuif_create(pd_if_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_cpuif_t           *pd_cpuif;

    HAL_TRACE_DEBUG("PD-CPUif::{}: Creating pd state for CPUif: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create CPU If PD
    pd_cpuif = pd_cpuif_alloc_init();
    if (pd_cpuif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    cpuif_link_pi_pd(pd_cpuif, args->intf);

    // Allocate Resources
    ret = pd_cpuif_alloc_res(pd_cpuif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-CPUIF::{}: Unable to alloc. resources for Uplinnkif: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_cpuif_program_hw(pd_cpuif);

end:
    if (ret != HAL_RET_OK) {
        pd_cpuif_cleanup(pd_cpuif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD CPUIf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_update (pd_if_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD cpuif Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_delete (pd_if_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_cpuif_t  *cpuif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-cpuif:{}:deleting pd state for cpuif {}",
                    __FUNCTION__, args->intf->if_id);
    cpuif_pd = (pd_cpuif_t *)args->intf->pd_if;

    // deprogram HW
    ret = pd_cpuif_deprogram_hw(cpuif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-cpuif:{}:unable to deprogram hw", __FUNCTION__);
    }

    // pd cleanup
    ret = pd_cpuif_cleanup(cpuif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-cpuif:{}:failed pd cpuif delete",
                      __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD CPU if
// ----------------------------------------------------------------------------
hal_ret_t 
pd_cpuif_alloc_res(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t            ret = HAL_RET_OK;
    //indexer::status      rs = indexer::SUCCESS;

    pd_cpuif->cpu_lport_id = CPU_LPORT;
    // Allocate lport
    //rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_cpuif->
    //        cpu_lport_id);
    //if (rs != indexer::SUCCESS) {
    //    return HAL_RET_NO_RESOURCE;
    //}
    HAL_TRACE_DEBUG("PD-CPUIf:{}: if_id:{} Allocated lport_id:{}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_cpuif->pi_if),
                    pd_cpuif->cpu_lport_id);

    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD CPU if
// ----------------------------------------------------------------------------
hal_ret_t 
pd_cpuif_dealloc_res(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD cpuif Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If 
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_cleanup(pd_cpuif_t *upif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!upif_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = pd_cpuif_dealloc_res(upif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-cpuif:{}: unable to dealloc res for cpuif: {}", 
                      __FUNCTION__, 
                      ((if_t *)(upif_pd->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    cpuif_delink_pi_pd(upif_pd, (if_t *)upif_pd->pi_if);

    // Freeing PD
    pd_cpuif_free(upif_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_deprogram_hw (pd_cpuif_t *pd_cpuif)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-Program Output Mapping Table
    ret = pd_cpuif_pd_depgm_output_mapping_tbl(pd_cpuif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-cpuif:{}:unable to deprogram hw", __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram output mapping table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_pd_depgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    DirectMap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);
    
    ret = dm_omap->remove(pd_cpuif->cpu_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-cpuif:{}:unable to deprogram omapping table",
                __FUNCTION__, pd_cpuif->cpu_lport_id);
    } else {
        HAL_TRACE_ERR("pd-cpuif:{}:deprogrammed omapping table",
                __FUNCTION__, pd_cpuif->cpu_lport_id);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_program_hw(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t            ret;

    // Program Output Mapping 
    ret = pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif);

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_cpu data.output_mapping_action_u.output_mapping_redirect_to_cpu
hal_ret_t
pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    output_mapping_actiondata   data;
    DirectMap                   *dm_omap = NULL;
    pd_lif_t                    *pd_lif = NULL;

    memset(&data, 0, sizeof(data));

    pd_lif = pd_cpuif_get_pd_lif(pd_cpuif);

    data.actionid = OUTPUT_MAPPING_REDIRECT_TO_CPU_ID;
    om_cpu.dst_lif = pd_lif->hw_lif_id; // 1023 
    // TODO: Fix this
    om_cpu.egress_mirror_en = 0;
    om_cpu.control_tm_oqueue = 0;
    om_cpu.cpu_copy_tm_oqueue = 0;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    ret = dm_omap->insert_withid(&data, pd_cpuif->cpu_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-cpuIf::{}: lif_id:{} Unable to program",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_DEBUG("PD-cpuIf::{}: lif_id:{} Success",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }
    return ret;
}

pd_lif_t *
pd_cpuif_get_pd_lif(pd_cpuif_t *pd_cpuif)
{
    if_t        *pi_if = NULL;
    pd_lif_t    *pd_lif = NULL;
    lif_t       *pi_lif = NULL;

    pi_if = (if_t *)pd_cpuif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_lif = if_get_lif(pi_if);
    HAL_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    HAL_ASSERT(pi_lif != NULL);

    return pd_lif;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_make_clone(if_t *hal_if, if_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpuif_t       *pd_cpuif_clone = NULL;

    pd_cpuif_clone = pd_cpuif_alloc_init();
    if (pd_cpuif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_cpuif_clone, hal_if->pd_if, sizeof(pd_cpuif_t));

    cpuif_link_pi_pd(pd_cpuif_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_mem_free(pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_cpuif_t      *cpuif_pd;

    cpuif_pd = (pd_cpuif_t *)args->intf->pd_if;
    pd_cpuif_pd_mem_free(cpuif_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
