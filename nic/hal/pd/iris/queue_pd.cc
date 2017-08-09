#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <queue_pd.hpp>
#include <pd_api.hpp>
#include <qos_api.hpp>

namespace hal {
namespace pd {

static hal_ret_t
pd_queue_create_one (queue_t *queue_node)
{
    hal_ret_t   ret;
    pd_queue_t  *pd_queue;

    HAL_TRACE_DEBUG("PD-Queue::{}: Creating pd state for queue (handle): {}", 
            __func__, queue_get_queue_handle(queue_node));

    // Create queue PD
    pd_queue = queue_pd_alloc_init();
    if (pd_queue == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_queue, queue_node);

    // Allocate Resources
    ret = queue_pd_alloc_res(pd_queue);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-Queue::{}: Unable to alloc. resources for queue (handle): {}",
                __func__, queue_get_queue_handle(queue_node));
        goto end;
    }
end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_queue, queue_node);
        queue_pd_free(pd_queue);
    }
    return ret;
}

static void
queue_create_cleanup_ (queue_t *queue)
{
    pd_queue_t *pd_queue;
    pd_queue = (pd_queue_t *)queue_get_pd_queue(queue);

    if (!pd_queue) {
        return;
    }

    queue_pd_dealloc_res(pd_queue);
    unlink_pi_pd(pd_queue, queue);
    queue_pd_free(pd_queue);
}

static void
queue_create_cleanup (pd_queue_args_t *args)
{
    uint32_t i;

    for(i = 0; i < args->cnt_l0; i++) {
        queue_create_cleanup_(args->l0_nodes[i]);
    }
    for(i = 0; i < args->cnt_l1; i++) {
        queue_create_cleanup_(args->l1_nodes[i]);
    }
    for(i = 0; i < args->cnt_l2; i++) {
        queue_create_cleanup_(args->l2_nodes[i]);
    }
}

hal_ret_t
pd_queue_create (pd_queue_args_t *args) 
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t i;
    queue_t *l0_node;
    queue_t *l1_node;
    queue_t *l2_node;

    // This assumes that the l0_nodes, l1_nodes and l2_nodes are 
    // sorted according to their priority. If it changes in the future, 
    // need to revisit
    for(i = 0; i < args->cnt_l0; i++) {
        l0_node = args->l0_nodes[i];
        ret = pd_queue_create_one(l0_node);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-Queue::{}: Unable to alloc. resources for queue (handle): {}",
                          __func__, queue_get_queue_handle(l0_node));
            goto end;
        }
    }

    for(i = 0; i < args->cnt_l1; i++) {
        l1_node = args->l1_nodes[i];
        ret = pd_queue_create_one(l1_node);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-Queue::{}: Unable to alloc. resources for l1 node (handle): {}",
                          __func__, queue_get_queue_handle(l1_node));
            goto end;
        }
    }

    for(i = 0; i < args->cnt_l2; i++) {
        l2_node = args->l2_nodes[i];
        ret = pd_queue_create_one(l2_node);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-Queue::{}: Unable to alloc. resources for l2 node (handle): {}",
                          __func__, queue_get_queue_handle(l2_node));
            goto end;
        }
    }

    // Program HW
    ret = queue_pd_program_hw(args);

