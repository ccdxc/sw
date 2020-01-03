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
#include <learn_utils.hpp>
#include <vector>
#include <unistd.h>
#include <string.h>

using namespace std;

namespace learn {

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

    // TODO: though we start this thread after is_nicmgr_ready(), still uio
    // devices are not created by the time we reach here. So this hack
    // for now. We need to come back and fix this later.
    sleep(30);
    params.log_cb = learn_log;
    params.eal_init_list = "-c 3 -n 4 --in-memory --file-prefix learn --master-lcore 1";
    params.log_name = "learn_dpdk";
    params.mbuf_pool_name = "learn_dpdk";
    params.mbuf_size = 512;
    params.num_mbuf = 1024;
    params.vdev_list.push_back(string(LEARN_LIF_NAME));
    ret = dpdk_init(&params);
    if (ret != SDK_RET_OK) {
        // learn lif may not be present on all pipelines, so exit
        // on dpdk init failure.
        PDS_TRACE_ERR("DPDK init failed! Exit learn thread!");
        goto end;
    }

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
end:
    return NULL;
}

}    // namespace learn
