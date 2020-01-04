//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Learn core functionality
///
//----------------------------------------------------------------------------

#include <string.h>
#include <unistd.h>
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "learn_utils.hpp"

using namespace std;

namespace learn {

dpdk_device *g_learn_lif = NULL;
dpdk_mbuf *tx_packets[DPDK_MAX_BURST_SIZE];

bool
learn_thread_enabled (void)
{
    // there is no other way to get if we are running gtest without model.
    // if we start learn thread in gtest, DPDK will assert as model is not
    // running, so disable learn thread while running under gtest
    if (getenv("CAPRI_MOCK_MODE")) {
        return false;
    }

    if (0 != strlen(LEARN_LIF_NAME)) {
        return true;
    }
    return false;
}

static int
learn_log (sdk_trace_level_e trace_level,
           const char *format, ...)
{
    // TODO: All DPDK logs to be redirected here and write logs to file
    return 0;
}

static inline sdk_ret_t
learn_p4_rx_hdr_remove (dpdk_mbuf *pkt)
{
    char *hdr;

    hdr = dpdk_device::remove_header(pkt, LEARN_P4_TO_ARM_HDR_SZ);
    if (unlikely(hdr == NULL)) {
        return SDK_RET_OOB;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
learn_p4_tx_hdr_add (dpdk_mbuf *pkt)
{
    char *tx_hdr;

    tx_hdr = dpdk_device::add_header(pkt, LEARN_ARM_TO_P4_HDR_SZ);
    if (unlikely(tx_hdr == NULL)) {
        return SDK_RET_OOM;
    }
    learn_p4_tx_hdr_fill(tx_hdr);
    return SDK_RET_OK;
}

sdk_ret_t
learn_pkt_headers_process (dpdk_mbuf *pkt)
{
    sdk_ret_t ret;

    // TODO: plug in learn code, which learns info from pkt

    ret = learn_p4_rx_hdr_remove(pkt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("learn thread : Failed to remove rx header");
        return ret;
    }

    // TODO: pass learn info to add_tx_header for populating lif
    ret = learn_p4_tx_hdr_add(pkt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("learn thread : Failed to add tx header");
        return ret;
    }

    return SDK_RET_OK;
}

void
learn_packets_process (dpdk_mbuf **rx_pkts, uint16_t rx_count)
{
    sdk_ret_t ret;
    dpdk_mbuf *rx_pkt;
    uint16_t tx_count = 0, tx_fail;

    PDS_TRACE_DEBUG("learn thread received %u packets", rx_count);
    for (uint16_t i = 0; i < rx_count; i++) {
        rx_pkt = rx_pkts[i];
        ret = learn_pkt_headers_process(rx_pkt);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("learn thread : Failed to process learn pkt");
            // TODO: Drop packets?
            continue;
        }
        // queue pkt to send
        tx_packets[tx_count++] = rx_pkt;
    }
    tx_fail = g_learn_lif->transmit_packets(0, tx_packets, tx_count);
    PDS_TRACE_DEBUG("learn thread re-injected %u packets", tx_count-tx_fail);
}

void
learn_pkt_poll_timer_cb (void *timer)
{
    uint16_t rx_count = 0;
    dpdk_mbuf **rx_pkts;

    rx_pkts = g_learn_lif->receive_packets(0, DPDK_MAX_BURST_SIZE, &rx_count);
    if (unlikely(!rx_pkts || !rx_count)) {
        return;
    }

    learn_packets_process(rx_pkts, rx_count);
}

sdk_ret_t
learn_init (void)
{
    sdk_ret_t ret;
    sdk_dpdk_params_t params;
    sdk_dpdk_device_params_t args;

    params.log_cb = learn_log;
    params.eal_init_list = "-c 3 -n 4 --in-memory --file-prefix learn --master-lcore 1";
    params.log_name = "learn_dpdk";
    params.mbuf_pool_name = "learn_dpdk";
    params.mbuf_size = 512;
    params.num_mbuf = 1024;
    params.vdev_list.push_back(string(LEARN_LIF_NAME));
    ret = dpdk_init(&params);
    if (unlikely(ret != SDK_RET_OK)) {
        // learn lif may not be present on all pipelines, so exit
        // on dpdk init failure.
        PDS_TRACE_ERR("DPDK init failed! learn thread exiting!");
        return ret;
    }

    args.dev_name = LEARN_LIF_NAME;
    args.num_rx_desc = 1024;
    args.num_rx_queue = 1;
    args.num_tx_desc = 1024;
    args.num_tx_queue = 1;
    g_learn_lif = dpdk_device::factory(&args);
    SDK_ASSERT(g_learn_lif);

    return SDK_RET_OK;
}

void *
learn_thread_start (void *ctxt)
{
    sdk_ret_t ret;

    SDK_THREAD_INIT(ctxt);

    // TODO: though we start this thread after is_nicmgr_ready(), still uio
    // devices are not created by the time we reach here. So this hack
    // for now. We need to come back and fix this later.
    sleep(30);

    ret = learn_init();
    if (unlikely(ret != SDK_RET_OK)) {
        goto end;
    }

    PDS_TRACE_DEBUG("learn thread entering packet loop");
    while (1) {
        learn_pkt_poll_timer_cb(NULL);
        // Lazy poll
        usleep(100000);
    }
end:
    return NULL;
}

}    // namespace learn