end:
    if (ret != HAL_RET_OK) {
        queue_create_cleanup(args);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Queue PD Instance
// ----------------------------------------------------------------------------
inline pd_queue_t *
queue_pd_alloc_init (void)
{
    return queue_pd_init(queue_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Queue Instance
// ----------------------------------------------------------------------------
inline pd_queue_t *
queue_pd_alloc (void)
{
    pd_queue_t    *queue;

    queue = (pd_queue_t *)g_hal_state_pd->queue_pd_slab()->alloc();
    if (queue == NULL) {
        return NULL;
    }
    return queue;
}

// ----------------------------------------------------------------------------
// Initialize Queue PD instance
// ----------------------------------------------------------------------------
inline pd_queue_t *
queue_pd_init (pd_queue_t *queue)
{
    // Nothing to do currently
    if (!queue) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    queue->hw_queue_id = HAL_INVALID_HW_QUEUE_ID;

    return queue;
}

// ----------------------------------------------------------------------------
// Allocate resources for Queue 
// ----------------------------------------------------------------------------
hal_ret_t 
queue_pd_alloc_res (pd_queue_t *pd_queue)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;
    queue_t              *queue = (queue_t*)pd_queue->pi_queue;
    uint32_t             port_num;
    queue_node_type_e    node_type;

    port_num = queue_get_port_num(queue);
    node_type = queue_get_node_type(queue);

    // Allocate queue hwid
    rs = g_hal_state_pd->queue_hwid_idxr(port_num, node_type)->alloc(
                                        (uint32_t *)&pd_queue->hw_queue_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Deallocate resources for Queue 
// ----------------------------------------------------------------------------
void
queue_pd_dealloc_res (pd_queue_t *pd_queue)
{
    queue_t              *queue = (queue_t*)pd_queue->pi_queue;
    uint32_t             port_num;
    queue_node_type_e    node_type;

    if (!queue) {
        return;
    }

    port_num = queue_get_port_num(queue);
    node_type = queue_get_node_type(queue);

    // Deallocate queue hwid
    if (pd_queue->hw_queue_id != HAL_INVALID_HW_QUEUE_ID) {
        g_hal_state_pd->queue_hwid_idxr(port_num, node_type)->free(
                                            (uint32_t)pd_queue->hw_queue_id);
    }
}

static void
scheduler_param_fill_helper (queue_t *queue, 
                             tm_queue_node_t *tm_q_nodes)
{
    pd_queue_t *pd_queue = (pd_queue_t *)queue_get_pd_queue(queue);
    tm_queue_node_t *sch_node;
    sched_type_e sched_type;
    sched_config_t *sched_cfg;

    sch_node = &tm_q_nodes[pd_queue->hw_queue_id];

    sch_node->in_use = true;
    sch_node->parent_node = 
        ((pd_queue_t *)queue_get_pd_queue(find_queue_by_handle(queue->parent_handle)))->hw_queue_id;

    sched_type = queue_get_sched_type(queue);
    sched_cfg = queue_get_sched_config(queue);
    if (sched_type == SCHED_TYPE_STRICT) {
        sch_node->sched_type = TM_SCHED_TYPE_STRICT;
        sch_node->u.strict.rate = sched_cfg->strict.rate;
    } else if (sched_type == SCHED_TYPE_DWRR) {
        sch_node->sched_type = TM_SCHED_TYPE_DWRR;
        sch_node->u.dwrr.weight = sched_cfg->dwrr.weight;
    }
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
queue_pd_program_hw (pd_queue_args_t *args)
{
    hal_ret_t          ret = HAL_RET_OK;
    tm_scheduler_map_t sch_map = {0};
    uint32_t           i;
    queue_t            *l0_node;
    queue_t            *l1_node;
    uint32_t           tm_port;

    for(i = 0; i < args->cnt_l0; i++) {
        l0_node = args->l0_nodes[i];
        tm_port = queue_get_port_num(l0_node);

        scheduler_param_fill_helper(l0_node, sch_map.l0_nodes);
    }

    for(i = 0; i < args->cnt_l1; i++) {
        l1_node = args->l1_nodes[i];
        scheduler_param_fill_helper(l1_node, sch_map.l1_nodes);
    }

    ret = capri_tm_scheduler_map_update(tm_port, &sch_map);

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing Queue PD
// ----------------------------------------------------------------------------
hal_ret_t
queue_pd_free (pd_queue_t *queue)
{
    g_hal_state_pd->queue_pd_slab()->free(queue);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd (pd_queue_t *pd_queue, queue_t *pi_queue)
{
    pd_queue->pi_queue = pi_queue;
    queue_set_pd_queue(pi_queue, pd_queue);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd (pd_queue_t *pd_queue, queue_t *pi_queue)
{
    pd_queue->pi_queue = NULL;
    queue_set_pd_queue(pi_queue, NULL);
}
}    // namespace pd
}    // namespace hal
