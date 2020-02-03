//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "arp_trans.hpp"
#include "arp_learn.hpp"
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

static hal_ret_t get_arp_status(vrf_id_t vrf_id, ip_addr_t *ip_addr, ArpStatus *arp_status);
static hal_ret_t do_proxy_arp_processing(struct ether_arp *arphead,
                        fte::ctx_t &ctx);
static hal_ret_t
arp_ip_move_handler(hal_handle_t ep_handle, const ip_addr_t *ip_addr) {

    return arp_process_ip_move(ep_handle, ip_addr);
}

void arp_init() {
    register_arp_ep_status_callback(get_arp_status);
    register_ip_move_check_handler(arp_ip_move_handler, ARP_LEARN);
}

bool is_arp_flow(const hal::flow_key_t *key) {
    return (key->flow_type == hal::FLOW_TYPE_L2 &&
            key->ether_type == ETH_TYPE_ARP);
}

bool is_rarp_flow(const hal::flow_key_t *key) {
    return (key->flow_type == hal::FLOW_TYPE_L2 &&
            key->ether_type == ETH_TYPE_RARP);
}

static hal_ret_t arp_process_req_entry(uint16_t opcode, uint8_t *hw_address,
        uint8_t *protocol_address, fte::ctx_t &ctx) {

    arp_trans_key_t trans_key;
    arp_trans_t *trans;
    arp_event_data_t event_data;
    uint32_t event;
    ip_addr_t ip_addr = {0};

    memcpy(&(ip_addr.addr.v4_addr), protocol_address,
            sizeof(ip_addr.addr.v4_addr));
    ip_addr.addr.v4_addr = ntohl(ip_addr.addr.v4_addr);
    arp_trans_t::init_arp_trans_key(hw_address,
                                    ctx.sep(), ARP_TRANS_IPV4, &ip_addr,
                                    &trans_key);
    if (opcode == ARPOP_REVREQUEST) {
        event = RARP_REQ;
    } else {
        event = ARP_ADD;
    }
    trans = arp_trans_t::find_arptrans_by_id(trans_key);
    if (trans == NULL) {
        HAL_TRACE_INFO("Creating new ARP transaction {}", event);
        trans = new arp_trans_t(&trans_key, ctx);
    }
    event_data.ip_addr = ip_addr;
    event_data.fte_ctx = &ctx;
    event_data.in_fte_pipeline = true;
    HAL_TRACE_INFO("Processing ARP event {}", event);
    arp_trans_t::process_learning_transaction(trans, ctx, event,
                                         (fsm_event_data)(&event_data));
    return HAL_RET_OK;
}


static hal_ret_t arp_process_entry(struct ether_arp *arphead,
                                   fte::ctx_t &ctx) {
    arp_trans_key_t trans_key;
    arp_event_data_t event_data;
    arp_trans_t *trans;
    arp_fsm_event_t event;
    uint16_t opcode = ntohs(arphead->ea_hdr.ar_op);
    ip_addr_t ip_addr = {0};

    arp_process_req_entry(opcode,
            arphead->arp_sha, arphead->arp_spa, ctx);

    if (opcode == ARPOP_REVREPLY) {
        memcpy(&(ip_addr.addr.v4_addr), arphead->arp_tpa,
                sizeof(ip_addr.addr.v4_addr));
        arp_trans_t::init_arp_trans_key(arphead->arp_tha,
                                        ctx.dep(), ARP_TRANS_IPV4,
                                        &ip_addr,
                                        &trans_key);
        trans = arp_trans_t::find_arptrans_by_id(trans_key);
        if (trans == nullptr) {
            /* Did not see a RARP request, but seeing an RARP response */
            HAL_TRACE_ERR("RARP request missing, ignoring RARP response.");
            return HAL_RET_OK;
        }
        event = RARP_REPLY;
        memcpy(&(ip_addr.addr.v4_addr), arphead->arp_tpa,
                sizeof(ip_addr.addr.v4_addr));
        event_data.ip_addr = ip_addr;
        event_data.fte_ctx = &ctx;
        HAL_TRACE_INFO("Processing ARP event {}", event);
        arp_trans_t::process_learning_transaction(trans, ctx, event,
                                         (fsm_event_data)(&event_data));
    }

    return HAL_RET_OK;
}

