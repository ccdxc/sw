#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/internal/copp_pd.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos_api.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
copp_pd_link_pi_pd (pd_copp_t *pd_copp, copp_t *pi_copp)
{
    pd_copp->pi_copp = pi_copp;
    copp_set_pd_copp(pi_copp, pd_copp);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
copp_pd_delink_pi_pd (pd_copp_t *pd_copp, copp_t *pi_copp)
{
    pd_copp->pi_copp = NULL;
    copp_set_pd_copp(pi_copp, NULL);
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
    SDK_ASSERT_RETURN((copp_tbl != NULL), HAL_RET_ERR);

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
copp_pd_reset_copp_action_tbl (pd_copp_t *pd_copp, bool insert)
{
    hal_ret_t       ret = HAL_RET_OK;
    sdk_ret_t       sdk_ret;
    copp_t          *pi_copp = pd_copp->pi_copp;
    directmap       *copp_action_tbl = NULL;
    copp_action_actiondata_t d = {0};

    copp_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_COPP_ACTION);
    SDK_ASSERT_RETURN((copp_action_tbl != NULL), HAL_RET_ERR);

    d.action_id = COPP_EXECUTE_COPP_ID;

    if (insert) {
        sdk_ret = copp_action_tbl->insert_withid(&d, pd_copp->hw_policer_id);
    } else {
        sdk_ret = copp_action_tbl->remove(pd_copp->hw_policer_id);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp policer action table write failure, copp {}, ret {}",
                      pi_copp->key, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Copp action : {} hw_policer_id: {} programmed",
                    pi_copp->key,
                    pd_copp->hw_policer_id);
    return HAL_RET_OK;
}

static hal_ret_t
copp_pd_deprogram_hw (pd_copp_t *pd_copp)
{
    hal_ret_t ret;
    copp_t    *pi_copp = pd_copp->pi_copp;

    ret = copp_pd_cleanup_copp_tbl(pd_copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Copp {} error cleaning up copp tbl ret {}",
                      pi_copp->key, ret);
        return ret;
    }

    ret = copp_pd_reset_copp_action_tbl(pd_copp, false);

    return ret;
}

#define COPP_ACTION(_d, _arg) _d.action_u.copp_execute_copp._arg
static hal_ret_t
copp_pd_program_copp_tbl (pd_copp_t *pd_copp, bool update, bool is_restore)
{
    hal_ret_t       ret = HAL_RET_OK;
    sdk_ret_t       sdk_ret;
    copp_t          *pi_copp = pd_copp->pi_copp;
    directmap       *copp_tbl = NULL;
    copp_actiondata_t d = {0};
    copp_actiondata_t d_mask = {0};
    uint64_t        refresh_interval_us = HAL_DEFAULT_COPP_REFRESH_INTERVAL;
    uint64_t        rate_tokens = 0;
    uint64_t        burst_tokens = 0;
    uint64_t        rate;

    copp_tbl = g_hal_state_pd->dm_table(P4TBL_ID_COPP);
    SDK_ASSERT_RETURN((copp_tbl != NULL), HAL_RET_ERR);

    d.action_id = COPP_EXECUTE_COPP_ID;

    rate = pi_copp->policer.rate;

    if (rate == 0) {
        COPP_ACTION(d, entry_valid) = 0;
    } else {
        COPP_ACTION(d, entry_valid) = 1;
        COPP_ACTION(d, pkt_rate) =
            pi_copp->policer.type == POLICER_TYPE_PPS ? 1 : 0;

        ret = policer_to_token_rate(&pi_copp->policer,
                                    refresh_interval_us,
                                    &rate_tokens, &burst_tokens);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error converting rate to token rate ret {}", ret);
            return ret;
        }

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

    memset(&d_mask.action_u.copp_execute_copp, 0xff,
           sizeof(copp_execute_copp_t));
    COPP_ACTION(d_mask, rsvd) = 0;
    COPP_ACTION(d_mask, axi_wr_pend) = 0;
    if (update) {
        memset(COPP_ACTION(d_mask, tbkt), 0,
               sizeof(COPP_ACTION(d_mask, tbkt)));
    }

    // TODO Fixme. Setting entry-valid to 0 until copp is verified and values
    // are determined
    if (!is_platform_type_hw()) {
        COPP_ACTION(d, entry_valid) = 0;
    }
    if (is_restore) {
        sdk_ret = copp_tbl->insert_withid(&d, pd_copp->hw_policer_id, &d_mask);
    } else if (update) {
        sdk_ret = copp_tbl->update(pd_copp->hw_policer_id, &d, &d_mask);
    } else {
        sdk_ret = copp_tbl->insert(&d, &pd_copp->hw_policer_id, &d_mask);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("copp policer table write failure, copp {}, ret {}",
                      pi_copp->key, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("Copp: {} hw_policer_id: {} policer: {} "
                    "rate_tokens: {} burst_tokens: {} programmed",
                    pi_copp->key,
                    pd_copp->hw_policer_id,
                    pi_copp->policer,
                    rate_tokens, burst_tokens);
    return HAL_RET_OK;
}
#undef COPP_ACTION

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
static hal_ret_t
copp_pd_program_hw (pd_copp_t *pd_copp, bool update, bool is_restore = false)
{
    hal_ret_t ret;
    copp_t    *copp = pd_copp->pi_copp;

    ret = copp_pd_program_copp_tbl(pd_copp, update, is_restore);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming the copp table "
                      "Copp {} ret {}",
                      copp->key, ret);
        return ret;
    }

    if (!update) {
        ret = copp_pd_reset_copp_action_tbl(pd_copp, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the copp action table "
                          "Copp {} ret {}",
                          copp->key, ret);
            return ret;
        }
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
pd_copp_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_copp_update_args_t *args = pd_func_args->pd_copp_update;
    pd_copp_t  *pd_copp;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->copp != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->copp->pd != NULL), HAL_RET_INVALID_ARG);

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
pd_copp_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t             ret = HAL_RET_OK;
    pd_copp_delete_args_t *args = pd_func_args->pd_copp_delete;
    pd_copp_t             *pd_copp;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->copp != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->copp->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for copp {}",
                    args->copp->key);
    pd_copp = (pd_copp_t *)args->copp->pd;

    // free up the resource and memory
    ret = copp_pd_cleanup(pd_copp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd copp cleanup {}, ret {}",
                      args->copp->key, ret);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Copp Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;;
    pd_copp_create_args_t *args = pd_func_args->pd_copp_create;
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
pd_copp_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_copp_make_clone_args_t *args = pd_func_args->pd_copp_make_clone;
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

