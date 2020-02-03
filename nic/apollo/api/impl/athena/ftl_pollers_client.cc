//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Flow Table Library software pollers client
///
//----------------------------------------------------------------------------

#include "ftl_pollers_client.hpp"
#include "ftl_dev_impl.hpp"

namespace ftl_pollers_client {

/*
 * Some theoretical max for convenience; in reality number of poller
 * queues would be much smaller.
 */
#define FTL_POLLERS_MAX_QUEUES          64
#define FTL_POLLERS_BURST_COUNT         128
#define FTL_POLLERS_BURST_BUF_SZ        (FTL_POLLERS_BURST_COUNT * \
                                         sizeof(poller_slot_data_t))
static uint32_t             pollers_qcount;

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
                   expiry_user_cb_t expiry_user_cb);
static void
expiry_submap_process(uint32_t submap_id,
                      uint32_t expiry_id_base,
                      pds_flow_age_expiry_type_t expiry_type,
                      uint8_t submap,
                      expiry_user_cb_t expiry_user_cb);
static void
expiry_cb_dflt(uint32_t expiry_id,
               pds_flow_age_expiry_type_t expiry_type);

sdk_ret_t
init(void)
{
    client_queue_t          *queue;
    sdk_ret_t               ret;

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
            queue->poller_slot_data =
                   (poller_slot_data_t *)rte_malloc("poller_slot_data",
                                                    FTL_POLLERS_BURST_BUF_SZ, 0);
            if (!queue->poller_slot_data) {
                PDS_TRACE_ERR("failed to allocate slot data for qid %u", qid);
                return SDK_RET_OOM;
            }
        }
    }

    PDS_TRACE_DEBUG("init completed");
    return ret;
}

uint32_t
qcount_get(void)
{
    return pollers_qcount;
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
     expiry_user_cb_t expiry_user_cb,
     bool bringup_log)
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

    if (!expiry_user_cb) {
        expiry_user_cb = expiry_cb_dflt;
    }

    burst_count = FTL_POLLERS_BURST_COUNT;
    ret = ftl_dev_impl::pollers_dequeue_burst(qid, queue->poller_slot_data,
                                              FTL_POLLERS_BURST_BUF_SZ,
                                              &burst_count);
    if ((ret == SDK_RET_OK) && burst_count) {
        if (bringup_log) {
            PDS_TRACE_DEBUG("pollers_dequeue_burst poller_qid %d burst_count %u",
                            qid, burst_count);
        }
        slot_data = queue->poller_slot_data;
        for (uint32_t i = 0; i < burst_count; i++, slot_data++) {
            if (bringup_log) {
                PDS_TRACE_DEBUG("table_id_base %u scanner_qid %u scanner_qtype %u "
                                "flags %u", slot_data->table_id_base,
                                slot_data->scanner_qid, slot_data->scanner_qtype,
                                slot_data->flags);
                PDS_TRACE_DEBUG("expiry_map0 0x%llx expiry_map1 0x%llx expiry_map2 "
                                "0x%llx expiry_map3 0x%llx", slot_data->expiry_map[0],
                                slot_data->expiry_map[1], slot_data->expiry_map[2],
                                slot_data->expiry_map[3]);
            }

            /*
             * Reschedule scanner if applicable
             */
            if (slot_data->flags & SCANNER_RESCHED_REQUESTED) {
                ftl_dev_impl::scanners_start_single(
                              (enum ftl_qtype)slot_data->scanner_qtype,
                              slot_data->scanner_qid);
            }

            /*
             * Note: loops are unrolled here for efficiency
             */
            expiry_type = slot_data->scanner_qtype == FTL_QTYPE_SCANNER_SESSION ?
                          EXPIRY_TYPE_SESSION : EXPIRY_TYPE_CONNTRACK;

#if SCANNER_EXPIRY_NUM_MAP_ENTRIES_MAX != 4
#error "Need more unrolled calls to expiry_map_process"
#endif
            expiry_map_process(0, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[0], expiry_user_cb);
            expiry_map_process(1, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[1], expiry_user_cb);
            expiry_map_process(2, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[2], expiry_user_cb);
            expiry_map_process(3, slot_data->table_id_base, expiry_type,
                               slot_data->expiry_map[3], expiry_user_cb);
        }
    }

    return ret;
}

static void
expiry_map_process(uint32_t map_id,
                   uint32_t table_id_base,
                   pds_flow_age_expiry_type_t expiry_type,
                   uint64_t expiry_map,
                   expiry_user_cb_t expiry_user_cb)
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
                              submaps.s.submap[0], expiry_user_cb);
        expiry_submap_process(1, expiry_id, expiry_type,
                              submaps.s.submap[1], expiry_user_cb);
        expiry_submap_process(2, expiry_id, expiry_type,
                              submaps.s.submap[2], expiry_user_cb);
        expiry_submap_process(3, expiry_id, expiry_type,
                              submaps.s.submap[3], expiry_user_cb);
        expiry_submap_process(4, expiry_id, expiry_type,
                              submaps.s.submap[4], expiry_user_cb);
        expiry_submap_process(5, expiry_id, expiry_type,
                              submaps.s.submap[5], expiry_user_cb);
        expiry_submap_process(6, expiry_id, expiry_type,
                              submaps.s.submap[6], expiry_user_cb);
        expiry_submap_process(7, expiry_id, expiry_type,
                              submaps.s.submap[7], expiry_user_cb);
    }
}

static void
expiry_submap_process(uint32_t submap_id,
                      uint32_t expiry_id_base,
                      pds_flow_age_expiry_type_t expiry_type,
                      uint8_t submap,
                      expiry_user_cb_t expiry_user_cb)
{
    uint32_t    sub_expiry_id;

    sub_expiry_id = expiry_id_base +
                    (submap_id * sizeof(uint8_t) * BITS_PER_BYTE);
    while (submap) {
        if (submap & 1) {
            expiry_user_cb(sub_expiry_id, expiry_type);
        }

        sub_expiry_id++;
        submap >>= 1;
    }
}

static void
expiry_cb_dflt(uint32_t expiry_id,
               pds_flow_age_expiry_type_t expiry_type)
{
    PDS_TRACE_DEBUG("entry %u type %d expired", expiry_id, expiry_type);
}

} // namespace ftl_pollers_client
