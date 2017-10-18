#ifndef __QOS_HPP__
#define __QOS_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "../../gen/proto/hal/qos.pb.h"

using hal::utils::ht_ctxt_t;
using hal::utils::bitmap;

using qos::BufPoolSpec;
using qos::BufPoolKeyHandle;
using qos::BufPoolRequestMsg;
using qos::BufPoolResponse;
using qos::BufPoolResponseMsg;
using qos::BufPoolDeleteRequestMsg;
using qos::BufPoolDeleteResponseMsg;
using qos::BufPoolGetRequestMsg;
using qos::BufPoolGetResponseMsg;
using qos::QueueSpec;
using qos::QueueKeyHandle;
using qos::QueueRequestMsg;
using qos::QueueResponse;
using qos::QueueResponseMsg;
using qos::QueueDeleteRequestMsg;
using qos::QueueDeleteResponseMsg;
using qos::QueueGetRequestMsg;
using qos::QueueGetResponseMsg;
using qos::PolicerSpec;
using qos::PolicerKeyHandle;
using qos::PolicerRequestMsg;
using qos::PolicerResponse;
using qos::PolicerResponseMsg;
using qos::PolicerDeleteRequestMsg;
using qos::PolicerDeleteResponseMsg;
using qos::PolicerGetRequestMsg;
using qos::PolicerGetResponseMsg;

