#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/copp_pd.hpp"
#include "nic/hal/pd/iris/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/qos_api.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
copp_pd_link_pi_pd (pd_copp_t *pd_copp, copp_t *pi_copp)
{
    pd_copp->pi_copp = pi_copp;
    pi_copp->pd = pd_copp;
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
copp_pd_delink_pi_pd (pd_copp_t *pd_copp, copp_t *pi_copp)
{
    if (pd_copp) {
        pd_copp->pi_copp = NULL;
    }
    if (pi_copp) {
        pi_copp->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Copp
// ----------------------------------------------------------------------------
static hal_ret_t
copp_pd_alloc_res (pd_copp_t *pd_copp)
{

    // Allocate any hardware resources
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Copp
// ----------------------------------------------------------------------------
static hal_ret_t
copp_pd_dealloc_res (pd_copp_t *pd_copp)
{
    // Deallocate any hardware resources
    return HAL_RET_OK;
}


static hal_ret_t
copp_pd_cleanup_copp_tbl (pd_copp_t *pd_copp)
{
    hal_ret_t       ret;
    sdk_ret_t       sdk_ret;
    directmap       *copp_tbl = NULL;

    copp_tbl = g_hal_state_pd->dm_table(P4TBL_ID_COPP);
    HAL_ASSERT_RETURN((copp_tbl != NULL), HAL_RET_ERR);

    sdk_ret = copp_tbl->remove(pd_copp->hw_policer_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to cleanup for copp: {}",
                      pd_copp->pi_copp->key);
    } else {
        HAL_TRACE_DEBUG("Programmed cleanup copp: {}",
                        pd_copp->pi_copp->key);
    }

    return ret;
}

static hal_ret_t
copp_pd_deprogram_hw (pd_copp_t *pd_copp)
{
    hal_ret_t   ret;
    ret = copp_pd_cleanup_copp_tbl(pd_copp);

    return ret;
}

#define COPP_ACTION(_d, _arg) _d.copp_action_u.copp_execute_copp._arg
static hal_ret_t
copp_pd_program_copp_tbl (pd_copp_t *pd_copp, bool update)
{
    hal_ret_t       ret = HAL_RET_OK;
    sdk_ret_t       sdk_ret;
    copp_t          *pi_copp = pd_copp->pi_copp;
    directmap       *copp_tbl = NULL;
    copp_actiondata d = {0};
    copp_actiondata d_mask = {0};
    uint64_t        refresh_interval_us = 60;
    uint64_t        rate_tokens = 0;
    uint64_t        burst_tokens = 0;
    uint64_t        bps_rate;


    copp_tbl = g_hal_state_pd->dm_table(P4TBL_ID_COPP);
    HAL_ASSERT_RETURN((copp_tbl != NULL), HAL_RET_ERR);

    d.actionid = COPP_EXECUTE_COPP_ID;

    bps_rate = pi_copp->policer.bps_rate;
    burst_tokens = pi_copp->policer.burst_size;

    if (bps_rate == 0) {
        COPP_ACTION(d, entry_valid) = 0;
    } else {
        COPP_ACTION(d, entry_valid) = 1;
        COPP_ACTION(d, pkt_rate) = 0;

        ret = policer_rate_per_sec_to_token_rate(bps_rate, refresh_interval_us, 
                                                 &rate_tokens, burst_tokens);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error converting rate to token rate ret {}", ret);
            return ret;
        }

        burst_tokens += rate_tokens;

        memcpy(COPP_ACTION(d, burst), &burst_tokens,
               std::min(sizeof(COPP_ACTION(d, burst)), 
                        sizeof(burst_tokens)));
        memcpy(COPP_ACTION(d, rate), &rate_tokens,
               std::min(sizeof(COPP_ACTION(d, rate)), sizeof(rate_tokens)));

        uint64_t tbkt = burst_tokens;
        memcpy(COPP_ACTION(d, tbkt), &tbkt, 
               std::min(sizeof(COPP_ACTION(d, tbkt)), 
                        sizeof(burst_tokens)));
    }

    memset(&d_mask.copp_action_u.copp_execute_copp, 0xff,
           sizeof(copp_execute_copp_t));
    COPP_ACTION(d_mask, rsvd) = 0;
    COPP_ACTION(d_mask, axi_wr_pend) = 0;
    if (update) {
        memset(COPP_ACTION(d_mask, tbkt), 0, 
               sizeof(COPP_ACTION(d_mask, tbkt)));
    }

    // TODO Fixme. Setting entry-valid to 0 until copp is verified and values
    // are determined
    COPP_ACTION(d, entry_valid) = 0;
    if (update) {
        sdk_ret = copp_tbl->update(pd_copp->hw_policer_id, &d);
    } else {
        sdk_ret = copp_tbl->insert(&d, &pd_copp->hw_policer_id);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp policer table write failure, copp {}, ret {}",
                      pi_copp->key, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("copp {} hw_policer_id {} rate {} burst {} programmed",
                    pi_copp->key,
                    pd_copp->hw_policer_id, pi_copp->policer.bps_rate,
                    pi_copp->policer.burst_size);
    return HAL_RET_OK;
}
#undef COPP_ACTION

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
static hal_ret_t
copp_pd_program_hw (pd_copp_t *pd_copp, bool update)
{
    hal_ret_t ret;
    copp_t    *copp = pd_copp->pi_copp;

    ret = copp_pd_program_copp_tbl(pd_copp, update);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming the copp table "
                      "Copp {} ret {}",
                      copp->key, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Copp Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Copp
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
static hal_ret_t
copp_pd_cleanup (pd_copp_t *pd_copp)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!pd_copp) {
        // Nothing to do
        goto end;
    }

    if (pd_copp->hw_policer_id != INVALID_INDEXER_INDEX) {
        // TODO: deprogram hw
        ret = copp_pd_deprogram_hw(pd_copp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram hw for copp: {}",
                          pd_copp->pi_copp->key);
            goto end;
        }
    }

    // Releasing resources
    ret = copp_pd_dealloc_res(pd_copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to dealloc res for copp: {}",
                      pd_copp->pi_copp->key);
        goto end;
    }

    // Delinking PI<->PD
    copp_pd_delink_pi_pd(pd_copp, pd_copp->pi_copp);

    // Freeing PD
    copp_pd_free(pd_copp);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Copp Update
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_update (pd_copp_update_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_copp_t  *pd_copp;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->copp != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->copp->pd != NULL), HAL_RET_INVALID_ARG);

    HAL_TRACE_DEBUG("updating pd state for copp:{}",
                    args->copp->key);

    pd_copp = (pd_copp_t *)args->copp->pd;

    ret = copp_pd_program_hw(pd_copp, true);
    return ret;
}

