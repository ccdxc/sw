//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Flow Table Library software pollers client
///
//----------------------------------------------------------------------------

#include "ftl_dev_impl.hpp"
#include "ftl_pollers_client.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "nic/apollo/api/include/athena/pds_conntrack.h"

#include <rte_spinlock.h>
#include <rte_atomic.h>

namespace ftl_pollers_client {

/*
 * Some theoretical max for convenience; in reality number of poller
 * queues would be much smaller.
 */
#define FTL_POLLERS_MAX_QUEUES          64
#define FTL_POLLERS_BURST_COUNT         128
#define FTL_POLLERS_BURST_BUF_SZ        (FTL_POLLERS_BURST_COUNT * \
                                         sizeof(poller_slot_data_t))
static rte_atomic16_t       module_inited = RTE_ATOMIC16_INIT(0);
static uint32_t             pollers_qcount;
static bool                 expiry_log_en;

static pds_ret_t
expiry_fn_dflt_fn(uint32_t expiry_id,
                  pds_flow_age_expiry_type_t expiry_type,
                  void *user_ctx);

/*
 * The user is expected to invoke poll_control() only once during
 * initialization of the user code (as opposed to doing so dynamically
 * multiple times during the course of execution). As a result, it is
 * not necessary to atomically store/fetch the expiry_fn field (which is
 * a multi-byte pointer). However, if in the future the above assumption 
 * happens to prove false, the following #define may be enabled to get the
 * needed protection.
 */
//#define POLLERS_EXPIRY_FN_ATOMIC

#ifdef POLLERS_EXPIRY_FN_ATOMIC
static rte_atomic64_t       pollers_expiry_fn = 
                            RTE_ATOMIC64_INIT((int64_t)expiry_fn_dflt_fn);
#else
static pds_flow_expiry_fn_t pollers_expiry_fn = expiry_fn_dflt_fn;
#endif
volatile uint8_t            user_will_poll_;

static inline void
pollers_expiry_fn_set(pds_flow_expiry_fn_t expiry_fn)
{
    if (!expiry_fn) {
        expiry_fn = expiry_fn_dflt_fn;
    }
#ifdef POLLERS_EXPIRY_FN_ATOMIC
    rte_atomic64_set(&pollers_expiry_fn, (int64_t)expiry_fn);
#else
    pollers_expiry_fn = expiry_fn;
#endif
}

static inline pds_flow_expiry_fn_t
pollers_expiry_fn_get(void)
{
#ifdef POLLERS_EXPIRY_FN_ATOMIC
    return (pds_flow_expiry_fn_t)rte_atomic64_read(&pollers_expiry_fn);
#else
    return pollers_expiry_fn;
#endif
}

/*
 * Expiry submaps for unrolling loops
 */
typedef union {
    struct {
        uint8_t     submap[8];
    } __PACK__ s;
    uint64_t        full_map;
} __PACK__ expiry_submaps_t;

/*
 * Pollers per-queue state
 */
typedef struct {
    uint32_t                qid;
    poller_slot_data_t      *poller_slot_data;
} client_queue_t;

static client_queue_t       client_queue[FTL_POLLERS_MAX_QUEUES];

static client_queue_t *
client_queue_get(uint32_t qid)
{
    return (qid < pollers_qcount) && (qid < FTL_POLLERS_MAX_QUEUES) ?
            &client_queue[qid] : nullptr;
}

static void
expiry_map_process(uint32_t map_id,
                   uint32_t table_id_base,
                   pds_flow_age_expiry_type_t expiry_type,
                   uint64_t expiry_map,
                   void *user_ctx);
static void
expiry_submap_process(uint32_t submap_id,
                      uint32_t expiry_id_base,
                      pds_flow_age_expiry_type_t expiry_type,
                      uint8_t submap,
                      void *user_ctx);
sdk_ret_t
init(void)
{
    client_queue_t  *queue;
    sdk_ret_t       ret = SDK_RET_OK;

    if (rte_atomic16_test_and_set(&module_inited)) {
        ret = ftl_dev_impl::init();
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("init failed: error %d", ret);
            return ret;
        }

        ret = ftl_dev_impl::pollers_qcount_get(&pollers_qcount);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("pollers_qcount_get failed: error %d", ret);
            return ret;
        }
        PDS_TRACE_DEBUG("pollers_qcount %u", pollers_qcount);

        queue = &client_queue[0];
        for (uint32_t qid = 0; qid < FTL_POLLERS_MAX_QUEUES; qid++, queue++) {
            queue->qid = qid;
            if (qid < pollers_qcount) {
                queue->poller_slot_data = (poller_slot_data_t *)
                               SDK_MALLOC(SDK_MEM_ALLOC_FTL_POLLER_SLOT_DATA,
                                          FTL_POLLERS_BURST_BUF_SZ);
                if (!queue->poller_slot_data) {
                    PDS_TRACE_ERR("failed to allocate slot data for qid %u", qid);
                    return SDK_RET_OOM;
                }
            }
        }

        PDS_TRACE_DEBUG("init completed");
    }
    return ret;
}

