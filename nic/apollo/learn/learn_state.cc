//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn global state
///
//----------------------------------------------------------------------------

#include <unistd.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "nic/apollo/learn/learn_uio.hpp"

namespace learn {

learn_state::learn_state() {

    // initialize to default values
    epoch_ = LEARN_API_EPOCH_START;
    vnic_objid_idxr_ = nullptr;
    learn_lif_ = nullptr;
    ep_mac_state_ = nullptr;
    ep_ip_state_ = nullptr;

    memset(&counters_, 0, sizeof(counters_));

    // default timeout/age values
    arp_probe_timeout_secs_ = LEARN_EP_ARP_PROBE_TIMEOUT_SEC;
    pkt_poll_interval_msecs_ = LEARN_PKT_POLL_INTERVAL_MSEC;
}

learn_state::~learn_state() {
    rte_indexer::destroy(vnic_objid_idxr_);
}

static int
learn_log (sdk_trace_level_e trace_level,
           const char *format, ...)
{
    // TODO: All DPDK logs to be redirected here and write logs to file
    return 0;
}

sdk_ret_t
learn_state::lif_init_(void) {
    sdk_ret_t ret;
    sdk_dpdk_params_t params;
    sdk_dpdk_device_params_t args;
    const char *lif_name = impl::learn_lif_name();
    int count = 0;

    SDK_ASSERT(lif_name);

    // wait for uio devices to come up
    while (!uio_device_ready()) {
        if (count >= UIO_DEV_SCAN_MAX_RETRY) {
            PDS_TRACE_ERR("UIO device not created, asserting!!");
            SDK_ASSERT(0);
        }
        if (0 == (count % 30)) {
            PDS_TRACE_INFO("UIO device not created yet, retry count %d", count);
        }
        sleep(UIO_DEV_SCAN_INTERVAL);
        count++;
    }
    PDS_TRACE_INFO("UIO device created, retry count %d", count);

    params.log_cb = learn_log;
    params.eal_init_list = "-n 4 --in-memory --file-prefix learn "
                           "--master-lcore 1 -c 3";
    params.log_name = "learn_dpdk";
    params.mbuf_pool_name = "learn_dpdk";
    params.mbuf_size = LEARN_LIF_PKT_BUF_SZ;
    params.num_mbuf = LEARN_LIF_MBUF_COUNT;
    params.vdev_list.push_back(string(lif_name));
    ret = dpdk_init(&params);
    if (unlikely(ret != SDK_RET_OK)) {
        // learn lif may not be present on all pipelines, so exit
        // on dpdk init failure.
        PDS_TRACE_ERR("DPDK init failed with return code %u", ret);
        return ret;
    }

    args.dev_name = lif_name;
    args.num_rx_desc = LEARN_LIF_RX_DESC_COUNT;
    args.num_rx_queue = 1;
    args.num_tx_desc = LEARN_LIF_TX_DESC_COUNT;
    args.num_tx_queue = 1;
    learn_lif_ = dpdk_device::factory(&args);

    return SDK_RET_OK;
}

sdk_ret_t
learn_state::init (void) {
    // instantiate MAC and IP states
    ep_mac_state_ = new ep_mac_state();
    ep_ip_state_ = new ep_ip_state();
    if (ep_mac_state_ == nullptr || ep_ip_state_ == nullptr) {
        return SDK_RET_OOM;
    }

    vnic_objid_idxr_ = rte_indexer::factory(PDS_MAX_VNIC, false, true);
    if (vnic_objid_idxr_ == nullptr) {
        return SDK_RET_ERR;
    }

    if (lif_init_() != SDK_RET_OK) {
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

uint32_t
learn_state::ep_timeout(void) const {
    uint32_t age;
    age = device_db()->find()->learn_age_timeout();
    if (age == 0) {
        age = LEARN_EP_DEFAULT_AGE_SEC;
    }
    return age;
}

sdk_ret_t
learn_state::walk(state_walk_cb_t walk_cb, void *user_data) {
    sdk_ret_t ret;

    ret = ep_mac_state_->walk(walk_cb, user_data);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ep_ip_state_->walk(walk_cb, user_data);
}

sdk_ret_t
learn_state::slab_walk(state_walk_cb_t walk_cb, void *user_data) {
    sdk_ret_t ret;

    ret = ep_mac_state_->slab_walk(walk_cb, user_data);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = ep_ip_state_->slab_walk(walk_cb, user_data);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return SDK_RET_OK;
}

}    // namepsace learn