//-----------------------------------------------------------------------------
// PD Copp Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_copp_delete (pd_copp_delete_args_t *args)
{
    // Deletion of copp is not allowed
    return HAL_RET_INVALID_ARG;

#if 0
    hal_ret_t ret = HAL_RET_OK;
    pd_copp_t *pd_copp;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->copp != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->copp->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for copp {}",
                    args->copp->key);
    pd_copp = (pd_copp_t *)args->copp->pd;

    // free up the resource and memory
    ret = copp_pd_cleanup(pd_copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd copp cleanup Copp {}, ret {}",
                      args->copp->key, ret);
    }

    return ret;
#endif
}

// ----------------------------------------------------------------------------
// Copp Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_create (pd_copp_create_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;;
    pd_copp_t *pd_copp;

    HAL_TRACE_DEBUG("creating pd state for copp: {}",
                    args->copp->key);

    // Create copp PD
    pd_copp = copp_pd_alloc_init();
    if (pd_copp == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    copp_pd_link_pi_pd(pd_copp, args->copp);

    // Allocate Resources
    ret = copp_pd_alloc_res(pd_copp);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("Unable to alloc. resources for Copp: {} ret {}",
                      args->copp->key, ret);
        goto end;
    }

    // Program HW
    ret = copp_pd_program_hw(pd_copp, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program hw for Copp: {} ret {}",
                      args->copp->key, ret);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in programming hw for Copp: {}: ret: {}",
                      args->copp->key, ret);
        // unlink_pi_pd(pd_copp, args->copp);
        // copp_pd_free(pd_copp);
        copp_pd_cleanup(pd_copp);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_make_clone (pd_copp_make_clone_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_copp_t *pd_copp_clone = NULL;
    copp_t *copp = args->copp;
    copp_t *clone = args->clone;

    pd_copp_clone = copp_pd_alloc_init();
    if (pd_copp_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_copp_clone, copp->pd, sizeof(pd_copp_t));

    copp_pd_link_pi_pd(pd_copp_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_mem_free (pd_copp_mem_free_args_t *args)
{
    pd_copp_t        *pd_copp;

    pd_copp = (pd_copp_t *)args->copp->pd;
    copp_pd_mem_free(pd_copp);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