typedef struct copp_pd_policer_stats_s {
    uint64_t permitted_bytes;
    uint64_t permitted_packets;
    uint64_t denied_bytes;
    uint64_t denied_packets;
} __PACK__ copp_pd_policer_stats_t;

#define COPP_STATS(_d, _arg) _d.action_u.copp_action_copp_action._arg
static hal_ret_t
copp_pd_populate_policer_stats (qos::PolicerStats *stats_rsp, pd_copp_t *pd_copp)
{
    hal_ret_t               ret = HAL_RET_OK;
    copp_pd_policer_stats_t stats_0 = {0};
    copp_pd_policer_stats_t stats_1 = {0};
    mem_addr_t              stats_addr = 0;
    sdk_ret_t               sdk_ret;
    copp_t                  *pi_copp = pd_copp->pi_copp;
    directmap               *copp_action_tbl = NULL;
    copp_action_actiondata_t  d;

    copp_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_COPP_ACTION);
    SDK_ASSERT_RETURN((copp_action_tbl != NULL), HAL_RET_ERR);

    ret = hal_pd_stats_addr_get(P4TBL_ID_COPP_ACTION, 
                                pd_copp->hw_policer_id, &stats_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error getting stats address for copp {} hw-id {}, ret {}",
                      pi_copp->key, pd_copp->hw_policer_id, ret);
        return ret;
    }

    sdk_ret = sdk::asic::asic_mem_read(stats_addr, (uint8_t *)&stats_0,
                                       sizeof(stats_0));
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Error reading stats for copp {} hw-id {}, ret {}",
                      pi_copp->key, pd_copp->hw_policer_id, ret);
        return ret;
    }

    // read the d-vector
    sdk_ret = copp_action_tbl->retrieve(pd_copp->hw_policer_id, &d);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading copp action entry copp {} hw-id {}, ret {}",
                      pi_copp->key, pd_copp->hw_policer_id, ret);
        return ret;
    }

    sdk_ret = sdk::asic::asic_mem_read(stats_addr, (uint8_t *)&stats_1,
                                       sizeof(stats_1));
    if (sdk_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Error reading stats for copp {} hw-id {}, ret {}",
                      pi_copp->key, pd_copp->hw_policer_id, ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Copp stat read copp {} hw_id {} stats_addr {:#x} "
                    "stats_0.permitted_packets {} "
                    "stats_0.permitted_bytes {} "
                    "stats_0.denied_packets {} "
                    "stats_0.denied_bytes {} "
                    "stats_1.permitted_packets {} "
                    "stats_1.permitted_bytes {} "
                    "stats_1.denied_packets {} "
                    "stats_1.denied_bytes {} "
                    "d.permitted_packets {} "
                    "d.permitted_bytes {} "
                    "d.denied_packets {} "
                    "d.denied_bytes {} ",
                    pi_copp->key, pd_copp->hw_policer_id,
                    stats_addr,
                    stats_0.permitted_packets ,
                    stats_0.permitted_bytes ,
                    stats_0.denied_packets ,
                    stats_0.denied_bytes ,
                    stats_1.permitted_packets ,
                    stats_1.permitted_bytes ,
                    stats_1.denied_packets ,
                    stats_1.denied_bytes ,
                    COPP_STATS(d,permitted_packets),
                    COPP_STATS(d,permitted_bytes),
                    COPP_STATS(d,denied_packets),
                    COPP_STATS(d,denied_bytes));

    if (stats_1.permitted_packets == stats_0.permitted_packets) {
        stats_1.permitted_packets += COPP_STATS(d, permitted_packets);
        stats_1.permitted_bytes += COPP_STATS(d, permitted_bytes);
    }
    if (stats_1.denied_packets == stats_0.denied_packets) {
        stats_1.denied_packets += COPP_STATS(d, denied_packets);
        stats_1.denied_bytes += COPP_STATS(d, denied_bytes);
    }

    stats_rsp->set_permitted_packets(stats_1.permitted_packets);
    stats_rsp->set_permitted_bytes(stats_1.permitted_bytes);
    stats_rsp->set_dropped_packets(stats_1.denied_packets);
    stats_rsp->set_dropped_bytes(stats_1.denied_bytes);
    return HAL_RET_OK;
}
#undef COPP_STATS

