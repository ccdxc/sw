#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
// #include "nic/hal/svc/qos_svc.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/proto/hal/qos.pb.h"

namespace hal {

void *
buf_pool_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((buf_pool_t *)entry)->buf_pool_id);
}

uint32_t
buf_pool_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(buf_pool_id_t)) % ht_size;
}

bool
buf_pool_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(buf_pool_id_t *)key1 == *(buf_pool_id_t *)key2) {
        return true;
    }
    return false;
}

void *
buf_pool_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((buf_pool_t *)entry)->hal_handle);
}

uint32_t
buf_pool_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
buf_pool_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert this buf_pool in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_buf_pool_to_db (buf_pool_t *buf_pool)
{
    uint32_t cos;
    hal_ret_t ret;

    g_hal_state->buf_pool_id_ht()->insert(buf_pool, &buf_pool->ht_ctxt);
    g_hal_state->buf_pool_hal_handle_ht()->insert(buf_pool, &buf_pool->hal_handle_ht_ctxt);

    // Add all the cos values part of this buf pool to the global cos map bmp
    ret = buf_pool->spec.cos_bmp->first_set(&cos);
    while (ret == HAL_RET_OK) {
        g_hal_state->buf_pool_cos_usage_bmp(buf_pool->port_num)->set(cos);
        ret = buf_pool->spec.cos_bmp->next_set(cos, &cos);
    }

    return HAL_RET_OK;
}

static hal_ret_t 
validate_buf_pool_create (BufPoolSpec& spec,
                          BufPoolResponse *rsp) 
{
    int c;

    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_BUF_POOL_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().key_or_handle_case() != 
        qos::BufPoolKeyHandle::kBufPoolId) {
        rsp->set_api_status(types::API_STATUS_BUF_POOL_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    /* Check that all the required parameters are present */
    if (spec.tcs_size() == 0) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    for (c = 0; c < spec.tcs_size(); c++) {
        if (spec.tcs(c).cos() >= HAL_MAX_COSES) {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    return HAL_RET_OK;
}

static bool
is_cos_mapped_to_buf_pool (uint32_t port_num, uint32_t cos) 
{
    return g_hal_state->buf_pool_cos_usage_bmp(port_num)->is_set(cos);
}

hal_ret_t
buf_pool_create (BufPoolSpec& spec,
                 BufPoolResponse *rsp)
{
    hal_ret_t              ret;
    buf_pool_t             *buf_pool = NULL;
    pd::pd_buf_pool_args_t pd_buf_pool_args;
    uint32_t               cos;
    int                    c;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-BUF-POOL:{}: BufPool create ", __func__);

    ret = validate_buf_pool_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-BUF-POOL:{}: BufPool create request validation failed."
                     " Err: {}",
                      __func__, HAL_RET_INVALID_ARG);
        return ret;
    }

    HAL_TRACE_DEBUG("PI-BUF-POOL:{}: BufPool create for id {}",
                    __func__, spec.key_or_handle().buf_pool_id());

    // Do some runtime validation
    // Check that the COS values are not already part of some buf-pool
    for (c = 0; c < spec.tcs_size(); c++) {
        cos = spec.tcs(c).cos();
        if (is_cos_mapped_to_buf_pool(spec.port_num(), cos)) {
            HAL_TRACE_ERR("PI-BUF-POOL:{}: BufPool create for id {} failed.."
                          "Cos {} already mapped to a buffer pool",
                          __func__, spec.key_or_handle().buf_pool_id(), 
                          cos);
            rsp->set_api_status(types::API_STATUS_BUF_POOL_COS_MAP_EXISTS);
            return HAL_RET_INVALID_ARG;
        }
    }

    // allocate a buf-pool instance
    buf_pool = buf_pool_alloc_init();
    if (buf_pool == NULL) {
        HAL_TRACE_ERR("PI-BUF-POOL:{}: Out of Memory. Err: {}",
                      __func__, HAL_RET_OOM);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // save the configs from the spec
    buf_pool->buf_pool_id = spec.key_or_handle().buf_pool_id();
    buf_pool->port_num = spec.port_num();
    buf_pool->hal_handle = hal_alloc_handle();
    
    buf_pool->spec.reserved_bytes = spec.reserved_bytes();
    buf_pool->spec.headroom_bytes = spec.headroom_bytes();
    buf_pool->spec.sharing_factor = spec.sharing_factor();
    buf_pool->spec.xon_threshold = spec.xon_threshold();
    buf_pool->spec.xoff_clear_limit = spec.xoff_clear_limit();
    buf_pool->spec.mtu = spec.mtu();

    for (c = 0; c < spec.tcs_size(); c++) {
        cos = spec.tcs(c).cos();
        buf_pool->spec.cos_bmp->set(cos);
    }

    // allocate PD resources and program hardware
    pd::pd_buf_pool_args_init(&pd_buf_pool_args);
    pd_buf_pool_args.buf_pool = buf_pool;
    ret = pd::pd_buf_pool_create(&pd_buf_pool_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD buf-pool create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this buf-pool to the db
    add_buf_pool_to_db(buf_pool);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->mutable_buf_pool_handle()->set_handle(buf_pool->hal_handle);

end:

    if (ret != HAL_RET_OK && buf_pool != NULL) {
        buf_pool_free(buf_pool);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
buf_pool_update (BufPoolSpec& spec,
                 BufPoolResponse *rsp)
{
    return HAL_RET_OK;
}



void *
queue_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((queue_t *)entry)->queue_id);
}

uint32_t
queue_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(queue_id_t)) % ht_size;
}

bool
queue_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(queue_id_t *)key1 == *(queue_id_t *)key2) {
        return true;
    }
    return false;
}

