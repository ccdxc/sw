// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/gft/vrf_pd.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/src/proxy.hpp"

namespace hal {
namespace pd {

#define inp_prop data.input_properties_action_u.input_properties_input_properties

static hal_ret_t vrf_pd_program_hw (pd_vrf_t *vrf_pd);
static hal_ret_t vrf_pd_deprogram_hw (pd_vrf_t *vrf_pd);

//-----------------------------------------------------------------------------
// PD vrf Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_create (pd_vrf_create_args_t *args)
{
    hal_ret_t               ret;
    pd_vrf_t                *vrf_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:creating pd state for vrf {}",
                    __FUNCTION__, args->vrf->vrf_id);

    // allocate PD vrf state
    vrf_pd = vrf_pd_alloc_init();
    if (vrf_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    link_pi_pd(vrf_pd, args->vrf);

    // allocate resources
    ret = vrf_pd_alloc_res(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to allocated resources", 
                      __FUNCTION__);
        goto end;
    }

    // program hw
    ret = vrf_pd_program_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to program hw", __FUNCTION__);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        vrf_pd_cleanup(vrf_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_update (pd_vrf_update_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_delete (pd_vrf_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_t    *vrf_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->vrf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->vrf->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:Deleting pd state for vrf {}",
                    __FUNCTION__, args->vrf->vrf_id);
    vrf_pd = (pd_vrf_t *)args->vrf->pd;

    // deprogram HW
    ret = vrf_pd_deprogram_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = vrf_pd_cleanup(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd vrf delete",
                      __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_deprogram_hw (pd_vrf_t *vrf_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_program_hw (pd_vrf_t *vrf_pd)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_alloc_res(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_dealloc_res(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Cleanup
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_cleanup(pd_vrf_t *vrf_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!vrf_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = vrf_pd_dealloc_res(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for vrf: {}", 
                      __FUNCTION__, 
                      ((vrf_t *)(vrf_pd->vrf))->vrf_id);
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(vrf_pd, (vrf_t *)vrf_pd->vrf);

    // Freeing PD
    vrf_pd_free(vrf_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten)
{
    pd_ten->vrf = pi_ten;
    pi_ten->pd = pd_ten;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
delink_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten)
{
    if (pd_ten) {
        pd_ten->vrf = NULL;
    }
    if (pi_ten) {
        pi_ten->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_make_clone(pd_vrf_make_clone_args_t *args)
{
    hal_ret_t     ret           = HAL_RET_OK;
    pd_vrf_t      *pd_ten_clone = NULL;
    vrf_t         *ten, *clone;

    ten = args->vrf;
    clone = args->clone;

    pd_ten_clone = vrf_pd_alloc_init();
    if (pd_ten_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ten_clone, ten->pd, sizeof(pd_vrf_t));

    link_pi_pd(pd_ten_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_mem_free(pd_vrf_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_t    *vrf_pd;

    vrf_pd = (pd_vrf_t *)args->vrf->pd;
    vrf_pd_mem_free(vrf_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
