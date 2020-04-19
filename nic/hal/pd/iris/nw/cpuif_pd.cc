#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "gen/proto/interface.pb.h"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/hal/pd/iris/nw/cpuif_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// CPU If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_cpuif_t           *pd_cpuif;

    HAL_TRACE_DEBUG("Creating pd state for CPUif: {}",
                    if_get_if_id(args->intf));

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
        HAL_TRACE_ERR("Unable to alloc. resources for Uplinnkif: {}",
                      if_get_if_id(args->intf));
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
pd_cpuif_update (pd_if_update_args_t *args)
{
    hal_ret_t   ret;
    pd_cpuif_t  *pd_cpuif = (pd_cpuif_t *)args->intf_clone->pd_if;

    // Program Output Mapping
    ret = pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif, TABLE_OPER_UPDATE);

    return ret;
}

//-----------------------------------------------------------------------------
// PD cpuif Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_cpuif_t  *cpuif_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Deleting pd state for cpuif {}",
                    args->intf->if_id);
    cpuif_pd = (pd_cpuif_t *)args->intf->pd_if;

    // deprogram HW
    ret = pd_cpuif_deprogram_hw(cpuif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }

    // pd cleanup
    ret = pd_cpuif_cleanup(cpuif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd cpuif delete");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Cpuif Get
//-----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_get (pd_if_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *hal_if = args->hal_if;
    pd_cpuif_t              *cpuif_pd = (pd_cpuif_t *)hal_if->pd_if;
    InterfaceGetResponse    *rsp = args->rsp;

    auto cpu_info = rsp->mutable_status()->mutable_cpu_info();
    cpu_info->set_cpu_lport_id(cpuif_pd->cpu_lport_id);

    return ret;
}

// ----------------------------------------------------------------------------
// Restoring data post-upgrade
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_restore_data (pd_if_restore_args_t *args)
{
    hal_ret_t       ret       = HAL_RET_OK;
    if_t            *hal_if   = args->hal_if;
    pd_cpuif_t      *pd_cpuif = (pd_cpuif_t *)hal_if->pd_if;
    auto cpu_info             = args->if_status->cpu_info();

    pd_cpuif->cpu_lport_id = cpu_info.cpu_lport_id();

    return ret;
}

// ----------------------------------------------------------------------------
// CPU If Restore
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_restore (pd_if_restore_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_cpuif_t  *pd_cpuif;

    HAL_TRACE_DEBUG("Restoring pd state for if_id: {}",
                    if_get_if_id(args->hal_if));

    // Create Uplink if
    pd_cpuif = pd_cpuif_alloc_init();
    if (pd_cpuif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    cpuif_link_pi_pd(pd_cpuif, args->hal_if);

    // Restore PD info
    ret = pd_cpuif_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore PD data for IF: {}, err:{}",
                      if_get_if_id(args->hal_if), ret);
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
    HAL_TRACE_DEBUG("if_id:{} Allocated lport_id:{}",
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
        HAL_TRACE_ERR("unable to dealloc res for cpuif: {}",
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
        HAL_TRACE_ERR("unable to deprogram hw");
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
    sdk_ret_t                   sdk_ret;
    directmap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->remove(pd_cpuif->cpu_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram omapping table at: {}",
                      pd_cpuif->cpu_lport_id);
    } else {
        HAL_TRACE_ERR("deprogrammed omapping table at: {}",
                      pd_cpuif->cpu_lport_id);
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
    ret = pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif, TABLE_OPER_INSERT);

    return ret;
}



// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_cpu data.action_u.output_mapping_redirect_to_cpu
hal_ret_t
pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif, table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    output_mapping_actiondata_t   data;
    directmap                   *dm_omap = NULL;
    pd_lif_t                    *pd_lif = NULL;
    if_t                        *pi_if = (if_t *)pd_cpuif->pi_if;

    memset(&data, 0, sizeof(data));

    if (!pi_if->allow_rx) {
        data.action_id = OUTPUT_MAPPING_OUTPUT_MAPPING_DROP_ID;
    } else {
        pd_lif = pd_cpuif_get_pd_lif(pd_cpuif);
        data.action_id = OUTPUT_MAPPING_REDIRECT_TO_CPU_ID;
        om_cpu.dst_lif = pd_lif->hw_lif_id; // 33
        // TODO: Fix this
        om_cpu.egress_mirror_en = 0;
        om_cpu.control_tm_oq = 0;
        om_cpu.cpu_copy_tm_oq = 0;
    }

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = dm_omap->insert_withid(&data, pd_cpuif->cpu_lport_id);
    } else {
        sdk_ret = dm_omap->update(pd_cpuif->cpu_lport_id, &data);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("cpu if_id:{} Unable to program. err: {}", 
                      pi_if->if_id, ret);
    } else {
        HAL_TRACE_DEBUG("cpu if_id:{} Success", pi_if->if_id);
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
    SDK_ASSERT_RETURN(pi_if != NULL, 0);

    pi_lif = if_get_lif(pi_if);
    SDK_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    SDK_ASSERT(pi_lif != NULL);

    return pd_lif;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpuif_t       *pd_cpuif_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

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
pd_cpuif_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_cpuif_t      *cpuif_pd;

    cpuif_pd = (pd_cpuif_t *)args->intf->pd_if;
    pd_cpuif_pd_mem_free(cpuif_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
