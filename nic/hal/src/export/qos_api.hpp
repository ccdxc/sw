/* 
 * ----------------------------------------------------------------------------
 *
 * qos_api.hpp
 *
 * QOS APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __QOS_API_HPP__
#define __QOS_API_HPP__


namespace hal {

using hal::buf_pool_t;

// Buf-Pool APIs
uint32_t buf_pool_get_port_num(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_buf_pool_id(buf_pool_t *pi_buf_pool);
void buf_pool_set_pd_buf_pool(buf_pool_t *pi_buf_pool, void *pd_buf_pool);
uint32_t buf_pool_get_reserved_bytes(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_headroom_bytes(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_sharing_factor(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_xon_threshold(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_xoff_clear_limit(buf_pool_t *pi_buf_pool);
uint32_t buf_pool_get_mtu(buf_pool_t *pi_buf_pool);
void buf_pool_get_cos_mapping(buf_pool_t *pi_buf_pool, uint32_t *cos_map, 
                              uint32_t nelems, uint32_t *nelems_filled_p);

using hal::queue_t;

// Queue APIs
uint32_t queue_get_port_num(queue_t *pi_queue);
uint32_t queue_get_queue_id(queue_t *pi_queue);
hal_handle_t queue_get_queue_handle(queue_t *pi_queue);
void queue_set_pd_queue(queue_t *pi_queue, void *pd_queue);
void* queue_get_pd_queue(queue_t *pi_queue);
queue_node_type_e queue_get_node_type(queue_t *pi_queue);
sched_type_e queue_get_sched_type(queue_t *pi_queue);
sched_config_t* queue_get_sched_config(queue_t *pi_queue);


using hal::policer_t;

// Policer APIs
uint32_t policer_get_policer_id(policer_t *pi_policer);
void policer_set_pd_policer(policer_t *pi_policer, void *pd_policer);
policer_direction_e policer_get_direction(policer_t *pi_policer);
bool policer_get_marking_action(policer_t *pi_policer, qos_marking_action_t *m_action);

} // namespace hal
#endif // __QOS_API_HPP__
