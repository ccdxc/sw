#include "arp_trans.hpp"
#include "arp_learn.hpp"
#include "nic/include/eth.h"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace network {

void arp_init() {}

bool is_arp_flow(const hal::flow_key_t *key) {
    return (key->ether_type == ETH_TYPE_ARP);
}

bool is_arp_broadcast(const hal::flow_key_t *key) {
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if ((((key->dmac[i] + 1) & key->dmac[i]) != 0)) {
            return false;
        }
    }
    return true;
}

static hal_ret_t arp_process_entry(uint8_t *hw_address,
                                   uint8_t *protocol_address,
                                   fte::ctx_t &ctx) {
    arp_trans_key_t trans_key;
    arp_event_data_t event_data;

    int i;

    for (i = 0; i < ETHER_ADDR_LEN; i++) {
        /*Ignore broadcast and all zeros */
        if ((((hw_address[i] + 1) & hw_address[i]) != 0) ||
            (hw_address[i] != 0)) {
            break;
        }
    }

    if (i == ETHER_ADDR_LEN) {
        /* Ignore as mac address is 0 */
        return HAL_RET_OK;
    }

    arp_trans_t::init_arp_trans_key(hw_address, ctx.sep(), &trans_key);

    arp_trans_t *trans = arp_trans_t::find_arptrans_by_id(trans_key);

    event_data.fte_ctx = &ctx;
    event_data.protocol_address = protocol_address;
    if (trans == NULL) {
        trans = new arp_trans_t(hw_address, ctx);
    }

    arp_trans_t::process_transaction(trans, ARP_ADD,
                                         (fsm_event_data)(&event_data));
    return HAL_RET_OK;
}

static hal_ret_t validate_arp_packet(const struct ether_arp *arphead,
                                     fte::ctx_t &ctx) {
    hal::ep_t *sep = ctx.sep();


    /* Checking ARP spoofing */
    if (memcmp(sep->l2_key.mac_addr, arphead->arp_sha, ETHER_ADDR_LEN)) {
        HAL_TRACE_ERR(
            "Hardware address {} does not match the endpoint hardware address {}",
            macaddr2str(arphead->arp_sha), macaddr2str(sep->l2_key.mac_addr));
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
    if (opcode == ARPOP_REQUEST || opcode == ARPOP_REPLY) {
        ret = arp_process_entry(arphead->arp_sha, arphead->arp_spa, ctx);
    } else if (opcode == ARPOP_REVREPLY) {
        /* HAve to fix RARP Still, as we have to trust RARP Server Response */
        /*ret = arp_process_entry(arphead->arp_tha, arphead->arp_tpa, ctx);*/
    }

out:
    return ret;
}

}  // namespace network
}  // namespace hal
