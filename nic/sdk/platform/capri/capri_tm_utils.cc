//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "platform/capri/capri_p4.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "platform/capri/capri_tm_utils.hpp"
#include "platform/capri/capri_state.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace capri {

using sdk::asic::pd::queue_credits_get_cb_t;
using sdk::asic::pd::port_queue_credit_t;
using sdk::asic::pd::queue_credit_t;

// ----------------------------------------------------------------------------
// capri thresholds get
// ----------------------------------------------------------------------------
sdk_ret_t
capri_thresholds_get (capri_thresholds_t *thresholds)
{
    sdk_ret_t ret = SDK_RET_OK;

    for (uint32_t port = CAPRI_TM_UPLINK_PORT_BEGIN; port <=
         CAPRI_TM_UPLINK_PORT_END; port++) {
        thresholds->occupancy[port].port = port;
        for (uint32_t i = 0; i < CAPRI_QUEUES_PER_PORT; i++) {
            thresholds->occupancy[port].queue_occupancy[i] =
                            capri_tm_get_port_occupancy(port, i);
        }
    }
    for (uint32_t i = 0; i < CAPRI_TM_MAX_HBM_ETH_CONTEXTS; i++) {
        thresholds->threshold[i].hbm_context = i;
        thresholds->threshold[i].xon_threshold = capri_tm_get_xon_threshold(i);
        thresholds->threshold[i].xoff_threshold =
            capri_tm_get_xoff_threshold(i);
    }

    return ret;
}

sdk_ret_t
capri_populate_queue_stats (tm_port_t port,
                            capri_queue_t iqs[CAPRI_TM_MAX_IQS],
                            capri_queue_t oqs[CAPRI_TM_MAX_OQS],
                            capri_queue_stats_t *stats)
{
    sdk_ret_t first_err_ret = SDK_RET_OK;
    sdk_ret_t ret = SDK_RET_OK;

    for (unsigned i = 0; i < CAPRI_TM_MAX_IQS; i++) {
        if (iqs[i].valid) {
            ret = capri_tm_get_iq_stats(port, iqs[i].queue,
                                        &stats->iq_stats[i].stats);
            if (ret != SDK_RET_OK) {
                SDK_TRACE_ERR("Failed to get iq stats for port %d queue %d ret %d",
                              port, iqs[i].queue, ret);
                // Continue
                if (first_err_ret == SDK_RET_OK) {
                    first_err_ret = ret;
                }
            } else {
                stats->iq_stats[i].iq.valid = true;
                stats->iq_stats[i].iq.queue = iqs[i].queue;
            }
        }
    }

    for (unsigned i = 0; i < CAPRI_TM_MAX_OQS; i++) {
        if (oqs[i].valid) {
            ret = capri_tm_get_oq_stats(port, oqs[i].queue,
                                        &stats->oq_stats[i].stats);
            if (ret != SDK_RET_OK) {
                SDK_TRACE_ERR("Failed to get oq stats for port %d queue %d ret %d",
                              port, oqs[i].queue, ret);
                // Continue
                if (first_err_ret == SDK_RET_OK) {
                    first_err_ret = ret;
                }
            } else {
                stats->oq_stats[i].oq.valid = true;
                stats->oq_stats[i].oq.queue = oqs[i].queue;
            }
        }
    }

    return first_err_ret;
}

sdk_ret_t
capri_queue_stats_get (tm_port_t port,
                       capri_queue_stats_t *stats)
{
    capri_queue_t iqs[CAPRI_TM_MAX_IQS] = {};
    capri_queue_t oqs[CAPRI_TM_MAX_OQS] = {};
    uint32_t num_iqs = 0, num_oqs = 0;

    num_iqs = capri_tm_get_num_iqs_for_port(port);
    num_oqs = capri_tm_get_num_oqs_for_port(port);

    for (unsigned i = 0; i < num_iqs; i++) {
        iqs[i].valid = true;
        iqs[i].queue = i;
    }

    for (unsigned i = 0; i < num_oqs; i++) {
        oqs[i].valid = true;
        oqs[i].queue = i;
    }

    return capri_populate_queue_stats(port, iqs, oqs, stats);
}

sdk_ret_t
capri_queue_credits_get (queue_credits_get_cb_t cb,
                         void *ctxt)
{
    tm_port_t port;
    port_queue_credit_t credits = {0};
    uint32_t oq;

    for (port = CAPRI_TM_P4_PORT_BEGIN; port <= CAPRI_TM_P4_PORT_END; port++) {
        credits.num_queues = capri_tm_get_num_oqs_for_port(port);
        credits.queues = (sdk::asic::pd::queue_credit_t *)SDK_CALLOC(
                         SDK_MEM_ALLOC_INFRA,
                         sizeof(queue_credit_t) * credits.num_queues);
        for (oq = 0; oq < credits.num_queues; oq++) {
            credits.queues[oq].oq = oq;
            capri_tm_get_current_credits(port, oq, &credits.queues[oq].credit);
        }
        cb(port, &credits, ctxt);
        SDK_FREE(SDK_MEM_ALLOC_INFRA, credits.queues);
    }

    return SDK_RET_OK;
}

} // namespace capri
} // namespace platform
} // namespace sdk
