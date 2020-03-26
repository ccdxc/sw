#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/gft/uplinkif_pd.hpp"
#include "asic/pd/pd.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// uplink if create
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_create (pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_uplinkif_t        *pd_upif;

    HAL_TRACE_DEBUG("creating pd state for if_id: {}",
                    if_get_if_id(args->intf));

    // create lif PD
    pd_upif = uplinkif_pd_alloc_init();
    if (pd_upif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link PI & PD
    uplinkif_link_pi_pd(pd_upif, args->intf);

    // allocate Resources
    ret = uplinkif_pd_alloc_res(pd_upif);
    if (ret != HAL_RET_OK) {
        // no Resources, dont allocate PD
        HAL_TRACE_ERR("Failed to alloc. resources for if_id: {}",
                      if_get_if_id(args->intf));
        goto end;
    }

    // program HW
    ret = uplinkif_pd_program_hw(pd_upif);

end:

    if (ret != HAL_RET_OK) {
        uplinkif_pd_cleanup(pd_upif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// pd uplinkif update
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_update (pd_if_update_args_t *args)
{
    // nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// pd uplinkif delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkif_t  *uplinkif_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for uplinkif {}", args->intf->if_id);
    uplinkif_pd = (pd_uplinkif_t *)args->intf->pd_if;

    // deprogram HW
    ret = uplinkif_pd_deprogram_hw(uplinkif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram hw");
    }

    // pd cleanup
    ret = uplinkif_pd_cleanup(uplinkif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd uplinkif delete");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// allocate resources for PD uplink if
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_alloc_res (pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // allocate lif hwid
    pd_upif->hw_lif_id = if_allocate_hwlif_id();
    if (pd_upif->hw_lif_id == INVALID_INDEXER_INDEX) {
        HAL_TRACE_ERR("failed to alloc hw_lif_id err: {}", rs);
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated hw_lif_id: {}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->hw_lif_id);

    return ret;
}

//-----------------------------------------------------------------------------
// de-allocate resources
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_dealloc_res (pd_uplinkif_t *upif_pd)
{
    hal_ret_t           ret = HAL_RET_OK;

    if (upif_pd->hw_lif_id != INVALID_INDEXER_INDEX) {
        // tODO: Have to free up the index from lif manager

        HAL_TRACE_DEBUG("freed hw_lif_id: {}", upif_pd->hw_lif_id);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// pd uplinkif cleanup
//  - release all resources
//  - delink PI <-> PD
//  - free pd if
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_cleanup (pd_uplinkif_t *upif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!upif_pd) {
        // nothing to do
        goto end;
    }

    // releasing resources
    ret = uplinkif_pd_dealloc_res(upif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to dealloc res for uplinkif: {}",
                      ((if_t *)(upif_pd->pi_if))->if_id);
        goto end;
    }

    // delinking PI<->PD
    uplinkif_delink_pi_pd(upif_pd, (if_t *)upif_pd->pi_if);

    // freeing PD
    uplinkif_pd_free(upif_pd);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// deprogram HW
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_deprogram_hw (pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;

    // de program TM register
    ret = uplinkif_pd_depgm_tm_register(pd_upif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram hw");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// de-program TM Register
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_depgm_tm_register (pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));
    sdk_ret = asicpd_tm_uplink_lif_set(tm_oport, 0);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram for if_id: {}",
                      if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("deprogrammed for if_id: {} "
                        "iport:{} => hwlif: {}",
                        if_get_if_id((if_t *)pd_upif->pi_if),
                        tm_oport, 0);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// program HW
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_program_hw (pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret;

    // TODO : program TM table port_num -> lif_hw_id
    ret = uplinkif_pd_pgm_tm_register(pd_upif);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

//-----------------------------------------------------------------------------
// program TM Register
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_pgm_tm_register(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));

    sdk_ret = asicpd_tm_uplink_lif_set(tm_oport, pd_upif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program for if_id: {}",
                      if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("programmed for if_id: {} "
                        "iport:{} => hwlif: {}",
                        if_get_if_id((if_t *)pd_upif->pi_if),
                        tm_oport, pd_upif->hw_lif_id);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// makes a clone
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_make_clone (pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_uplinkif_t       *pd_uplinkif_clone = NULL;
    if_t                *hal_if = args->hal_if;
    if_t                *clone = args->clone;

    pd_uplinkif_clone = uplinkif_pd_alloc_init();
    if (pd_uplinkif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    memcpy(pd_uplinkif_clone, hal_if->pd_if, sizeof(pd_uplinkif_t));
    uplinkif_link_pi_pd(pd_uplinkif_clone, clone);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// frees PD memory without indexer free.
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_mem_free (pd_if_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkif_t  *upif_pd;

    upif_pd = (pd_uplinkif_t *)args->intf->pd_if;
    uplinkif_pd_mem_free(upif_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
