// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/pd/gft/if_pd.hpp"
#include "nic/hal/pd/gft/lif_pd.hpp"
#include "nic/hal/pd/gft/enicif_pd.hpp"
#include "nic/p4/nw/include/defines.h"
// #include "nic/hal/pd/gft/p4pd_defaults.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Enic If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_enicif_t          *pd_enicif;

    HAL_TRACE_DEBUG("{}: creating pd state for enicif: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create Enic If PD
    pd_enicif = pd_enicif_alloc_init();
    if (pd_enicif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    pd_enicif_link_pi_pd(pd_enicif, args->intf);

    // Allocate Resources
    ret = pd_enicif_alloc_res(pd_enicif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("{}: unable to alloc. resources for enicif: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_enicif_program_hw(pd_enicif);

end:
    if (ret != HAL_RET_OK) {
        pd_enicif_cleanup(pd_enicif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_update (pd_if_update_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_enicif_t    *enicif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:deleting pd state for enicif: {}",
                    __FUNCTION__, args->intf->if_id);
    enicif_pd = (pd_enicif_t *)args->intf->pd_if;

    // deprogram HW
    ret = pd_enicif_deprogram_hw(enicif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram hw", __FUNCTION__);
    }

    ret = pd_enicif_cleanup(enicif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd enicif delete",
                      __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD EnicIf
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_alloc_res(pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_dealloc_res(pd_enicif_t *pd_enicif)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If 
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_cleanup(pd_enicif_t *pd_enicif)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!pd_enicif) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = pd_enicif_dealloc_res(pd_enicif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for enicif: {}", 
                      __FUNCTION__, 
                      ((if_t *)(pd_enicif->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    pd_enicif_delink_pi_pd(pd_enicif, (if_t *)pd_enicif->pi_if);

    // Freeing PD
    pd_enicif_free(pd_enicif);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_deprogram_hw (pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_program_hw(pd_enicif_t *pd_enicif)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_enicif_t       *pd_enicif_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

    pd_enicif_clone = pd_enicif_alloc_init();
    if (pd_enicif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_enicif_clone, hal_if->pd_if, sizeof(pd_enicif_t));

    pd_enicif_link_pi_pd(pd_enicif_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_enicif_t  *upif_pd;

    upif_pd = (pd_enicif_t *)args->intf->pd_if;
    pd_enicif_free(upif_pd);

    return ret;
}

pd_lif_t *
pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif)
{
    if_t        *pi_if = NULL;
    pd_lif_t    *pd_lif = NULL;
    lif_t       *pi_lif = NULL;

    pi_if = (if_t *)pd_enicif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_lif = if_get_lif(pi_if);
    HAL_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    HAL_ASSERT(pi_lif != NULL);

    return pd_lif;
}

}    // namespace pd
}    // namespace hal