uint32_t
qcount_get(void)
{
    return pollers_qcount;
}

pds_ret_t
expiry_fn_dflt(pds_flow_expiry_fn_t *ret_fn_dflt)
{
    *ret_fn_dflt = expiry_fn_dflt_fn;
    return PDS_RET_OK;
}

sdk_ret_t
poll_control(bool user_will_poll,
             pds_flow_expiry_fn_t expiry_fn)
{
    user_will_poll_ = user_will_poll;
    pollers_expiry_fn_set(expiry_fn);
    return SDK_RET_OK;
}

sdk_ret_t
force_session_expired_ts_set(bool force_expired_ts)
{
    return ftl_dev_impl::force_session_expired_ts_set(force_expired_ts);
}

sdk_ret_t
force_conntrack_expired_ts_set(bool force_expired_ts)
{
    return ftl_dev_impl::force_conntrack_expired_ts_set(force_expired_ts);
}

sdk_ret_t
session_scanners_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics)
{
    return ftl_dev_impl::session_scanners_metrics_get(metrics);
}

sdk_ret_t
conntrack_scanners_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics)
{
    return ftl_dev_impl::conntrack_scanners_metrics_get(metrics);
}

sdk_ret_t
pollers_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics)
{
    return ftl_dev_impl::pollers_metrics_get(metrics);
}

uint32_t
session_table_depth_get(void)
{
    uint32_t    table_depth;

    return ftl_dev_impl::session_table_depth_get(&table_depth) == SDK_RET_OK ?
           table_depth : 0;
}

uint32_t
conntrack_table_depth_get(void)
{
    uint32_t    table_depth;

    return ftl_dev_impl::conntrack_table_depth_get(&table_depth) == SDK_RET_OK ?
           table_depth : 0;
}

void
expiry_log_set(bool enable_sense)
{
    expiry_log_en = enable_sense;
}

/*
 * Submit a burst dequeue on a poller queue corresponding to qid.
 *
 * Typically each poller queue should be polled from a different lcore for
 * maximum efficiency, but the implementation is flexible enough to also
 * allow multiple queues to be polled from one lcore, or one queue to be poll
 * from multiple lcores. The internal API ftl_dev_impl::pollers_dequeue_burst()
 * contains the necessary locking but would obviously be less efficient
 * when a given queue is "overly polled".
 */
sdk_ret_t
poll(uint32_t qid,
     void *user_ctx)
{
    client_queue_t              *queue = client_queue_get(qid);
    poller_slot_data_t          *slot_data;
    pds_flow_age_expiry_type_t  expiry_type;
    uint32_t                    burst_count;
    sdk_ret_t                   ret = SDK_RET_OK;

    if (!queue) {
        PDS_TRACE_ERR("invalid qid %u max is %u", qid, pollers_qcount);
        return SDK_RET_INVALID_ARG;
    }

    burst_count = FTL_POLLERS_BURST_COUNT;
    ret = ftl_dev_impl::pollers_dequeue_burst(qid, queue->poller_slot_data,
                                              FTL_POLLERS_BURST_BUF_SZ,
                                              &burst_count);
    if ((ret == SDK_RET_OK) && burst_count) {
        if (expiry_log_en) {
            PDS_TRACE_DEBUG("pollers_dequeue_burst poller_qid %d burst_count %u",
                            qid, burst_count);
        }
        slot_data = queue->poller_slot_data;
        for (uint32_t i = 0; i < burst_count; i++, slot_data++) {
            if (expiry_log_en) {
                PDS_TRACE_DEBUG("table_id_base %u scanner_qid %u scanner_qtype %u "
                                "flags %u", slot_data->table_id_base,
                                slot_data->scanner_qid, slot_data->scanner_qtype,
                                slot_data->flags);
                PDS_TRACE_DEBUG(" expiry_map0 0x%" PRIx64 
                                " expiry_map1 0x%" PRIx64 
                                " expiry_map2 0x%" PRIx64 
                                " expiry_map3 0x%" PRIx64,
                                slot_data->expiry_map[0], slot_data->expiry_map[1],
                                slot_data->expiry_map[2], slot_data->expiry_map[3]);
            }

            /*
             * Note: loops are unrolled here for efficiency
             */
            expiry_type = slot_data->scanner_qtype == FTL_DEV_QTYPE_SCANNER_SESSION ?
                          EXPIRY_TYPE_SESSION : EXPIRY_TYPE_CONNTRACK;

#if SCANNER_EXPIRY_NUM_MAP_ENTRIES_MAX != 4
#error "May need more unrolled calls to expiry_map_process"
#endif
            expiry_map_process(0, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[0], user_ctx);
            expiry_map_process(1, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[1], user_ctx);
            expiry_map_process(2, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[2], user_ctx);
            expiry_map_process(3, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[3], user_ctx);
            /*
             * Reschedule scanner if applicable
             */
            if (slot_data->flags & SCANNER_RESCHED_REQUESTED) {
                ftl_dev_impl::scanners_start_single(
                              (enum ftl_qtype)slot_data->scanner_qtype,
                              slot_data->scanner_qid);
            }
        }
    }

    return ret;
}

