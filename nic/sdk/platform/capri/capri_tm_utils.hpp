// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_TM_UTILS_HPP__
#define __CAPRI_TM_UTILS_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/capri/capri_p4.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace capri {

#define CAPRI_QUEUES_PER_PORT          (8)
#define CAPRI_QOS_MAX_IQS              (32)
#define CAPRI_QOS_MAX_OQS              (32)

typedef struct capri_queue_s {
    bool   valid;
    tm_q_t queue;
} __PACK__ capri_queue_t;

typedef struct capri_occupancy_s {
    tm_port_t port;
    uint32_t  queue_occupancy[CAPRI_QUEUES_PER_PORT];
} capri_occupancy_t;

typedef struct capri_threshold_s {
    uint32_t hbm_context;
    uint32_t xon_threshold;
    uint32_t xoff_threshold;
} capri_threshold_t;

typedef struct capri_thresholds_s {
    capri_occupancy_t occupancy[CAPRI_TM_NUM_UPLINK_PORTS];
    capri_threshold_t threshold[CAPRI_TM_MAX_HBM_ETH_CONTEXTS];
} capri_thresholds_t;

sdk_ret_t capri_thresholds_get(capri_thresholds_t *thresholds);

typedef struct capri_input_queue_stats_s {
    capri_queue_t iq;
    tm_iq_stats_t stats;
} capri_input_queue_stats_t;

typedef struct capri_output_queue_stats_s {
    capri_queue_t oq;
    tm_oq_stats_t stats;
} capri_output_queue_stats_t;

typedef struct capri_queue_stats_s {
    capri_input_queue_stats_t  iq_stats[CAPRI_TM_MAX_IQS];
    capri_output_queue_stats_t oq_stats[CAPRI_TM_MAX_OQS];
} capri_queue_stats_t;

sdk_ret_t
capri_queue_stats_get(tm_port_t port, capri_queue_stats_t *stats);

sdk_ret_t
capri_populate_queue_stats(tm_port_t port,
                           capri_queue_t iqs[CAPRI_TM_MAX_IQS],
                           capri_queue_t oqs[CAPRI_TM_MAX_OQS],
                           capri_queue_stats_t *stats);

sdk_ret_t
capri_queue_credits_get(sdk::asic::pd::queue_credits_get_cb_t cb, void *ctxt);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif    // __CAPRI_TM_UTILS_HPP__
