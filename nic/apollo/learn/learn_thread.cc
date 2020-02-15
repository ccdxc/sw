//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn thread implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/ep_learn_local.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/learn_thread.hpp"

namespace learn {

bool
learning_enabled (void)
{

    // first check if learning is enabled in the device config
    // TODO: device cfg is not available, defer the check to later
#if 0
    device_entry *device = device_db()->find();
    if (device == nullptr) {
        PDS_TRACE_ERR("Unable to read device state");
        return false;
    }
    if (!device->learning_enabled()) {
        return false;
    }
#endif

    // there is no other way to get if we are running gtest without model.
    // if we start learn thread in gtest, DPDK will assert as model is not
    // running, so disable learn thread while running under gtest
    // TODO: remove this after making sure gtest setup disables learning
    if (getenv("CAPRI_MOCK_MODE")) {
        return false;
    }

    // check if pipeline defines learn lif
    if (!impl::learn_lif_name()) {
        return false;
    }
    return true;
}

void
learn_thread_exit_fn (void *ctxt)
{
}

void
learn_thread_event_cb (void *msg, void *ctxt)
{
}

#define learn_lif learn_db()->learn_lif()
void
learn_thread_pkt_poll_timer_cb (event::timer_t *timer)
{
    uint16_t rx_count = 0;
    dpdk_mbuf **rx_pkts;

    rx_pkts = learn_lif->receive_packets(0, DPDK_MAX_BURST_SIZE, &rx_count);
    if (unlikely(!rx_pkts || !rx_count)) {
        return;
    }

    PDS_TRACE_VERBOSE("received %u packets on learn lif", rx_count);
    LEARN_COUNTER_ADD(rx_pkts, rx_count);

    for (uint16_t i = 0; i < rx_count; i++) {
        process_learn_pkt((void *)rx_pkts[i]);
    }
}

void
learn_thread_ipc_cp_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    return;
}

void
learn_thread_init_fn (void *ctxt)
{
    static event::timer_t pkt_poll_timer;
    float pkt_poll_interval;

    // wait for uio devices to come up
    // TODO: get rid of sleep here
    sleep(30);

    // initalize learn state and dpdk_device
    SDK_ASSERT(learn_db()->init() == SDK_RET_OK);

    // control plane message handler
    sdk::ipc::reg_request_handler(LEARN_MSG_ID_MAPPING_API,
                                  learn_thread_ipc_cp_cb,
                                  NULL);

    // pkt poll timer handler
    pkt_poll_interval = learn_db()->pkt_poll_interval_msecs()/1000.;
    event::timer_init(&pkt_poll_timer, learn_thread_pkt_poll_timer_cb,
                      1., pkt_poll_interval);
    event::timer_start(&pkt_poll_timer);
}

}    // namespace learn
