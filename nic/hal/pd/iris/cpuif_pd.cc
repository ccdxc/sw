#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <interface.pb.h>
#include <lif_pd.hpp>
#include <cpuif_pd.hpp>
#include "l2seg_pd.hpp"
#include "if_pd_utils.hpp"
#include <defines.h>

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
    link_pi_pd(pd_cpuif, args->intf);

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
        unlink_pi_pd(pd_cpuif, args->intf);
        pd_cpuif_free(pd_cpuif);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize CPU If PD Instance
// ----------------------------------------------------------------------------
pd_cpuif_t *
pd_cpuif_alloc_init(void)
{
    return pd_cpuif_init(pd_cpuif_alloc());
}

// ----------------------------------------------------------------------------
// Allocate CPU IF Instance
// ----------------------------------------------------------------------------
pd_cpuif_t *
pd_cpuif_alloc (void)
{
    pd_cpuif_t    *cpuif;

    cpuif = (pd_cpuif_t *)g_hal_state_pd->cpuif_pd_slab()->alloc();
    if (cpuif == NULL) {
        return NULL;
    }
    return cpuif;
}

// ----------------------------------------------------------------------------
// Initialize CPU IF PD instance
// ----------------------------------------------------------------------------
pd_cpuif_t *
pd_cpuif_init (pd_cpuif_t *cpuif)
{
    // Nothing to do currently
    if (!cpuif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return cpuif;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD CPU if
// ----------------------------------------------------------------------------
hal_ret_t 
pd_cpuif_alloc_res(pd_cpuif_t *pd_cpuif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_cpuif->
            cpu_lport_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-CPUIf:{}: if_id:{} Allocated lport_id:{}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_cpuif->pi_if),
                    pd_cpuif->cpu_lport_id);

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
    om_cpu.tm_oqueue = 0;

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

// ----------------------------------------------------------------------------
// Freeing CPU IF PD
// ----------------------------------------------------------------------------
hal_ret_t
pd_cpuif_free (pd_cpuif_t *cpuif)
{
    g_hal_state_pd->cpuif_pd_slab()->free(cpuif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_cpuif_t *pd_cpuif, if_t *pi_if)
{
    pd_cpuif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_cpuif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_cpuif_t *pd_cpuif, if_t *pi_if)
{
    pd_cpuif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
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

}    // namespace pd
}    // namespace hal
