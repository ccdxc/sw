#include "arp_trans.hpp"
#include "arp_learn.hpp"
#include "nic/include/eth.h"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace eplearn {

void arp_init() {}

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
    trans = arp_trans_t::find_arptrans_by_id(trans_key);
    if (trans == NULL) {
        HAL_TRACE_INFO("Creating new ARP transaction {}", event);
        trans = new arp_trans_t(hw_address, ARP_TRANS_IPV4, ctx);
    }
    if (opcode == ARPOP_REVREQUEST) {
        event = RARP_REQ;
    } else {
        event = ARP_ADD;
    }
    memcpy(&(ip_addr.addr.v4_addr), protocol_address,
            sizeof(ip_addr.addr.v4_addr));
    ip_addr.addr.v4_addr = ntohl(ip_addr.addr.v4_addr);
    event_data.ip_addr = &ip_addr;
    event_data.fte_ctx = &ctx;
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
        event_data.ip_addr = &ip_addr;
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

    opcode = ntohs(arphead->ea_hdr.ar_op);

    if ((opcode != ARPOP_REQUEST) && (opcode != ARPOP_REPLY) &&
            (opcode != ARPOP_REVREQUEST) && (opcode != ARPOP_REVREPLY)) {
        HAL_TRACE_ERR(
            "Invalid ARP Request type {} received from EP"
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

hal_ret_t arp_process_packet(fte::ctx_t &ctx) {
    const unsigned char *ether_pkt = ctx.pkt();
    struct ether_arp *arphead;
    hal_ret_t ret = HAL_RET_OK;
    uint16_t opcode;
    const fte::cpu_rxhdr_t* cpu_hdr;

    if (ether_pkt == nullptr) {
        /* Skipping as not packet to process */
        goto out;
    }

    cpu_hdr = ctx.cpu_rxhdr();
    if (cpu_hdr->flags & CPU_FLAGS_VLAN_VALID) {
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

out:
    return ret;
}

}  // namespace eplearn
}  // namespace hal