// ----------------------------------------------------------------------------
// pd copp get
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd_copp_get_args_t *args = pd_func_args->pd_copp_get;
    copp_t             *copp = args->copp;
    pd_copp_t          *pd_copp = (pd_copp_t *)copp->pd;
    CoppGetResponse    *rsp = args->rsp;

    auto copp_info = rsp->mutable_status()->mutable_epd_status();
    copp_info->set_hw_policer_idx(pd_copp->hw_policer_id);
    ret = copp_pd_populate_policer_stats(
                        rsp->mutable_stats()->mutable_policer_stats(), pd_copp);

    return ret;
}

// ----------------------------------------------------------------------------
// pd copp restore from response
// ----------------------------------------------------------------------------
static hal_ret_t
copp_pd_restore_data (pd_copp_restore_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    copp_t    *copp = args->copp;
    pd_copp_t *pd_copp= (pd_copp_t *)copp->pd;

    auto copp_info = args->copp_status->epd_status();
    pd_copp->hw_policer_id = copp_info.hw_policer_idx();

    return ret;
}

//-----------------------------------------------------------------------------
// pd copp restore
//-----------------------------------------------------------------------------
hal_ret_t
pd_copp_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret;
    pd_copp_restore_args_t *args = pd_func_args->pd_copp_restore;
    pd_copp_t *copp_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Restoring pd state for copp {}", args->copp->key);

    // allocate PD copp state
    copp_pd = copp_pd_alloc_init();
    if (copp_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    copp_pd_link_pi_pd(copp_pd, args->copp);

    ret = copp_pd_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore pd data for copp: {}, err: {}",
                      args->copp->key, ret);
        goto end;
    }

    // This call will just populate table libs and calls to HW will be
    // a NOOP in p4pd code
    ret = copp_pd_program_hw(copp_pd, false, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to program hw", __FUNCTION__);
        goto end;
    }

end:

    if (ret != HAL_RET_OK) {
        copp_pd_cleanup(copp_pd);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_copp_mem_free (pd_func_args_t *pd_func_args)
{
    pd_copp_t        *pd_copp;
    pd_copp_mem_free_args_t *args = pd_func_args->pd_copp_mem_free;

    pd_copp = (pd_copp_t *)args->copp->pd;
    copp_pd_mem_free(pd_copp);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
