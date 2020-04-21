// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_TM_UTILS_HPP__
#define __ELBA_TM_UTILS_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/elba/elba_p4.hpp"
#include "platform/elba/elba_tm_rw.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace elba {

#define ELBA_QUEUES_PER_PORT          (8)
#define ELBA_QOS_MAX_IQS              (32)
#define ELBA_QOS_MAX_OQS              (32)

typedef struct elba_queue_s {
    bool   valid;
    tm_q_t queue;
} __PACK__ elba_queue_t;

typedef struct elba_occupancy_s {
    tm_port_t port;
    uint32_t  queue_occupancy[ELBA_QUEUES_PER_PORT];
} elba_occupancy_t;

typedef struct elba_threshold_s {
    uint32_t hbm_context;
    uint32_t xon_threshold;
    uint32_t xoff_threshold;
} elba_threshold_t;

typedef struct elba_thresholds_s {
    elba_occupancy_t occupancy[ELBA_TM_NUM_UPLINK_PORTS];
    elba_threshold_t threshold[ELBA_TM_MAX_HBM_ETH_CONTEXTS];
} elba_thresholds_t;

sdk_ret_t elba_thresholds_get(elba_thresholds_t *thresholds);

typedef struct elba_input_queue_stats_s {
    elba_queue_t iq;
    tm_iq_stats_t stats;
} elba_input_queue_stats_t;

typedef struct elba_output_queue_stats_s {
    elba_queue_t oq;
    tm_oq_stats_t stats;
} elba_output_queue_stats_t;

typedef struct elba_queue_stats_s {
    elba_input_queue_stats_t  iq_stats[ELBA_TM_MAX_IQS];
    elba_output_queue_stats_t oq_stats[ELBA_TM_MAX_OQS];
} elba_queue_stats_t;

sdk_ret_t
elba_queue_stats_get(tm_port_t port, elba_queue_stats_t *stats);

sdk_ret_t
elba_populate_queue_stats(tm_port_t port,
                           elba_queue_t iqs[ELBA_TM_MAX_IQS],
                           elba_queue_t oqs[ELBA_TM_MAX_OQS],
                           elba_queue_stats_t *stats);

sdk_ret_t
elba_queue_credits_get(sdk::asic::pd::queue_credits_get_cb_t cb, void *ctxt);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // __ELBA_TM_UTILS_HPP__
