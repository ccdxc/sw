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
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/learn_thread.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "pkt_utils.hpp"

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
    // TODO: learn lif name should be pipeline independent, replace this with
    // pipeline specific check
    if (strlen(LEARN_LIF_NAME) == 0) {
        return false;
    }

    return true;
}

void
learn_thread_init_fn (void *ctxt)
{
    static event::timer_t pkt_poll_timer;
    static event::timer_t aging_timer;
    float pkt_poll_interval;

    // wait for uio devices to come up
    // TODO: get rid of sleep here
    sleep(30);

    // initalize learn state and dpdk_device
    SDK_ASSERT(learn_db()->init() == SDK_RET_OK);

    // metaswitch message handler
    sdk::ipc::reg_request_handler(LEARN_MSG_ID_MAPPING_API,
                                  learn_thread_ipc_cp_cb,
                                  NULL);

    // pkt poll timer handler
    pkt_poll_interval = learn_db()->pkt_poll_interval_msecs()/1000.;
    event::timer_init(&pkt_poll_timer, learn_thread_pkt_poll_timer_cb,
                      1., pkt_poll_interval);
    event::timer_start(&pkt_poll_timer);


   // aging timer
    event::timer_init(&aging_timer, learn_thread_aging_timer_cb,
                      1., (float) learn_db()->aging_tick());
    event::timer_start(&aging_timer);
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
learn_thread_send_pkt (void *mbuf, learn_nexthop_t nexthop)
{
    uint16_t tx_fail;
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;
    char *tx_hdr = nullptr;

    if (dpdk_device::remove_header(pkt, LEARN_P4_TO_ARM_HDR_SZ) != nullptr) {
        tx_hdr = dpdk_device::add_header(pkt, LEARN_ARM_TO_P4_HDR_SZ);
    }

    if (tx_hdr == nullptr) {
        PDS_TRACE_ERR("Unable to insert or remove p4 header, dropping the pkt");
        learn_thread_drop_pkt(mbuf, PKT_DROP_REASON_MBUF_ERR);
        return;
    }

    learn_p4_tx_hdr_fill(tx_hdr);

    tx_fail = learn_lif->transmit_packets(0, &pkt, 1);

    if (tx_fail) {
        PDS_TRACE_ERR("Failed to reinject learn packet");
        learn_thread_drop_pkt(mbuf, PKT_DROP_REASON_TX_FAIL);
        return;
    }
    LEARN_COUNTER_INCR(tx_pkts);
    PDS_TRACE_VERBOSE("Tx count %u", LEARN_COUNTER_GET(tx_pkts));
}

void
learn_thread_drop_pkt (void *mbuf, uint8_t reason)
{
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;
    learn_lif->drop_packets(&pkt, 1);
    LEARN_COUNTER_INCR(drop_pkts);
    SDK_ASSERT((reason < PKT_DROP_REASON_MAX));
    LEARN_COUNTER_INCR(drop_reason[reason]);
    PDS_TRACE_VERBOSE("Dropped count %u", LEARN_COUNTER_GET(drop_pkts));
}

void
learn_thread_pkt_poll_timer_cb (event::timer_t *timer)
{
    uint16_t rx_count = 0;
    dpdk_mbuf **rx_pkts;

    rx_pkts = learn_lif->receive_packets(0, DPDK_MAX_BURST_SIZE, &rx_count);
    if (unlikely(!rx_pkts || !rx_count)) {
        return;
    }

    PDS_TRACE_VERBOSE("Received %u packets on learn lif", rx_count);
    LEARN_COUNTER_ADD(rx_pkts, rx_count);

    for (uint16_t i = 0; i < rx_count; i++) {
        PDS_TRACE_DEBUG("Looping back packet to p4");
        learn_thread_send_pkt((void *)rx_pkts[i], LEARN_PKT_NEXTHOP_NONE);
    }
}

void
learn_thread_api_async_cb_local (sdk_ret_t ret, const void *cookie)
{
    return;
}

void
learn_thread_api_async_cb_remote (sdk_ret_t ret, const void *cookie)
{
    return;
}


void
learn_thread_ipc_cp_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    return;
}


void
learn_thread_aging_timer_cb (event::timer_t *timer)
{
    return;
}

void
learn_thread_api_async_cb_aging (sdk_ret_t ret, const void *cookie)
{
    return;
}

}    // namespace learn
