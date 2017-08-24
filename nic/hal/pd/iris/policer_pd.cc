#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <policer_pd.hpp>
#include <pd_api.hpp>
#include <qos_api.hpp>

namespace hal {
namespace pd {

hal_ret_t
pd_policer_create (pd_policer_args_t *args) 
{
    hal_ret_t     ret;
    pd_policer_t *pd_policer;

    HAL_TRACE_DEBUG("PD-POLICER::{}: Creating pd state for policer: {}", 
            __func__, policer_get_policer_id(args->policer));

    // Create policer PD
    pd_policer = policer_pd_alloc_init();
    if (pd_policer == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_policer, args->policer);

    // Allocate Resources
    ret = policer_pd_alloc_res(pd_policer);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to alloc. resources for policer: {}",
                __func__, policer_get_policer_id(args->policer));
        goto end;
    }

    // Program HW
    ret = policer_pd_program_hw(pd_policer);

end:
    if (ret != HAL_RET_OK) {
        policer_pd_dealloc_res(pd_policer);
        unlink_pi_pd(pd_policer, args->policer);
        policer_pd_free(pd_policer);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Policer PD Instance
// ----------------------------------------------------------------------------
inline pd_policer_t *
policer_pd_alloc_init (void)
{
    return policer_pd_init(policer_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Policer Instance
// ----------------------------------------------------------------------------
inline pd_policer_t *
policer_pd_alloc (void)
{
    pd_policer_t    *policer;

    policer = (pd_policer_t *)g_hal_state_pd->policer_pd_slab()->alloc();
    if (policer == NULL) {
        return NULL;
    }
    return policer;
}

// ----------------------------------------------------------------------------
// Initialize Policer PD instance
// ----------------------------------------------------------------------------
inline pd_policer_t *
policer_pd_init (pd_policer_t *policer)
{
    // Nothing to do currently
    if (!policer) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    policer->hw_policer_id = HAL_INVALID_HW_POLICER_ID;

    return policer;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Policer
// ----------------------------------------------------------------------------
hal_ret_t 
policer_pd_alloc_res(pd_policer_t *pd_policer)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;
    // Allocate policer hwid
    if (policer_get_direction((policer_t*)pd_policer->pi_policer) == INGRESS_QOS) {
        rs = g_hal_state_pd->ingress_policer_hwid_idxr()->alloc((uint32_t *)&pd_policer->hw_policer_id);
    } else {
        rs = g_hal_state_pd->egress_policer_hwid_idxr()->alloc((uint32_t *)&pd_policer->hw_policer_id);
    }
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Deallocate resources for Policer 
// ----------------------------------------------------------------------------
void
policer_pd_dealloc_res (pd_policer_t *pd_policer)
{
    policer_t *policer = (policer_t*)pd_policer->pi_policer;

    if (!policer) {
        return;
    }

    // Deallocate policer hwid
    if (pd_policer->hw_policer_id == HAL_INVALID_HW_POLICER_ID) {
        return;
    }
    if (policer_get_direction(policer) == INGRESS_QOS) {
        g_hal_state_pd->ingress_policer_hwid_idxr()->free(
            (uint32_t)pd_policer->hw_policer_id);
    } else {
        g_hal_state_pd->egress_policer_hwid_idxr()->free(
            (uint32_t)pd_policer->hw_policer_id);
    }
}

static hal_ret_t
policer_pd_pgm_ingress_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t                  ret = HAL_RET_OK;
    DirectMap                  *policer_tbl = NULL;
    ingress_policer_actiondata data;

    memset(&data, 0, sizeof(data));

    data.actionid = INGRESS_POLICER_EXECUTE_INGRESS_POLICER_ID;

    policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_INGRESS_POLICER);
    HAL_ASSERT_RETURN((policer_tbl != NULL), HAL_RET_ERR);

    ret = policer_tbl->insert_withid(&data, pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to program for ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed for ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

static hal_ret_t
policer_pd_pgm_egress_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t                 ret = HAL_RET_OK;
    DirectMap                 *policer_tbl = NULL;
    egress_policer_actiondata data;

    memset(&data, 0, sizeof(data));

    data.actionid = EGRESS_POLICER_EXECUTE_EGRESS_POLICER_ID;

    policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_EGRESS_POLICER);
    HAL_ASSERT_RETURN((policer_tbl != NULL), HAL_RET_ERR);

    ret = policer_tbl->insert_withid(&data, pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to program for egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed for egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

#define policer_action data.ingress_policer_action_action_u.ingress_policer_action_ingress_policer_action
static hal_ret_t
policer_pd_pgm_ingress_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t                         ret = HAL_RET_OK;
    DirectMap                         *policer_action_tbl = NULL;
    ingress_policer_action_actiondata data;
    qos_marking_action_t              marking_action;

    memset(&data, 0, sizeof(data));

    policer_get_marking_action((policer_t*)pd_policer->pi_policer, 
                               &marking_action);

    data.actionid = INGRESS_POLICER_ACTION_INGRESS_POLICER_ACTION_ID;


    policer_action.cos_en = qos_marking_action_pcp_rewrite_en(&marking_action);
    policer_action.cos = qos_marking_action_pcp(&marking_action);
    policer_action.dscp_en = qos_marking_action_dscp_rewrite_en(&marking_action);
    policer_action.dscp = qos_marking_action_dscp(&marking_action);

    policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_INGRESS_POLICER_ACTION);
    HAL_ASSERT_RETURN((policer_action_tbl != NULL), HAL_RET_ERR);

    ret = policer_action_tbl->insert_withid(&data, pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to program for ingress policer action: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed for ingress policer action: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}
#undef policer_action

#define policer_action data.egress_policer_action_action_u.egress_policer_action_egress_policer_action
static hal_ret_t
policer_pd_pgm_egress_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t                        ret = HAL_RET_OK;
    DirectMap                        *policer_action_tbl = NULL;
    egress_policer_action_actiondata data;

    memset(&data, 0, sizeof(data));

    data.actionid = EGRESS_POLICER_ACTION_EGRESS_POLICER_ACTION_ID;

    policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_EGRESS_POLICER_ACTION);
    HAL_ASSERT_RETURN((policer_action_tbl != NULL), HAL_RET_ERR);

    ret = policer_action_tbl->insert_withid(&data, pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to program for egress policer action: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed for egress policer action: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}
#undef policer_action

static hal_ret_t
policer_pd_pgm_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t ret = HAL_RET_OK;

    if (policer_get_direction((policer_t*)pd_policer->pi_policer) == INGRESS_QOS) {
        ret = policer_pd_pgm_ingress_policer_tbl(pd_policer);
    } else {
        ret = policer_pd_pgm_egress_policer_tbl(pd_policer);
    }
    return ret;
}

static hal_ret_t
policer_pd_pgm_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t ret = HAL_RET_OK;

    if (policer_get_direction((policer_t*)pd_policer->pi_policer) == INGRESS_QOS) {
        ret = policer_pd_pgm_ingress_policer_action_tbl(pd_policer);
    } else {
        ret = policer_pd_pgm_egress_policer_action_tbl(pd_policer);
    }
    return ret;
}

static hal_ret_t 
policer_pd_cleanup_ingress_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t   ret = HAL_RET_OK;
    DirectMap   *policer_tbl = NULL;
    
    policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_INGRESS_POLICER);
    HAL_ASSERT_RETURN((policer_tbl != NULL), HAL_RET_ERR);

    ret = policer_tbl->remove(pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to cleanup for ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed cleanup ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

static hal_ret_t 
policer_pd_cleanup_egress_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t   ret = HAL_RET_OK;
    DirectMap   *policer_tbl = NULL;
    
    policer_tbl = g_hal_state_pd->dm_table(P4TBL_ID_EGRESS_POLICER);
    HAL_ASSERT_RETURN((policer_tbl != NULL), HAL_RET_ERR);

    ret = policer_tbl->remove(pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to cleanup for egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed cleanup egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

static hal_ret_t 
policer_pd_cleanup_ingress_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t   ret = HAL_RET_OK;
    DirectMap   *policer_action_tbl = NULL;
    
    policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_INGRESS_POLICER_ACTION);
    HAL_ASSERT_RETURN((policer_action_tbl != NULL), HAL_RET_ERR);

    ret = policer_action_tbl->remove(pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to cleanup for ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed cleanup ingress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

static hal_ret_t 
policer_pd_cleanup_egress_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t   ret = HAL_RET_OK;
    DirectMap   *policer_action_tbl = NULL;
    
    policer_action_tbl = g_hal_state_pd->dm_table(P4TBL_ID_EGRESS_POLICER_ACTION);
    HAL_ASSERT_RETURN((policer_action_tbl != NULL), HAL_RET_ERR);

    ret = policer_action_tbl->remove(pd_policer->hw_policer_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-POLICER::{}: Unable to cleanup for egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    } else {
        HAL_TRACE_DEBUG("PD-POLICER::{}: Programmed cleanup egress policer: {}",
                __FUNCTION__, policer_get_policer_id((policer_t *)pd_policer->pi_policer));
    }

    return ret;
}

static hal_ret_t 
policer_pd_cleanup_policer_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t ret = HAL_RET_OK;

    if (policer_get_direction((policer_t*)pd_policer->pi_policer) == INGRESS_QOS) {
        ret = policer_pd_cleanup_ingress_policer_tbl(pd_policer);
    } else {
        ret = policer_pd_cleanup_egress_policer_tbl(pd_policer);
    }

    return ret;
}

static hal_ret_t 
policer_pd_cleanup_policer_action_tbl (pd_policer_t *pd_policer)
{
    hal_ret_t ret = HAL_RET_OK;

    if (policer_get_direction((policer_t*)pd_policer->pi_policer) == INGRESS_QOS) {
        ret = policer_pd_cleanup_ingress_policer_action_tbl(pd_policer);
    } else {
        ret = policer_pd_cleanup_egress_policer_action_tbl(pd_policer);
    }

    return ret;
}

static void
policer_pd_unprogram_hw (pd_policer_t *pd_policer)
{
    policer_pd_cleanup_policer_tbl(pd_policer);
    policer_pd_cleanup_policer_action_tbl(pd_policer);
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
policer_pd_program_hw(pd_policer_t *pd_policer)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = policer_pd_pgm_policer_tbl(pd_policer);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = policer_pd_pgm_policer_action_tbl(pd_policer);
    if (ret != HAL_RET_OK) {
        policer_pd_unprogram_hw(pd_policer);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing Policer PD
// ----------------------------------------------------------------------------
hal_ret_t
policer_pd_free (pd_policer_t *policer)
{
    g_hal_state_pd->policer_pd_slab()->free(policer);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_policer_t *pd_policer, policer_t *pi_policer)
{
    pd_policer->pi_policer = pi_policer;
    policer_set_pd_policer(pi_policer, pd_policer);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_policer_t *pd_policer, policer_t *pi_policer)
{
    pd_policer->pi_policer = NULL;
    policer_set_pd_policer(pi_policer, NULL);
}
}    // namespace pd
}    // namespace hal