namespace hal {

typedef enum {
    INGRESS_QOS = 0,
    EGRESS_QOS = 1
} qos_direction_e;

typedef struct qos_marking_action_s {
    bool     pcp_rewrite_en;     // Rewrite the 802.1q pcp value
    uint32_t pcp;                // 802.1q pcp value to mark with
    bool     dscp_rewrite_en;    // Rewrite the dscp value
    uint32_t dscp;               // DSCP value to mark with
} __PACK__ qos_marking_action_t;

// Structure to define qos action
typedef struct qos_action_s {
    qos_direction_e         direction;
    bool                    queue_valid;    // Queue is valid
    hal_handle_t            queue_handle;
    bool                    policer_valid;  // Policer is enabled
    hal_handle_t            policer_handle;
    qos_marking_action_t    marking_action;
} __PACK__ qos_action_t;

hal_ret_t
qos_extract_action_from_spec (qos_action_t *qos_action,
                              const qos::QOSActions& spec,
                              qos_direction_e direction);

hal_ret_t
qos_action_to_qos_action_spec (const qos_action_t *qos_action,
                                 qos::QOSActions *spec);

typedef uint32_t buf_pool_id_t;

#define HAL_MAX_BUF_POOLS            (32*HAL_MAX_TM_PORTS)
#define HAL_MAX_COSES                32

// Specifications for the buf-pool
typedef struct buf_pool_spec_s {
    uint32_t reserved_bytes;      // Number of bytes reserved for
                                  // this pool
    uint32_t headroom_bytes;      // Number of additional bytes
                                  // reserved for this pool
                                  // Before this is used, xoff will be asserted
    uint32_t sharing_factor;      // Sharing factor used to grab
                                  // buffers from shared pool
    uint32_t xon_threshold;       // Relative threshold from the
                                  // max occupancy at which xoff will be cleared
    uint32_t xoff_clear_limit;    // When the pool occupancy goes
                                  // below this limit, xoff will be cleared
    uint32_t mtu;                 // MTU of the packets in bytes
    bitmap   *cos_bmp;            // COS values using this buffer pool
} __PACK__ buf_pool_spec_t;

// Buf-Pool structure
typedef struct buf_pool_s {
    hal_spinlock_t  slock;                 // lock to protect this structure
    buf_pool_id_t   buf_pool_id;           // buf-pool id assigned
    uint32_t        port_num;              // buf-pool's port number

    buf_pool_spec_t spec;

    hal_handle_t    hal_handle;            // HAL allocated handle

    ht_ctxt_t       ht_ctxt;               // buf_pool_id based hash table ctxt
    ht_ctxt_t       hal_handle_ht_ctxt;    // hal handle based hash table ctxt

    void            *pd_buf_pool;
} __PACK__ buf_pool_t;

static inline hal_ret_t
buf_pool_free (buf_pool_t *buf_pool)
{
    buf_pool->spec.cos_bmp ? delete buf_pool->spec.cos_bmp : HAL_NOP;

    HAL_SPINLOCK_DESTROY(&buf_pool->slock);
    g_hal_state->buf_pool_slab()->free(buf_pool);
    return HAL_RET_OK;
}

// allocate a buf-pool instance
static inline buf_pool_t *
buf_pool_alloc (void)
{
    buf_pool_t    *buf_pool;

    buf_pool = (buf_pool_t *)g_hal_state->buf_pool_slab()->alloc();
    if (buf_pool == NULL) {
        return NULL;
    }
    buf_pool->spec.cos_bmp = bitmap::factory(HAL_MAX_COSES, true);
    if (buf_pool->spec.cos_bmp == NULL) {
        buf_pool_free(buf_pool);
        return NULL;
    }
    return buf_pool;
}

// initialize a buf-pool instance
static inline buf_pool_t *
buf_pool_init (buf_pool_t *buf_pool)
{
    if (!buf_pool) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&buf_pool->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    buf_pool->ht_ctxt.reset();
    buf_pool->hal_handle_ht_ctxt.reset();

    return buf_pool;
}

// allocate and initialize a buf-pool instance
static inline buf_pool_t *
buf_pool_alloc_init (void)
{
    return buf_pool_init(buf_pool_alloc());
}

static inline buf_pool_t *
find_buf_pool_by_id (buf_pool_id_t buf_pool_id)
{
    return (buf_pool_t *)g_hal_state->buf_pool_id_ht()->lookup(&buf_pool_id);
}

static inline buf_pool_t *
find_buf_pool_by_handle (hal_handle_t handle)
{
    return (buf_pool_t *)g_hal_state->buf_pool_hal_handle_ht()->lookup(&handle);
}

extern void *buf_pool_get_key_func(void *entry);
extern uint32_t buf_pool_compute_hash_func(void *key, uint32_t ht_size);
extern bool buf_pool_compare_key_func(void *key1, void *key2);

extern void *buf_pool_get_handle_key_func(void *entry);
extern uint32_t buf_pool_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool buf_pool_compare_handle_key_func(void *key1, void *key2);


hal_ret_t buf_pool_create(qos::BufPoolSpec& spec,
                          qos::BufPoolResponse *rsp);
hal_ret_t buf_pool_update(qos::BufPoolSpec& spec,
                          qos::BufPoolResponse *rsp);




typedef uint32_t queue_id_t;

#define HAL_MAX_QUEUE_NODES (HAL_MAX_TM_PORTS*52)

typedef enum {
    L0_NODE = 0,
    L1_NODE = 1,
    L2_NODE = 2
} queue_node_type_e;

typedef enum {
    SCHED_TYPE_NONE = 0,
    SCHED_TYPE_STRICT = 1,
    SCHED_TYPE_DWRR = 2
} sched_type_e;

typedef union sched_config_s {
    struct {
        uint32_t  weight;                // Weight for DWRR
    } dwrr;
    struct {
        uint32_t  rate;                  // Rate for strict priority scheduling
    } strict;
} __PACK__ sched_config_t;

// Queue structure
typedef struct queue_s {
    hal_spinlock_t    slock;                 // lock to protect this structure
    queue_id_t        queue_id;              // Queue id assigned
    uint32_t          port_num;              // Queue's port number

    queue_node_type_e node_type;             // Node type
    uint32_t          priority;              // Priority of the queue
    sched_type_e      sched_type;            // Scheduler type for this queue
    sched_config_t    u;

    hal_handle_t      parent_handle;         // Parent queue node's handle


    hal_handle_t      hal_handle;            // HAL allocated handle

    ht_ctxt_t         ht_ctxt;               // queue_id based hash table ctxt
    ht_ctxt_t         hal_handle_ht_ctxt;    // hal handle based hash table ctxt

    void              *pd_queue;
} __PACK__ queue_t;


// allocate a Queue instance
static inline queue_t *
queue_alloc (void)
{
    queue_t    *queue;

    queue = (queue_t *)g_hal_state->queue_slab()->alloc();
    if (queue == NULL) {
        return NULL;
    }
    return queue;
}

// initialize a Queue instance
static inline queue_t *
queue_init (queue_t *queue)
{
    if (!queue) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&queue->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    queue->ht_ctxt.reset();
    queue->hal_handle_ht_ctxt.reset();

    return queue;
}

// allocate and initialize a queue instance
static inline queue_t *
queue_alloc_init (void)
{
    return queue_init(queue_alloc());
}

static inline hal_ret_t
queue_free (queue_t *queue)
{
    HAL_SPINLOCK_DESTROY(&queue->slock);
    g_hal_state->queue_slab()->free(queue);
    return HAL_RET_OK;
}

static inline queue_t *
find_queue_by_id (queue_id_t queue_id)
{
    return (queue_t *)g_hal_state->queue_id_ht()->lookup(&queue_id);
}

static inline queue_t *
find_queue_by_handle (hal_handle_t handle)
{
    return (queue_t *)g_hal_state->queue_hal_handle_ht()->lookup(&handle);
}

extern void *queue_get_key_func(void *entry);
extern uint32_t queue_compute_hash_func(void *key, uint32_t ht_size);
extern bool queue_compare_key_func(void *key1, void *key2);

extern void *queue_get_handle_key_func(void *entry);
extern uint32_t queue_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool queue_compare_handle_key_func(void *key1, void *key2);


hal_ret_t queue_create(qos::QueueSpec& spec,
                       qos::QueueResponse *rsp);
hal_ret_t queue_update(qos::QueueSpec& spec,
                       qos::QueueResponse *rsp);


typedef uint32_t policer_id_t;

#define HAL_MAX_POLICERS    2048

// Specifications for the Policer
typedef struct policer_spec_s {
    qos_direction_e      direction;
    uint32_t             bandwidth;
    uint32_t             burst_size;
    qos_marking_action_t marking_action;
} __PACK__ policer_spec_t;

// Policer structure
typedef struct policer_s {
    hal_spinlock_t slock;                 // lock to protect this structure
    policer_id_t   policer_id;           // Policer id assigned

    policer_spec_t spec;

    hal_handle_t   hal_handle;            // HAL allocated handle

    ht_ctxt_t      ht_ctxt;               // policer_id based hash table ctxt
    ht_ctxt_t      hal_handle_ht_ctxt;    // hal handle based hash table ctxt

    void           *pd_policer;
} __PACK__ policer_t;


// allocate a Policer instance
static inline policer_t *
policer_alloc (void)
{
    policer_t    *policer;

    policer = (policer_t *)g_hal_state->policer_slab()->alloc();
    if (policer == NULL) {
        return NULL;
    }
    return policer;
}

// initialize a Policer instance
static inline policer_t *
policer_init (policer_t *policer)
{
    if (!policer) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&policer->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    policer->ht_ctxt.reset();
    policer->hal_handle_ht_ctxt.reset();

    return policer;
}

// allocate and initialize a policer instance
static inline policer_t *
policer_alloc_init (void)
{
    return policer_init(policer_alloc());
}

static inline hal_ret_t
policer_free (policer_t *policer)
{
    HAL_SPINLOCK_DESTROY(&policer->slock);
    g_hal_state->policer_slab()->free(policer);
    return HAL_RET_OK;
}

static inline policer_t *
find_policer_by_id (policer_id_t policer_id, qos_direction_e direction)
{
    if (direction == INGRESS_QOS) {
        return (policer_t *)g_hal_state->ingress_policer_id_ht()->lookup(&policer_id);
    } else {
        return (policer_t *)g_hal_state->egress_policer_id_ht()->lookup(&policer_id);
    }
}

static inline policer_t *
find_policer_by_handle (hal_handle_t handle, qos_direction_e direction)
{
    if (direction == INGRESS_QOS) {
        return (policer_t *)g_hal_state->ingress_policer_hal_handle_ht()->lookup(&handle);
    } else {
        return (policer_t *)g_hal_state->egress_policer_hal_handle_ht()->lookup(&handle);
    }
}

extern void *policer_get_key_func(void *entry);
extern uint32_t policer_compute_hash_func(void *key, uint32_t ht_size);
extern bool policer_compare_key_func(void *key1, void *key2);

extern void *policer_get_handle_key_func(void *entry);
extern uint32_t policer_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool policer_compare_handle_key_func(void *key1, void *key2);

hal_ret_t policer_create(qos::PolicerSpec& spec,
                         qos::PolicerResponse *rsp);
hal_ret_t policer_update(qos::PolicerSpec& spec,
                         qos::PolicerResponse *rsp);

}    // namespace hal

#endif    // __QOS_HPP__
