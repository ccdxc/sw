#include <qos.hpp>
#include <pd.hpp>
#include <qos_api.hpp>

namespace hal {

// ----------------------------------------------------------------------------
// Buf-Pool API: Get Buf-Pool's Port number
// ----------------------------------------------------------------------------
uint32_t 
buf_pool_get_port_num(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->port_num;
}

// ----------------------------------------------------------------------------
// Buf-Pool API: Get Buf-Pool's ID
// ----------------------------------------------------------------------------
uint32_t
buf_pool_get_buf_pool_id(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->buf_pool_id;
}

// ----------------------------------------------------------------------------
// Buf-Pool API: Set PD Buf-Pool in PI Buf-Pool
// ----------------------------------------------------------------------------
void 
buf_pool_set_pd_buf_pool(buf_pool_t *pi_buf_pool, void *pd_buf_pool)
{
    pi_buf_pool->pd_buf_pool = pd_buf_pool;
}

uint32_t 
buf_pool_get_reserved_bytes(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.reserved_bytes;
}

uint32_t 
buf_pool_get_headroom_bytes(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.headroom_bytes;
}

uint32_t 
buf_pool_get_sharing_factor(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.sharing_factor;
}


uint32_t 
buf_pool_get_xon_threshold(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.xon_threshold;
}

uint32_t 
buf_pool_get_xoff_clear_limit(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.xoff_clear_limit;
}

uint32_t 
buf_pool_get_mtu(buf_pool_t *pi_buf_pool)
{
    return pi_buf_pool->spec.mtu;
}

void 
buf_pool_get_cos_mapping(buf_pool_t *pi_buf_pool, uint32_t *cos_map, 
                         uint32_t nelems, uint32_t *nelems_filled_p)
{
    uint32_t cos;
    hal_ret_t ret;
    uint32_t nelems_filled = 0;

    ret = pi_buf_pool->spec.cos_bmp->first_set(&cos);
    while ((ret == HAL_RET_OK) && (nelems_filled < nelems)) {
        cos_map[nelems_filled++] = cos;
        ret = pi_buf_pool->spec.cos_bmp->next_set(cos, &cos);
    }
    *nelems_filled_p = nelems_filled;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue's Port number
// ----------------------------------------------------------------------------
uint32_t 
queue_get_port_num(queue_t *pi_queue)
{
    return pi_queue->port_num;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue's ID
// ----------------------------------------------------------------------------
uint32_t
queue_get_queue_id(queue_t *pi_queue)
{
    return pi_queue->queue_id;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue's hal handle
// ----------------------------------------------------------------------------
hal_handle_t 
queue_get_queue_handle(queue_t *pi_queue)
{
    return pi_queue->hal_handle;
}

// ----------------------------------------------------------------------------
// Queue API: Set PD Queue in PI Queue
// ----------------------------------------------------------------------------
void 
queue_set_pd_queue(queue_t *pi_queue, void *pd_queue)
{
    pi_queue->pd_queue = pd_queue;
}

// ----------------------------------------------------------------------------
// Queue API: Get PD Queue in PI Queue
// ----------------------------------------------------------------------------
void* queue_get_pd_queue(queue_t *pi_queue)
{
    return pi_queue->pd_queue;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue node type
// ----------------------------------------------------------------------------
queue_node_type_e 
queue_get_node_type(queue_t *pi_queue) 
{
    return pi_queue->node_type;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue scheduler type
// ----------------------------------------------------------------------------
sched_type_e
queue_get_sched_type(queue_t *pi_queue) 
{
    return pi_queue->sched_type;
}

// ----------------------------------------------------------------------------
// Queue API: Get Queue scheduler config
// ----------------------------------------------------------------------------
sched_config_t *
queue_get_sched_config(queue_t *pi_queue) 
{
    return &pi_queue->u;
}

// ----------------------------------------------------------------------------
// Policer API: Get Policer's ID
// ----------------------------------------------------------------------------
uint32_t
policer_get_policer_id(policer_t *pi_policer)
{
    return pi_policer->policer_id;
}

// ----------------------------------------------------------------------------
// Policer API: Set PD Policer in PI Policer
// ----------------------------------------------------------------------------
void 
policer_set_pd_policer(policer_t *pi_policer, void *pd_policer)
{
    pi_policer->pd_policer = pd_policer;
}

// ----------------------------------------------------------------------------
// Policer API: Get the direction of Policer in PI Policer
// ----------------------------------------------------------------------------
policer_direction_e 
policer_get_direction (policer_t *pi_policer) 
{
    return pi_policer->spec.direction;
}

// ----------------------------------------------------------------------------
// Policer API: Get the marking action of Policer in PI Policer
// ----------------------------------------------------------------------------
bool
policer_get_marking_action(policer_t *pi_policer, qos_marking_action_t *m_action) 
{
    qos_copy_marking_action(m_action, &pi_policer->spec.marking_action);
    return pi_policer->spec.qos_marking_en;
}
} // namespace hal