static hal_ret_t validate_arp_packet(const struct ether_arp *arphead,
                                     fte::ctx_t &ctx) {
    hal::ep_t *sep = ctx.sep();
    uint16_t opcode;
    unsigned char spa[sizeof(arphead->arp_spa)];

    opcode = ntohs(arphead->ea_hdr.ar_op);

    if ((opcode != ARPOP_REQUEST) && (opcode != ARPOP_REPLY) &&
            (opcode != ARPOP_REVREQUEST) && (opcode != ARPOP_REVREPLY)) {
        HAL_TRACE_ERR(
            "Invalid ARP Request type {} received from EP"
            "with hardware address",
             opcode, macaddr2str(sep->l2_key.mac_addr));
        return HAL_RET_ERR;
    }

    memset(spa, 0, sizeof(arphead->arp_spa));
    if ((opcode != ARPOP_REVREQUEST) && (memcmp(arphead->arp_spa, spa, sizeof(spa)) == 0)) {
        HAL_TRACE_ERR(
            "Ignoring ARP Request type {} received from EP with protocol address 0 "
            "with hardware address",
             opcode, macaddr2str(sep->l2_key.mac_addr));
        return HAL_RET_ERR;
    }

    /* Checking ARP spoofing */
    if (memcmp(sep->l2_key.mac_addr, arphead->arp_sha, ETHER_ADDR_LEN)) {
        HAL_TRACE_ERR(
            "Hardware address {} does not match the endpoint"
            " hardware address {}",
            macaddr2str(arphead->arp_sha), macaddr2str(sep->l2_key.mac_addr));
        return HAL_RET_ERR;
    }

    /* Make sure RARP request Target Hardware address also match */
    if ((opcode == ARPOP_REVREQUEST) &&
            (memcmp(sep->l2_key.mac_addr, arphead->arp_tha, ETHER_ADDR_LEN))) {
        HAL_TRACE_ERR(
            "Target Hardware address {} does not match the endpoint"
            " hardware address in RARP request {}",
            macaddr2str(arphead->arp_tha),
            macaddr2str(sep->l2_key.mac_addr));
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

bool
process_vmotion_rarp(fte::ctx_t *ctx) {
    const unsigned char *ether_pkt = ctx->pkt();
    struct ether_arp *arphead;
    hal_ret_t ret = HAL_RET_OK;
    uint16_t opcode;

    if (ether_pkt == nullptr) {
        /* Skipping as not packet to process */
        HAL_TRACE_ERR("ARP: ether_pkt -> NULL");
        return false;
    }

    if (ctx->vlan_valid()) {
        arphead = (struct ether_arp *)(ether_pkt + sizeof(vlan_header_t));
    } else {
        arphead = (struct ether_arp *)(ether_pkt + L2_ETH_HDR_LEN);
    }

    opcode = ntohs(arphead->ea_hdr.ar_op);
    ret = validate_arp_packet(arphead, *ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ARP: RARP type {} , SRC {} validation failed",
                    opcode, macaddr2str(arphead->arp_sha));
        return false;
    }
    HAL_TRACE_INFO("ARP: Processing RARP type {} , SRC {} ",
                    opcode, macaddr2str(arphead->arp_sha));

    if (opcode == ARPOP_REVREQUEST) {
       if (hal::g_hal_state->get_vmotion()->process_rarp(arphead->arp_sha))
           ctx->set_drop();
    }
    return true;
}

hal_ret_t
arp_process_packet(fte::ctx_t &ctx) {
    const unsigned char *ether_pkt = ctx.pkt();
    struct ether_arp *arphead;
    hal_ret_t ret = HAL_RET_OK;
    uint16_t opcode;

    if (ether_pkt == nullptr) {
        /* Skipping as not packet to process */
        goto out;
    }

    if (ctx.vlan_valid()) {
        arphead = (struct ether_arp *)(ether_pkt + sizeof(vlan_header_t));
    } else {
        arphead = (struct ether_arp *)(ether_pkt + L2_ETH_HDR_LEN);
    }

    ret = validate_arp_packet(arphead, ctx);
    if (ret != HAL_RET_OK) {
        goto out;
     }
    opcode = ntohs(arphead->ea_hdr.ar_op);
    HAL_TRACE_INFO("ARP: Processing ARP type {} , SRC {} ",
                    opcode, macaddr2str(arphead->arp_sha));


    ret = arp_process_entry(arphead, ctx);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ARP: Processing ARP type {} , SRC {} Failed",
                    opcode, macaddr2str(arphead->arp_sha));
    }

    do_proxy_arp_processing(arphead, ctx);

out:
    return ret;
}