void *
queue_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((queue_t *)entry)->hal_handle);
}

uint32_t
queue_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
queue_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// remove the queues in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
remove_queues_from_db (queue_t **l0_nodes, uint32_t cnt_l0,
                       queue_t **l1_nodes, uint32_t cnt_l1,
                       queue_t **l2_nodes, uint32_t cnt_l2)
{
    uint32_t i;
    queue_t *queue;

    for (i = 0; l0_nodes && (i < cnt_l0); i++) {
        queue = l0_nodes[i];
        g_hal_state->queue_id_ht()->remove(&queue->queue_id);
        g_hal_state->queue_hal_handle_ht()->remove(&queue->hal_handle);
    }

    for (i = 0; l1_nodes && (i < cnt_l1); i++) {
        queue = l1_nodes[i];
        g_hal_state->queue_id_ht()->remove(&queue->queue_id);
        g_hal_state->queue_hal_handle_ht()->remove(&queue->hal_handle);
    }

    for (i = 0; l2_nodes && (i < cnt_l2); i++) {
        queue = l2_nodes[i];
        g_hal_state->queue_id_ht()->remove(&queue->queue_id);
        g_hal_state->queue_hal_handle_ht()->remove(&queue->hal_handle);
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert the queues in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_queues_to_db (queue_t **l0_nodes, uint32_t cnt_l0,
                  queue_t **l1_nodes, uint32_t cnt_l1,
                  queue_t **l2_nodes, uint32_t cnt_l2)
{
    uint32_t i;
    queue_t *queue;

    for (i = 0; i < cnt_l0; i++) {
        queue = l0_nodes[i];
        g_hal_state->queue_id_ht()->insert(queue, &queue->ht_ctxt);
        g_hal_state->queue_hal_handle_ht()->insert(queue, &queue->hal_handle_ht_ctxt);
    }

    for (i = 0; i < cnt_l1; i++) {
        queue = l1_nodes[i];
        g_hal_state->queue_id_ht()->insert(queue, &queue->ht_ctxt);
        g_hal_state->queue_hal_handle_ht()->insert(queue, &queue->hal_handle_ht_ctxt);
    }

    for (i = 0; i < cnt_l2; i++) {
        queue = l2_nodes[i];
        g_hal_state->queue_id_ht()->insert(queue, &queue->ht_ctxt);
        g_hal_state->queue_hal_handle_ht()->insert(queue, &queue->hal_handle_ht_ctxt);
    }
    return HAL_RET_OK;
}

static bool 
queue_node_cmp_fn (queue_t *q1, queue_t *q2)
{
    return q1->priority < q2->priority;
}

static hal_ret_t 
validate_queue_create (QueueSpec& spec,
                       QueueResponse *rsp) 
{
    qos::QueueInfo *queue_info;
    int q;

    for (q = 0; q < spec.queues_size(); q++) {
        queue_info = spec.mutable_queues(q); 
        if (!queue_info->has_key_or_handle()) {
            rsp->set_api_status(types::API_STATUS_BUF_POOL_ID_INVALID);
            return HAL_RET_INVALID_ARG;
        }

        if (queue_info->key_or_handle().key_or_handle_case() != 
            qos::QueueKeyHandle::kQueueId) {
            rsp->set_api_status(types::API_STATUS_BUF_POOL_ID_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    }

    return HAL_RET_OK;
}

static queue_t*
queue_node_alloc (queue_node_type_e node_type, queue_id_t queue_id,
                  uint32_t port_num, uint32_t priority, sched_type_e sched_type)
{
    queue_t *node = NULL;

    node = queue_alloc_init();
    if (node == NULL) {
        return NULL;
    }
    node->node_type = node_type;
    node->queue_id = queue_id;
    node->port_num = port_num;
    node->hal_handle = hal_alloc_handle();
    node->priority = priority;
    node->sched_type = sched_type;
    return node;
}

static inline sched_type_e
get_sched_type (const qos::QueueSchedulerNode& sched_node)
{
    if (sched_node.has_dwrr()) {
        return SCHED_TYPE_DWRR;
    } else if (sched_node.has_strict()) {
        return SCHED_TYPE_STRICT;
    }
    return SCHED_TYPE_NONE;
}

static inline uint32_t
l1_l0_ratio_for_sched_policy (const qos::QueueSchedulerPolicy& sched_policy)
{
    switch (sched_policy) {
        case qos::TM_QUEUE_SCHEDULER_2_4:
            return 2;
        case qos::TM_QUEUE_SCHEDULER_4_2:
            return 4;
        case qos::TM_QUEUE_SCHEDULER_8_1:
            return 8;
        default:
            return 0;
    }
}

static inline uint32_t
l2_l1_ratio_for_sched_policy (const qos::QueueSchedulerPolicy& sched_policy)
{
    switch (sched_policy) {
        case qos::TM_QUEUE_SCHEDULER_2_4:
            return 4;
        case qos::TM_QUEUE_SCHEDULER_4_2:
            return 2;
        case qos::TM_QUEUE_SCHEDULER_8_1:
            return 1;
        default:
            return 0;
    }
}

hal_ret_t
queue_create (QueueSpec& spec,
              QueueResponse *rsp)
{
    hal_ret_t                        ret;
    queue_t                          *l0_node;
    queue_t                          *l1_node;
    queue_t                          *l2_node;
    pd::pd_queue_args_t              pd_queue_args;
    uint32_t                         cnt_l0 = 0, cnt_l1 = 0, cnt_l2 = 0;
    uint32_t                         l1_l0_ratio, l2_l1_ratio;
    queue_t                          **l0_nodes = NULL;
    queue_t                          **l1_nodes = NULL;
    queue_t                          **l2_nodes = NULL;
    uint32_t                         i, j;
    uint32_t                         port_num;
    qos::QueueInfo                   *l0_node_spec = NULL;
    qos::QueueSchedulerNode          *l1_node_spec = NULL;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-QUEUE:{}: Queue create ", __func__);

    ret = validate_queue_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-QUEUE:{}: Queue create request validation failed."
                     " Err: {}",
                      __func__, HAL_RET_INVALID_ARG);
        return ret;
    }

    HAL_TRACE_DEBUG("PI-QUEUE:{}: Queue create for port {}",
                    __func__, spec.port_num());

    port_num = spec.port_num();
    l1_l0_ratio = l1_l0_ratio_for_sched_policy(spec.scheduler_policy());
    l2_l1_ratio = l2_l1_ratio_for_sched_policy(spec.scheduler_policy());

    cnt_l0 = (uint32_t)spec.queues_size();
    cnt_l1 = (uint32_t)spec.l1_nodes_size();
    if (cnt_l1 != (1+((cnt_l0-1)/l1_l0_ratio))) {
        HAL_TRACE_ERR("PI-QUEUE:{}: Queue create request failed."
                      "Invalid count of queues {} and l1 nodes {} ",
                      __func__, cnt_l0, cnt_l1);
        rsp->set_api_status(types::API_STATUS_QUEUE_COUNT_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    cnt_l2 = 1 + ((cnt_l1 - 1)/l2_l1_ratio);


    l0_nodes = (queue_t **) HAL_CALLOC(HAL_MEM_ALLOC_QOS,
                                       sizeof(queue_t *) *
                                       cnt_l0);
    HAL_ASSERT(l0_nodes != NULL);
    l1_nodes = (queue_t **) HAL_CALLOC(HAL_MEM_ALLOC_QOS,
                                       sizeof(queue_t *) *
                                       cnt_l1);
    HAL_ASSERT(l1_nodes != NULL);
    l2_nodes = (queue_t **) HAL_CALLOC(HAL_MEM_ALLOC_QOS,
                                       sizeof(queue_t *) *
                                       cnt_l2);
    HAL_ASSERT(l2_nodes != NULL);

    // Create L2 nodes
    for (i = 0; i < cnt_l2; i++) {
        l2_node = queue_node_alloc(L2_NODE, 0, port_num, i, SCHED_TYPE_STRICT);
        if (l2_node == NULL) {
            HAL_TRACE_ERR("PI-QUEUE:{}: Out of Memory. Err: {}",
                          __func__, HAL_RET_OOM);
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            ret = HAL_RET_OOM;
            goto end;
        }
        l2_nodes[i] = l2_node;
    }

    // Create the L1 nodes
    for (i = 0; i < cnt_l1; i++) {
        l1_node_spec = spec.mutable_l1_nodes(i);

        l1_node = queue_node_alloc(L1_NODE, 0,
                                   port_num, l1_node_spec->priority(),
                                   get_sched_type(spec.l1_nodes(i)));
        if (l1_node == NULL) {
            HAL_TRACE_ERR("PI-QUEUE:{}: Out of Memory. Err: {}",
                          __func__, HAL_RET_OOM);
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            ret = HAL_RET_OOM;
            goto end;
        }

        switch(l1_node->sched_type) {
            case SCHED_TYPE_DWRR:
                l1_node->u.dwrr.weight = l1_node_spec->dwrr().weight();
                break;
            case SCHED_TYPE_STRICT:
                l1_node->u.strict.rate = l1_node_spec->strict().rate();
                break;
            default:
                break;
        }
        l1_nodes[i] = l1_node;
    }

    // Create the l0 nodes
    for (i = 0; i < cnt_l0; i++) {
        l0_node_spec = spec.mutable_queues(i); 

        l0_node = queue_node_alloc(L0_NODE, l0_node_spec->key_or_handle().queue_id(),
                                 port_num, l0_node_spec->queue_info().priority(),
                                 get_sched_type(l0_node_spec->queue_info()));
        if (l0_node == NULL) {
            HAL_TRACE_ERR("PI-QUEUE:{}: Out of Memory. Err: {}",
                          __func__, HAL_RET_OOM);
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            ret = HAL_RET_OOM;
            goto end;
        }

        switch(l0_node->sched_type) {
            case SCHED_TYPE_DWRR:
                l0_node->u.dwrr.weight = l0_node_spec->queue_info().dwrr().weight();
                break;
            case SCHED_TYPE_STRICT:
                l0_node->u.strict.rate = l0_node_spec->queue_info().strict().rate();
                break;
            default:
                break;
        }

        l0_nodes[i] = l0_node;
    }

    /* Set the queue handles in the response message before sorting below */
    for (i = 0; i < cnt_l0; i++) {
        l0_node = l0_nodes[i];
        rsp->add_status()->mutable_queue_handle()->set_handle(l0_node->hal_handle);
    }

    // Sort the L0 and L1 nodes according to their priority
    std::sort(l0_nodes, l0_nodes + cnt_l0, queue_node_cmp_fn);
    std::sort(l1_nodes, l1_nodes + cnt_l1, queue_node_cmp_fn);

    // Now marry the L0, L1 and L2 nodes according to the scheduling policy
    for (i = 0; i < cnt_l0; i++) {
        j = i/l1_l0_ratio;
        HAL_ASSERT((j < cnt_l1) && "Unexpected error");
        l0_node = l0_nodes[i];
        l1_node = l1_nodes[j];
        l0_node->parent_handle = l1_node->hal_handle;
    }

    for (i = 0; i < cnt_l1; i++) {
        j = i/l2_l1_ratio;
        HAL_ASSERT((j < cnt_l2) && "Unexpected error");
        l1_node = l1_nodes[i];
        l2_node = l2_nodes[j];
        l1_node->parent_handle = l2_node->hal_handle;
    }

    // add all the queues to the db before calling PD. PD 
    // will query for the parent nodes
    add_queues_to_db(l0_nodes, cnt_l0, l1_nodes, cnt_l1, l2_nodes, cnt_l2);

    // allocate PD resources and program hardware
    pd::pd_queue_args_init(&pd_queue_args);
    pd_queue_args.cnt_l0 = cnt_l0;
    pd_queue_args.cnt_l1 = cnt_l1;
    pd_queue_args.cnt_l2 = cnt_l2;
    pd_queue_args.l0_nodes = l0_nodes;
    pd_queue_args.l1_nodes = l1_nodes;
    pd_queue_args.l2_nodes = l2_nodes;
    ret = pd::pd_queue_create(&pd_queue_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD queue create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    rsp->set_api_status(types::API_STATUS_OK);
end:

    if (ret != HAL_RET_OK) {
        remove_queues_from_db(l0_nodes, cnt_l0, l1_nodes, cnt_l1, l2_nodes, cnt_l2);
        for(i = 0; l0_nodes && (i < cnt_l0); i++) {
            if (l0_nodes[i]) { queue_free(l0_nodes[i]); }
        }
        for(i = 0; l1_nodes && (i < cnt_l1); i++) {
            if (l1_nodes[i]) { queue_free(l1_nodes[i]); }
        }
        for(i = 0; l2_nodes && (i < cnt_l2); i++) {
            if (l2_nodes[i]) { queue_free(l2_nodes[i]); }
        }
        rsp->clear_status();
    }

    l0_nodes ? HAL_FREE(HAL_MEM_ALLOC_QOS, l0_nodes) : HAL_NOP;
    l1_nodes ? HAL_FREE(HAL_MEM_ALLOC_QOS, l1_nodes) : HAL_NOP;
    l2_nodes ? HAL_FREE(HAL_MEM_ALLOC_QOS, l2_nodes) : HAL_NOP;
    
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
queue_update (QueueSpec& spec,
              QueueResponse *rsp)
{
    return HAL_RET_OK;
}



void *
policer_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((policer_t *)entry)->policer_id);
}

uint32_t
policer_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(policer_id_t)) % ht_size;
}

bool
policer_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(policer_id_t *)key1 == *(policer_id_t *)key2) {
        return true;
    }
    return false;
}

