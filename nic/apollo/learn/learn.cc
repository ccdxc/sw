//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Learn core functionality
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include <vector>
#include <unistd.h>

using namespace std;

namespace learn {

// TODO: Channges this to correct lif once we define one for learn
#define LEARN_LIF_NAME "net_ionic0"

static int
learn_log (sdk_trace_level_e trace_level,
           const char *format, ...)
{
    // TODO: All DPDK logs to be redirected here and write logs to file
    return 0;
}

void *
learn_thread_start (void *ctxt)
{
    sdk_dpdk_params_t params;
    sdk_ret_t ret;
    sdk_dpdk_device_params_t args;
    dpdk_device *learn_lif = NULL;
    uint16_t recv_count = 0;
    dpdk_mbuf **packets = NULL;

    SDK_THREAD_INIT(ctxt);

    params.log_cb = learn_log;
    params.eal_init_list = "-c 3 -n 4 --in-memory --file-prefix learn --master-lcore 1";
    params.log_name = "learn_dpdk";
    params.mbuf_pool_name = "learn_dpdk";
    params.mbuf_size = 512;
    params.num_mbuf = 1024;
    params.vdev_list.push_back(string(LEARN_LIF_NAME));
    ret = dpdk_init(&params);
    SDK_ASSERT(ret == SDK_RET_OK);

    args.dev_name = LEARN_LIF_NAME;
    args.num_rx_desc = 1024;
    args.num_rx_queue = 1;
    args.num_tx_desc = 1024;
    args.num_tx_queue = 1;
    learn_lif = dpdk_device::factory(&args);
    SDK_ASSERT(learn_lif);

    while (1) {
        recv_count = 0;
        packets = learn_lif->recieve_packets(0, 256, &recv_count);
        if (packets && recv_count) {
            PDS_TRACE_DEBUG("Received %u packets", recv_count);
        }
        // Lazy poll
        usleep(100000);
    }
    return NULL;
}

}    // namespace learn
