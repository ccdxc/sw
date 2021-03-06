//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "dpkt_learn.hpp"

#include "dpkt_learn_trans.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/fte/utils/packet_utils.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint_api.hpp"

using namespace hal::pd;

namespace hal {
namespace eplearn {

void sessions_empty_cb(const ep_t *ep);

static hal_ret_t
dpkt_learn_ip_move_handler(hal_handle_t ep_handle, const ip_addr_t *ip_addr) {
    return dpkt_learn_process_ip_move(ep_handle, ip_addr);
}

void data_pkt_learn_init() {
    register_sessions_empty_callback(sessions_empty_cb);
    register_ip_move_check_handler(dpkt_learn_ip_move_handler, DPKT_LEARN);
}


bool
dpkt_learn_required(fte::ctx_t &ctx) {

    ep_t *sep = ctx.sep();
    flow_key_t key = ctx.get_key();
    ip_addr_t ip_addr = {0};

    if (!ctx.flow_miss()) {
        return false;
    }

    /* If this IP already part of EP, no learning required */
    memcpy(&ip_addr.addr.v4_addr, &key.sip.v4_addr, sizeof(key.sip.v4_addr));
    if (ip_in_ep(&ip_addr, sep, NULL)) {
        return false;
    }

    return true;
}

hal_ret_t
dpkt_learn_process_packet(fte::ctx_t &ctx) {
    dpkt_learn_trans_key_t trans_key;
    dpkt_learn_trans_t *trans;
    dpkt_learn_event_data_t event_data;
    uint32_t event;
    ip_addr_t ip_addr = {0};
    flow_key_t key = ctx.get_key(FLOW_ROLE_INITIATOR);

    ep_t *sep = ctx.sep();

    memcpy(&(ip_addr.addr.v4_addr), &key.sip.v4_addr,
            sizeof(ip_addr.addr.v4_addr));
    dpkt_learn_trans_t::init_dpkt_learn_trans_key(sep->l2_key.mac_addr,
                                    sep,  &ip_addr, &trans_key);
    event = DPKT_LEARN_ADD;
    trans = dpkt_learn_trans_t::find_dpkt_learn_trans_by_id(trans_key);
    if (trans == NULL) {
        HAL_TRACE_INFO("Creating new Flow miss learn transaction {}", event);
        trans = new dpkt_learn_trans_t(&trans_key,  ctx);
    }
    event_data.ip_addr = ip_addr;
    event_data.fte_ctx = &ctx;
    event_data.in_fte_pipeline = true;
    HAL_TRACE_INFO("Processing Flow miss learn event {}", event);
    dpkt_learn_trans_t::process_learning_transaction(trans, ctx, event,
                                         (fsm_event_data)(&event_data));
    return HAL_RET_OK;
}

void
sessions_empty_cb(const ep_t *ep) {

    dllist_ctxt_t       *lnode = NULL;
    ep_ip_entry_t       *pi_ip_entry = NULL;
    dpkt_learn_trans_t *trans;
    ip_addr_t           ip_addr;
    trans_ip_entry_key_t ip_key;
    vrf_t *vrf = vrf_lookup_by_handle(ep->vrf_handle);


    if (vrf == NULL) {
        HAL_ABORT(0);
    }

    lnode = ep->ip_list_head.next;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode -
                offsetof(ep_ip_entry_t, ep_ip_lentry));
        ip_addr = {0};
        ip_addr.addr = pi_ip_entry->ip_addr.addr;
        dpkt_learn_trans_t::init_ip_entry_key(&(ip_addr),
                vrf->vrf_id, &ip_key);
        trans = reinterpret_cast<dpkt_learn_trans_t *>(
            dpkt_learn_trans_t::dpkt_learn_ip_entry_ht()->lookup(&ip_key));
        if (trans != nullptr) {
            dpkt_learn_trans_t::process_transaction(trans, DPKT_LEARN_SESSIONS_DELETED,
                                         NULL);
        }
    }
}



}  // namespace eplearn
}  // namespace hal
