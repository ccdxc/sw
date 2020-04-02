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
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/learn_thread.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/ep_utils.hpp"

namespace learn {

typedef struct ep_mac_clear_args_s {
    pds_obj_key_t subnet;
    sdk_ret_t retcode;
} ep_mac_clear_args_t;

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
    if (getenv("ASIC_MOCK_MODE")) {
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

    rx_pkts = learn_lif->receive_packets(0, LEARN_LIF_RECV_BURST_SZ, &rx_count);
    if (unlikely(!rx_pkts || !rx_count)) {
        return;
    }
    LEARN_COUNTER_ADD(rx_pkts, rx_count);

    for (uint16_t i = 0; i < rx_count; i++) {
        process_learn_pkt((void *)rx_pkts[i]);
    }
}

static bool
ep_ip_entry_clear_cb (void *entry, void *retcode)
{
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;
    sdk_ret_t *ret = (sdk_ret_t *)retcode;

    *ret = ip_ageout(ip_entry);
    if (*ret != SDK_RET_OK) {
        return true; //stop iterating
    }
    return false;
}

static bool
ep_mac_entry_clear_cb (void *entry, void *ctxt)
{
    ep_mac_entry *mac_entry = (ep_mac_entry *)entry;
    ep_mac_clear_args_t *args = (ep_mac_clear_args_t *)ctxt;

    if ((args->subnet != k_pds_obj_key_invalid)
        && (args->subnet != mac_entry->key()->subnet)) {
        // mac belongs to a different subnet, skip this
        return false;
    }
    mac_entry->walk_ip_list(ep_ip_entry_clear_cb, &args->retcode);
    if (args->retcode == SDK_RET_OK) {
        args->retcode = mac_ageout(mac_entry);
    }
    if (args->retcode != SDK_RET_OK) {
        return true; //stop iterating
    }
    return false;
}

sdk_ret_t
process_subnet_delete (pds_obj_key_t key)
{
    ep_mac_clear_args_t args;

    if (key == k_pds_obj_key_invalid) {
        return SDK_RET_INVALID_ARG;
    }
    args.subnet = key;
    args.retcode = SDK_RET_OK;
    PDS_TRACE_DEBUG("Deleting all learnt MAC and IP entries for subnet %s",
                    key.str());
    learn_db()->ep_mac_db()->walk(ep_mac_entry_clear_cb, &args);
    if (args.retcode != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to clear entries related to subnet %s",
                      key.str());
        return args.retcode;
    }
    // sync call to delete subnet
    PDS_TRACE_DEBUG("Deleting subnet %s", key.str());
    return pds_subnet_delete(&key, PDS_BATCH_CTXT_INVALID);
}

void
learn_thread_ipc_api_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    sdk_ret_t ret;
    api::api_msg_t *api_msg  = (api::api_msg_t *)msg->data();
    api_ctxt_t *api_ctxt;

    PDS_TRACE_DEBUG("Rcvd API batch message");
    SDK_ASSERT(api_msg != nullptr);
    SDK_ASSERT(api_msg->msg_id == api::API_MSG_ID_BATCH);

    // TODO: this will be cleaned up once we have framework changes
    // to notify learn thread of subnet deletion.
    if (api_msg->batch.apis.size() == 1) {
        api_ctxt = *(api_msg->batch.apis.begin());
        if (api_ctxt->obj_id == OBJ_ID_SUBNET &&
            api_ctxt->api_op == API_OP_DELETE) {
            ret = process_subnet_delete(api_ctxt->api_params->key);
            sdk::ipc::respond(msg, &ret, sizeof(ret));
            return;
        }
    }

    ret = process_api_batch(api_msg);
    sdk::ipc::respond(msg, &ret, sizeof(ret));
}

void
learn_thread_ipc_clear_cmd_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    sdk_ret_t         ret = SDK_RET_OK;
    ep_mac_entry      *mac_entry;
    ep_ip_entry       *ip_entry;
    learn_clear_msg_t *req = (learn_clear_msg_t *)msg->data();
    ep_mac_key_t      *mac_key = &req->mac_key;
    ep_ip_key_t       *ip_key = &req->ip_key;

    switch(req->id) {
    case LEARN_CLEAR_MAC:
        PDS_TRACE_INFO("Received clear request for MAC %s/%s",
                       macaddr2str(mac_key->mac_addr),
                       mac_key->subnet.str());
        mac_entry = (ep_mac_entry *) learn_db()->ep_mac_db()->find(mac_key);
        if (mac_entry) {
            // clear all IPs linked to this MAC first
            mac_entry->walk_ip_list(ep_ip_entry_clear_cb, &ret);
            if (ret == SDK_RET_OK) {
                ret = mac_ageout(mac_entry);
            }
        } else {
            ret = SDK_RET_ENTRY_NOT_FOUND;
        }
        break;
    case LEARN_CLEAR_MAC_ALL:
        PDS_TRACE_INFO("received request to clear all MAC");
        ep_mac_clear_args_t args;
        args.subnet = k_pds_obj_key_invalid;
        args.retcode = SDK_RET_OK;
        learn_db()->ep_mac_db()->walk(ep_mac_entry_clear_cb, &args);
        ret = args.retcode;
        break;
    case LEARN_CLEAR_IP:
        PDS_TRACE_INFO("Received clear request for IP %s/%s",
                       ipaddr2str(&ip_key->ip_addr), ip_key->vpc.str());
        ip_entry = (ep_ip_entry *)learn_db()->ep_ip_db()->find(ip_key);
        if (ip_entry) {
            ret = ip_ageout(ip_entry);
        } else {
            ret = SDK_RET_ENTRY_NOT_FOUND;
        }
        break;
    case LEARN_CLEAR_IP_ALL:
        PDS_TRACE_INFO("Received request to clear all IP");
        learn_db()->ep_ip_db()->walk(ep_ip_entry_clear_cb, &ret);
        break;
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    sdk::ipc::respond(msg, &ret, sizeof(ret));
}

void
learn_thread_init_fn (void *ctxt)
{
    static event::timer_t pkt_poll_timer;
    float pkt_poll_interval;

    // initalize learn state and dpdk_device
    SDK_ASSERT(learn_db()->init() == SDK_RET_OK);

    // control plane message handler
    sdk::ipc::reg_request_handler(LEARN_MSG_ID_API, learn_thread_ipc_api_cb,
                                  NULL);
    sdk::ipc::reg_request_handler(LEARN_MSG_ID_CLEAR_CMD,
                                  learn_thread_ipc_clear_cmd_cb,
                                  NULL);

    // pkt poll timer handler
    pkt_poll_interval = learn_db()->pkt_poll_interval_msecs()/1000.;
    event::timer_init(&pkt_poll_timer, learn_thread_pkt_poll_timer_cb,
                      1., pkt_poll_interval);
    event::timer_start(&pkt_poll_timer);
}

}    // namespace learn