static void
expiry_map_process(uint32_t map_id,
                   uint32_t table_id_base,
                   pds_flow_age_expiry_type_t expiry_type,
                   uint64_t expiry_map,
                   void *user_ctx)
{
    expiry_submaps_t    submaps;
    uint32_t            expiry_id;

    if (expiry_map) {

        /*
         * Note: loops are unrolled further here for efficiency
         */
        submaps.full_map = expiry_map;
        expiry_id = table_id_base +
                    (map_id * sizeof(uint64_t) * BITS_PER_BYTE);
        expiry_submap_process(0, expiry_id, expiry_type,
                              submaps.s.submap[0], user_ctx);
        expiry_submap_process(1, expiry_id, expiry_type,
                              submaps.s.submap[1], user_ctx);
        expiry_submap_process(2, expiry_id, expiry_type,
                              submaps.s.submap[2], user_ctx);
        expiry_submap_process(3, expiry_id, expiry_type,
                              submaps.s.submap[3], user_ctx);
        expiry_submap_process(4, expiry_id, expiry_type,
                              submaps.s.submap[4], user_ctx);
        expiry_submap_process(5, expiry_id, expiry_type,
                              submaps.s.submap[5], user_ctx);
        expiry_submap_process(6, expiry_id, expiry_type,
                              submaps.s.submap[6], user_ctx);
        expiry_submap_process(7, expiry_id, expiry_type,
                              submaps.s.submap[7], user_ctx);
    }
}

static void
expiry_submap_process(uint32_t submap_id,
                      uint32_t expiry_id_base,
                      pds_flow_age_expiry_type_t expiry_type,
                      uint8_t submap,
                      void *user_ctx)
{
    pds_flow_expiry_fn_t    expiry_fn = pollers_expiry_fn_get();
    uint32_t                sub_expiry_id;

#define SUBMAP_TST_BIT(bit)                                         \
    if (submap & ((1 << (bit)))) {                                  \
        expiry_fn(sub_expiry_id + (bit), expiry_type, user_ctx);    \
    }                                                               \

    if (submap) {
        sub_expiry_id = expiry_id_base +
                        (submap_id * sizeof(uint8_t) * BITS_PER_BYTE);
        SUBMAP_TST_BIT(0);
        SUBMAP_TST_BIT(1);
        SUBMAP_TST_BIT(2);
        SUBMAP_TST_BIT(3);
        SUBMAP_TST_BIT(4);
        SUBMAP_TST_BIT(5);
        SUBMAP_TST_BIT(6);
        SUBMAP_TST_BIT(7);
    }
}

static pds_ret_t
expiry_fn_dflt_fn(uint32_t expiry_id,
                  pds_flow_age_expiry_type_t expiry_type,
                  void *user_ctx)
{
    pds_ret_t   ret = PDS_RET_INVALID_ARG;

    if (expiry_log_en) {
        PDS_TRACE_DEBUG("entry %u type %d expired", expiry_id, expiry_type);
    }

    switch (expiry_type) {

    case EXPIRY_TYPE_SESSION: {
        pds_flow_session_key_t  key = {0};

        /*
         * TODO: need to also delete at least the corresponding flow cache
         * (hash) entry but to do that, the session entry must provide
         * a back pointer to the flow cache index.
         */
        key.session_info_id = expiry_id;
        key.direction = HOST_TO_SWITCH | SWITCH_TO_HOST;
        ret = pds_flow_session_info_delete(&key);
        break;
    }

    case EXPIRY_TYPE_CONNTRACK: {
        pds_conntrack_key_t     key = {0};

        /*
         * Temporary: the following code is just a placeholder as the API
         * pds_conntrack_state_delete() alone is probably not sufficient
         * for completely removing a conntrack entry.
         */
        key.conntrack_id = expiry_id;
        ret = pds_conntrack_state_delete(&key);
        break;
    }

    default:
        break;
    }

    return ret;
}

} // namespace ftl_pollers_client
