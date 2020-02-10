//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn global state
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "pkt_utils.hpp"

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
    aging_secs_per_tick_ = LEARN_EP_AGING_TICK_SEC;
    ep_timeout_secs_ = LEARN_EP_DEFAULT_AGE_SEC;
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

    params.log_cb = learn_log;
    params.eal_init_list = "-n 4 --in-memory --file-prefix learn "
                           "--master-lcore 1 -c 3";
    params.log_name = "learn_dpdk";
    params.mbuf_pool_name = "learn_dpdk";
    params.mbuf_size = 2048;
    params.num_mbuf = 1024;
    params.vdev_list.push_back(string(LEARN_LIF_NAME));
    ret = dpdk_init(&params);
    if (unlikely(ret != SDK_RET_OK)) {
        // learn lif may not be present on all pipelines, so exit
        // on dpdk init failure.
        PDS_TRACE_ERR("DPDK init failed with return code %u", ret);
        return ret;
    }

    args.dev_name = LEARN_LIF_NAME;
    args.num_rx_desc = 1024;
    args.num_rx_queue = 1;
    args.num_tx_desc = 1024;
    args.num_tx_queue = 1;
    learn_lif_ = dpdk_device::factory(&args);

    return SDK_RET_OK;
}

sdk_ret_t
learn_state::init (void) {
    // instantiate mac and IP states
    ep_mac_state_ = new ep_mac_state();
    ep_ip_state_ = new ep_ip_state();
    if (ep_mac_state_ == nullptr || ep_ip_state_ == nullptr) {
        return SDK_RET_OOM;
    }

    vnic_objid_idxr_ = rte_indexer::factory(PDS_MAX_VNIC, false, true);
    if (vnic_objid_idxr_ == nullptr) {
        return SDK_RET_ERR;
    }

    // TODO: get aging time from device spec

    if (lif_init_() != SDK_RET_OK) {
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
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