static hal_ret_t
get_arp_status(vrf_id_t vrf_id, ip_addr_t *ip_addr, ArpStatus *arp_status)
{
    trans_ip_entry_key_t ip_entry_key;
    arp_trans_t *arp_trans;
    hal_ret_t ret = HAL_RET_OK;

    trans_t::init_ip_entry_key(ip_addr, vrf_id, &ip_entry_key);

    arp_trans = hal::eplearn::arp_trans_t::find_arp_trans_by_key(&ip_entry_key);
    if (arp_trans == nullptr) {
        arp_status->set_entry_active(false);
        ret = HAL_RET_ENTRY_NOT_FOUND;
        goto out;
    }

    arp_status->set_entry_active(true);
    arp_status->set_entry_timeout(arp_trans->get_timeout_remaining());

out:
    return ret;
}


struct arp_proxy_reply_ctx_t {
    ep_t          *ep;
    hal_ret_t     ret;
    ip_addr_t     src_ip;
    ip_addr_t     dst_ip;
    mac_addr_t    src_mac;
};


static void
send_proxy_arp_reply_request(void *data) {
    arp_proxy_reply_ctx_t *arp_ctx = reinterpret_cast<arp_proxy_reply_ctx_t*>(data);
    hal::l2seg_t   *l2seg;
    ep_t*           ep = arp_ctx->ep;
    fte::utils::arp_pkt_data_t pkt_data = { 0 };

    arp_ctx->ret = HAL_RET_OK;

    l2seg = l2seg_lookup_by_handle(ep->l2seg_handle);
    if (l2seg != nullptr && l2seg->eplearn_cfg.arp_cfg.enabled &&
            l2seg->eplearn_cfg.arp_cfg.probe_enabled) {
        pkt_data.ep = ep;
        pkt_data.src_ip_addr = &arp_ctx->src_ip;
        pkt_data.src_mac = &arp_ctx->src_mac;
        pkt_data.dst_ip_addr = &arp_ctx->dst_ip;
        arp_ctx->ret = fte::utils::hal_inject_arp_request_pkt(&pkt_data);
    }
}

static hal_ret_t
do_proxy_arp_processing(struct ether_arp *arphead,
                        fte::ctx_t &ctx) {
    hal_ret_t ret = HAL_RET_NOOP;
    hal::l2seg_t *l2seg  = ctx.sl2seg();
    uint16_t opcode = ntohs(arphead->ea_hdr.ar_op);
    ip_addr_t src_ip = {0};
    ip_addr_t dst_ip = {0};
    ep_t *dep = nullptr;
    hal_handle_t ep_handle = ctx.sep_handle();
    ep_t *sep_entry;
    struct arp_proxy_reply_ctx_t *fn_ctx;

    sep_entry = find_ep_by_handle(ep_handle);
    if (sep_entry == nullptr) {
        return HAL_RET_EP_NOT_FOUND;
    }

    if (opcode != ARPOP_REQUEST ||
            l2seg == nullptr || !l2seg->proxy_arp_enabled) {
        HAL_TRACE_INFO("Proxy ARP not applicable for this packet.");
        goto out;
    }

    memcpy(&(dst_ip.addr.v4_addr), arphead->arp_tpa,
                sizeof(dst_ip.addr.v4_addr));
    dst_ip.addr.v4_addr = ntohl(dst_ip.addr.v4_addr);
    dep = find_ep_by_v4_key_in_l2segment(dst_ip.addr.v4_addr, l2seg);
    if (!dep || !is_ep_remote(dep)) {
        HAL_TRACE_INFO("Skipping Proxy as Destination EP not found or not remote.");
        goto out;
    }

    fn_ctx = (struct arp_proxy_reply_ctx_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE,
                    sizeof(struct arp_proxy_reply_ctx_t));

    memcpy(&(src_ip.addr.v4_addr), arphead->arp_spa,
                sizeof(src_ip.addr.v4_addr));
    src_ip.addr.v4_addr = ntohl(src_ip.addr.v4_addr);

    memcpy(fn_ctx->src_mac,  dep->l2_key.mac_addr, sizeof(mac_addr_t));
    fn_ctx->src_ip = dst_ip;
    fn_ctx->dst_ip = src_ip;
    fn_ctx->ep = sep_entry;

    fte::fte_execute(0, send_proxy_arp_reply_request, fn_ctx);
    HAL_FREE(hal::HAL_MEM_ALLOC_FTE, fn_ctx);

    ret = HAL_RET_OK;
out:
    return ret;
}

}  // namespace eplearn
}  // namespace hal