void *
policer_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((policer_t *)entry)->hal_handle);
}

uint32_t
policer_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
policer_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert this policer in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_policer_to_db (policer_t *policer)
{
    if (policer->spec.direction == INGRESS_QOS) {
        g_hal_state->ingress_policer_id_ht()->insert(policer, &policer->ht_ctxt);
        g_hal_state->ingress_policer_hal_handle_ht()->insert(policer, &policer->hal_handle_ht_ctxt);
    } else {
        g_hal_state->egress_policer_id_ht()->insert(policer, &policer->ht_ctxt);
        g_hal_state->egress_policer_hal_handle_ht()->insert(policer, &policer->hal_handle_ht_ctxt);
    }

    return HAL_RET_OK;
}

static qos_direction_e 
policer_proto_dir_to_enum (qos::PolicerDirection proto_dir)
{
    switch(proto_dir) {
        case qos::INGRESS_POLICER:
            return INGRESS_QOS;
        case qos::EGRESS_POLICER:
            return EGRESS_QOS;
        default:
            return INGRESS_QOS;
    }
}

static hal_ret_t 
validate_policer_create (PolicerSpec& spec,
                         PolicerResponse *rsp) 
{
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_POLICER_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().key_or_handle_case() != 
        qos::PolicerKeyHandle::kPolicerId) {
        rsp->set_api_status(types::API_STATUS_POLICER_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

static inline void
qos_extract_marking_action_from_spec (qos_marking_action_t *m_spec, 
                                      const qos::MarkingActionSpec& spec)

{
    m_spec->pcp_rewrite_en = spec.pcp_rewrite_en();
    m_spec->pcp = spec.pcp();
    m_spec->dscp_rewrite_en = spec.dscp_rewrite_en();
    m_spec->dscp = spec.dscp();
}

hal_ret_t
policer_create (PolicerSpec& spec,
                PolicerResponse *rsp)
{
    hal_ret_t             ret = HAL_RET_OK;
    policer_t             *policer;
    pd::pd_policer_args_t pd_policer_args;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-POLICER:{}: Policer create ", __func__);

    ret = validate_policer_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-POLICER:{}: Policer create request validation failed."
                     " Err: {}",
                      __func__, HAL_RET_INVALID_ARG);
        return ret;
    }

    HAL_TRACE_DEBUG("PI-POLICER:{}: Policer create for id {}",
                    __func__, spec.key_or_handle().policer_id());

    // allocate a policer instance
    policer = policer_alloc_init();
    if (policer == NULL) {
        HAL_TRACE_ERR("PI-POLICER:{}: Out of Memory. Err: {}",
                      __func__, HAL_RET_OOM);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // save the configs from the spec
    policer->policer_id = spec.key_or_handle().policer_id();
    policer->hal_handle = hal_alloc_handle();
    
    policer->spec.direction = policer_proto_dir_to_enum(spec.direction());
    policer->spec.bandwidth = spec.bandwidth();
    policer->spec.burst_size = spec.burst_size();
    if (spec.has_marking_spec()) {
        qos_extract_marking_action_from_spec(&policer->spec.marking_action, spec.marking_spec());
    }

    // allocate PD resources and program hardware
    pd::pd_policer_args_init(&pd_policer_args);
    pd_policer_args.policer = policer;
    ret = pd::pd_policer_create(&pd_policer_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD policer create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this policer to the db
    add_policer_to_db(policer);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->mutable_policer_handle()->set_handle(policer->hal_handle);

end:

    if (ret != HAL_RET_OK && policer != NULL) {
        policer_free(policer);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
policer_update (PolicerSpec& spec,
                PolicerResponse *rsp)
{
    return HAL_RET_OK;
}


hal_ret_t
qos_extract_action_from_spec (qos_action_t *qos_action,
                              const qos::QOSActions& spec,
                              qos_direction_e direction)
{
    queue_id_t queue_id;
    queue_t *queue;
    hal_handle_t queue_handle;
    policer_id_t policer_id;
    policer_t *policer;
    hal_handle_t policer_handle;

    qos_action->direction = direction;

    // Sanity checks
    if (spec.has_queue_key_or_handle()) {
        auto queue_kh = spec.queue_key_or_handle();
        if (queue_kh.key_or_handle_case() == qos::QueueKeyHandle::kQueueId) {
            queue_id = queue_kh.queue_id();
            queue = find_queue_by_id(queue_id);
        } else {
            queue_handle = queue_kh.queue_handle().handle();
            queue = find_queue_by_handle(queue_handle);
        }

        if (queue == NULL) {
            HAL_TRACE_ERR("PI-QOS:{}: Output queue not found",
                          __func__);
            return HAL_RET_OQUEUE_NOT_FOUND;
        }
        qos_action->queue_valid = true;
        qos_action->queue_handle = queue->hal_handle;
    }

    if (spec.has_policer_key_or_handle()) {
        auto policer_kh = spec.policer_key_or_handle();
        if (policer_kh.key_or_handle_case() == qos::PolicerKeyHandle::kPolicerId) {
            policer_id = policer_kh.policer_id();
            policer = find_policer_by_id(policer_id, direction);
        } else {
            policer_handle = policer_kh.policer_handle().handle();
            policer = find_policer_by_handle(policer_handle, direction);
        }

        if (policer == NULL) {
            HAL_TRACE_ERR("PI-QOS:{}: Policer not found",
                          __func__);
            return HAL_RET_POLICER_NOT_FOUND;
        }
        qos_action->policer_valid = true;
        qos_action->policer_handle = policer->hal_handle;
    }

    if (spec.has_marking_spec()) {
        if (direction == INGRESS_QOS) {
            HAL_TRACE_ERR("PI-QOS:{}: Marking is not supported in ingress",
                          __func__);
            return HAL_RET_INVALID_ARG;
        }

        qos_extract_marking_action_from_spec(&qos_action->marking_action,
                                             spec.marking_spec());
    }
    return HAL_RET_OK;
}

}    // namespace hal
