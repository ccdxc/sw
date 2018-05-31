//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "arp_trans.hpp"
#include "arp_learn.hpp"
#include "nic/include/eth.h"
#include "nic/fte/utils/packet_utils.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/endpoint_api.hpp"

using namespace hal::pd;

namespace hal {
namespace eplearn {

static hal_ret_t get_arp_status(vrf_id_t vrf_id, ip_addr_t *ip_addr, ArpStatus *arp_status);
static hal_ret_t do_proxy_arp_processing(struct ether_arp *arphead,
                        fte::ctx_t &ctx);

void arp_init() {
    register_arp_ep_status_callback(get_arp_status);
}

bool is_arp_flow(const hal::flow_key_t *key) {
    return (key->flow_type == hal::FLOW_TYPE_L2 &&
            key->ether_type == ETH_TYPE_ARP);
}

static hal_ret_t arp_process_req_entry(uint16_t opcode, uint8_t *hw_address,
        uint8_t *protocol_address, fte::ctx_t &ctx) {

    arp_trans_key_t trans_key;
    arp_trans_t *trans;
    arp_event_data_t event_data;
    uint32_t event;
    ip_addr_t ip_addr = {0};

    arp_trans_t::init_arp_trans_key(hw_address,
                                    ctx.sep(), ARP_TRANS_IPV4, &trans_key);
    if (opcode == ARPOP_REVREQUEST) {
        event = RARP_REQ;
    } else {
        event = ARP_ADD;
    }
    trans = arp_trans_t::find_arptrans_by_id(trans_key);
    if (trans == NULL) {
        HAL_TRACE_INFO("Creating new ARP transaction {}", event);
        trans = new arp_trans_t(hw_address, ARP_TRANS_IPV4, ctx);
    }
    memcpy(&(ip_addr.addr.v4_addr), protocol_address,
            sizeof(ip_addr.addr.v4_addr));
    ip_addr.addr.v4_addr = ntohl(ip_addr.addr.v4_addr);
    event_data.ip_addr = ip_addr;
    event_data.fte_ctx = &ctx;
    event_data.in_fte_pipeline = true;
    HAL_TRACE_INFO("Processing ARP event {}", event);
    arp_trans_t::process_transaction(trans, event,
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
        arp_trans_t::init_arp_trans_key(arphead->arp_tha,
                                        ctx.dep(), ARP_TRANS_IPV4, &trans_key);
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
        arp_trans_t::process_transaction(trans, event,
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
    if (memcmp(arphead->arp_spa, spa, sizeof(spa)) == 0) {
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
    hal_ret_t ret;

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


static void
free_arp_proxy_reply_pkt (uint8_t *pkt)
{
    HAL_FREE(hal::HAL_MEM_ALLOC_SFW, pkt);
}

static hal_ret_t
proxy_arp_build_cpu_p4_plus_header(fte::ctx_t &ctx,
        cpu_to_p4plus_header_t &send_cpu_hdr,
        p4plus_to_p4_header_t &p4plus_hdr)
{
    hal_ret_t                            ret;
    if_t                                *sif;
    pd_if_get_lport_id_args_t            if_args;
    pd_get_cpu_bypass_flowid_args_t      args;

    send_cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;

    p4plus_hdr.flags = 0;
    p4plus_hdr.p4plus_app_id = P4PLUS_APPTYPE_CPU;

    /* This seems to be not required for now.
    pd_l2seg_get_fromcpu_vlanid_args_t   l2_args;
    l2_args.l2seg = (l2seg_t*)(l2seg);
    l2_args.vid = &(send_cpu_hdr.hw_vlan_id);
    if (hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                      (void*)&l2_args) == HAL_RET_OK) {
        send_cpu_hdr.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
    }*/

    args.hw_flowid = 0;
    ret = hal_pd_call(PD_FUNC_ID_BYPASS_FLOWID_GET, &args);
    if (ret != HAL_RET_OK) {
        goto out;
    }

    p4plus_hdr.flow_index_valid = 1;
    p4plus_hdr.flow_index = args.hw_flowid;
    p4plus_hdr.dst_lport_valid = 1;

    sif = hal::find_if_by_handle(ctx.sep()->if_handle);
    if (!sif) {
        HAL_TRACE_INFO("Source endpoint interface not found.");
        goto out;
    }

    if_args.pi_if = sif;
    if (hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_LPORT_ID,
            (void*)&if_args) != HAL_RET_OK) {
        HAL_TRACE_INFO("Destination lport not found for proxy arp.");
        goto out;
    }
    p4plus_hdr.dst_lport = if_args.lport_id;

    ret = HAL_RET_OK;
out:
    return ret;
}

static hal_ret_t
do_proxy_arp_processing (struct ether_arp *arphead,
                        fte::ctx_t &ctx)
{
    hal_ret_t ret = HAL_RET_NOOP;
    hal::l2seg_t *l2seg  = ctx.sl2seg();
    ether_header_t *ethhdr;
    cpu_to_p4plus_header_t send_cpu_hdr = {0};
    p4plus_to_p4_header_t p4plus_hdr = {0};
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    uint16_t opcode = ntohs(arphead->ea_hdr.ar_op);
    ip_addr_t src_ip = {0};
    ip_addr_t dst_ip = {0};
    uint8_t  *pkt;
    uint32_t pkt_len;
    vlan_header_t *vlan_hdr;
    uint16_t vlan_tag;
    uint16_t *vlan_tag_ptr = nullptr;
    ep_t *dep = nullptr;

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

    ret = proxy_arp_build_cpu_p4_plus_header(ctx, send_cpu_hdr, p4plus_hdr);
    if (ret != HAL_RET_OK) {
        goto out;
    }

    if (ctx.vlan_valid()) {
        pkt_len = ARP_DOT1Q_PKT_SIZE;
        vlan_hdr = (vlan_header_t *)(ctx.pkt() + cpu_hdr->l2_offset);
        vlan_tag = ntohs(vlan_hdr->vlan_tag);
        vlan_tag_ptr = &vlan_tag;
    } else {
        pkt_len = ARP_PKT_SIZE;
    }

    pkt = (uint8_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_SFW, pkt_len);
    if (!pkt) {
        HAL_TRACE_ERR("Memory allocation failed for packet!");
        goto out;
    }

    ethhdr = (ether_header_t *)(ctx.pkt() + cpu_hdr->l2_offset);
    memcpy(&(src_ip.addr.v4_addr), arphead->arp_spa,
                sizeof(src_ip.addr.v4_addr));
    src_ip.addr.v4_addr = ntohl(src_ip.addr.v4_addr);
    fte::utils::hal_build_arp_response_pkt(dep->l2_key.mac_addr,
            &dst_ip, ethhdr->smac, &src_ip, vlan_tag_ptr, pkt);

    HAL_TRACE_INFO("Queuing proxy ARP response.");
    ctx.queue_txpkt(pkt, ARP_PKT_SIZE, &send_cpu_hdr,
            &p4plus_hdr, hal::SERVICE_LIF_CPU, CPU_ASQ_QTYPE,
            CPU_ASQ_QID, CPU_SCHED_RING_ASQ, types::WRING_TYPE_ASQ,
            free_arp_proxy_reply_pkt);

    ret = HAL_RET_OK;
out:
    return ret;
}

}  // namespace eplearn
}  // namespace hal
