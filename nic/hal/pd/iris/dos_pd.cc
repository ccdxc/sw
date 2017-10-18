#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/dos_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/dos_api.hpp"

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD DoS Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_create (pd_dos_policy_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_dos_policy_t      *pd_dosp;

    HAL_TRACE_DEBUG("pd-dos:{}: creating pd state ",
                    __FUNCTION__);

    // Create dos PD
    pd_dosp = dos_pd_alloc_init();
    if (pd_dosp == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    dos_link_pi_pd(pd_dosp, args->dos_policy);

    // Allocate Resources
    ret = dos_pd_alloc_res(pd_dosp);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-DoS::{}: Unable to alloc. resources",
                      __FUNCTION__);
        goto end;
    }

    // Program HW
    ret = dos_pd_program_hw(pd_dosp, TRUE);

end:
    if (ret != HAL_RET_OK) {
        dos_pd_cleanup(pd_dosp);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD DoS Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_update (pd_dos_policy_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_dos_policy_t   *pd_dosp;

    HAL_TRACE_DEBUG("pd-dos:{}: updating pd state ",
                    __FUNCTION__);

    pd_dosp = (pd_dos_policy_t *)args->clone_policy->pd;
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("pd-dos:{}: unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }

    return ret;

#if 0
    pd_dosp = (pd_dos_policy_t *) args->dos_policy->pd;
    // Cache the PI pointer since the ptr in the
    // args is a local copy
    void *cached_pi_ptr = pd_dosp->pi_dos_policy;
     
    pd_dosp->pi_dos_policy = (void *) args->dos_policy; 
    // Program HW
    ret = dos_pd_program_hw(pd_dosp, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-DoS::{}: Unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }
    // Revert back to the cached PI ptr
    pd_dosp->pi_dos_policy = cached_pi_ptr;
#endif
}

//-----------------------------------------------------------------------------
// PD DoS Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_delete (pd_dos_policy_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t  *dos_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->dos_policy->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-dos:{}:deleting pd state for dos policy handle {}",
                    __FUNCTION__, args->dos_policy->hal_handle);
    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;

    // deprogram HW
    ret = dos_pd_deprogram_hw(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = dos_pd_cleanup(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}:failed pd dos delete",
                      __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_deprogram_hw (pd_dos_policy_t *dos_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-program DDoS service table
    // De-program DDoS src_vrf table
    // De-program DDoS src-dst table

    // De-program DDoS service policer
    // De-program DDoS src_vrf policer
    // De-program DDoS src-dst policer
    // De-program Input properties Table

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t 
dos_pd_alloc_res(pd_dos_policy_t *pd_nw)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t 
dos_pd_dealloc_res(pd_dos_policy_t *pd_nw)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Tenant
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
dos_pd_cleanup(pd_dos_policy_t *dos_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!dos_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = dos_pd_dealloc_res(dos_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-dos:{}: unable to dealloc res for dos hdl: {}", 
                      __FUNCTION__, 
                      ((dos_policy_t*)(dos_pd->pi_dos_policy))->hal_handle);
        goto end;
    }

    // Delinking PI<->PD
    dos_delink_pi_pd(dos_pd, (dos_policy_t *)dos_pd->pi_dos_policy);

    // Freeing PD
    dos_pd_free(dos_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
dos_pd_program_hw(pd_dos_policy_t *pd_nw, bool create)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program DDoS service table
    // Program DDoS src_vrf table
    // Program DDoS src-dst table

    // Program DDoS service policer
    // Program DDoS src_vrf policer
    // Program DDoS src-dst policer

    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
dos_link_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t *pi_nw)
{
    pd_nw->pi_dos_policy = pi_nw;
    dos_set_pd_dos(pi_nw, pd_nw);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
dos_delink_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t  *pi_nw)
{
    pd_nw->pi_dos_policy = NULL;
    dos_set_pd_dos(pi_nw, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_make_clone(dos_policy_t *dosp, dos_policy_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_dos_policy_t     *pd_dosp_clone = NULL;

    pd_dosp_clone = dos_pd_alloc_init();
    if (pd_dosp_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_dosp_clone, dosp->pd, sizeof(pd_dos_policy_t));

    dos_link_pi_pd(pd_dosp_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_dos_policy_mem_free(pd_dos_policy_args_t *args)
{
    pd_dos_policy_t       *dos_pd;
    hal_ret_t             ret = HAL_RET_OK;

    dos_pd = (pd_dos_policy_t *)args->dos_policy->pd;
    dos_pd_mem_free(dos_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
