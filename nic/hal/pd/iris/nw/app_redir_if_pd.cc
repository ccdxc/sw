#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/hal/pd/iris/nw/app_redir_if_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/p4/iris/include/defines.h"

namespace hal {
namespace pd {

static hal_ret_t pd_app_redir_if_alloc_res(pd_app_redir_if_t *pd_app_redir_if,
                                           if_t *intf);

// ----------------------------------------------------------------------------
// APPREDIR If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_app_redir_if_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_app_redir_if_t    *pd_app_redir_if;

    HAL_TRACE_DEBUG("{}: Creating pd state for app_redir_if: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create APPREDIR If PD
    pd_app_redir_if = pd_app_redir_if_alloc_init();
    if (pd_app_redir_if == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    app_redir_if_link_pi_pd(pd_app_redir_if, args->intf);

    // Allocate Resources
    ret = pd_app_redir_if_alloc_res(pd_app_redir_if, args->intf);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("{}: Unable to alloc. resources for app_redir_if: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        pd_app_redir_if_cleanup(pd_app_redir_if);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD app_redir_if Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_app_redir_if_update (pd_if_update_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD app_redir_if Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_app_redir_if_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_app_redir_if_t  *app_redir_if_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:deleting pd state for app_redir_if {}",
                    __FUNCTION__, args->intf->if_id);
    app_redir_if_pd = (pd_app_redir_if_t *)args->intf->pd_if;

    // pd cleanup
    ret = pd_app_redir_if_cleanup(app_redir_if_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd app_redir_if delete",
                      __FUNCTION__);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD App redir if Get
//-----------------------------------------------------------------------------
hal_ret_t
pd_app_redir_if_get (pd_if_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *hal_if = args->hal_if;
    pd_app_redir_if_t       *app_redir_if_pd =  (pd_app_redir_if_t*)hal_if->pd_if;
    InterfaceGetResponse    *rsp = args->rsp;

    auto app_info = rsp->mutable_status()->mutable_app_redir_info();
    app_info->set_lport_id(app_redir_if_pd->lport_id);

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD APPREDIR if
// ----------------------------------------------------------------------------
static hal_ret_t 
pd_app_redir_if_alloc_res(pd_app_redir_if_t *pd_app_redir_if,
                          if_t *intf)
{
    lif_t                         *lif;
    hal_ret_t                     ret = HAL_RET_OK;
    pd_lif_get_lport_id_args_t    args = { 0 };

    /*
     * PD resources should have already allocated when the LIF was created
     */
    lif = if_get_lif(intf);
    if (lif) {
        args.pi_lif = lif;
        pd_lif_get_lport_id(&args);
        pd_app_redir_if->lport_id = args.lport_id;
        // pd_app_redir_if->lport_id = lif_get_lport_id(lif);
        HAL_TRACE_DEBUG("{}: if_id:{} lif_id:{} lport_id:{}", 
                        __FUNCTION__, if_get_if_id((if_t *)pd_app_redir_if->pi_if),
                        lif_get_lif_id(lif), pd_app_redir_if->lport_id);
    } else {
        HAL_TRACE_ERR("{}: if_id:{} lif does not exist", 
                        __FUNCTION__, if_get_if_id((if_t *)pd_app_redir_if->pi_if));
        return HAL_RET_LIF_NOT_FOUND;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD APPREDIR if
// ----------------------------------------------------------------------------
hal_ret_t 
pd_app_redir_if_dealloc_res(pd_app_redir_if_t *pd_app_redir_if)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD app_redir_if Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If 
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
pd_app_redir_if_cleanup(pd_app_redir_if_t *upif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!upif_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = pd_app_redir_if_dealloc_res(upif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for app_redir_if: {}", 
                      __FUNCTION__, ((if_t *)(upif_pd->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    app_redir_if_delink_pi_pd(upif_pd, (if_t *)upif_pd->pi_if);

    // Freeing PD
    pd_app_redir_if_free(upif_pd);
end:
    return ret;
}

pd_lif_t *
pd_app_redir_if_get_pd_lif(pd_app_redir_if_t *pd_app_redir_if)
{
    if_t        *pi_if = NULL;
    pd_lif_t    *pd_lif = NULL;
    lif_t       *pi_lif = NULL;

    pi_if = (if_t *)pd_app_redir_if->pi_if;
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
pd_app_redir_if_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_app_redir_if_t       *pd_app_redir_if_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

    pd_app_redir_if_clone = pd_app_redir_if_alloc_init();
    if (pd_app_redir_if_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_app_redir_if_clone, hal_if->pd_if, sizeof(pd_app_redir_if_t));

    app_redir_if_link_pi_pd(pd_app_redir_if_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_app_redir_if_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_app_redir_if_t      *app_redir_if_pd;

    app_redir_if_pd = (pd_app_redir_if_t *)args->intf->pd_if;
    pd_app_redir_if_pd_mem_free(app_redir_if_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
