#include "arp_trans.hpp"
#include "arp_learn.hpp"
#include "nic/include/eth.h"
namespace hal {
namespace network {

void arp_init() {}

bool is_arp_flow(const hal::flow_key_t *key) {
    /* TODO : Look at Dot1q header as well */
    return (key->ether_type == ETH_TYPE_ARP);
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

    arp_trans_t *existing_trans = arp_trans_t::find_arptrans_by_id(trans_key);

    event_data.fte_ctx = &ctx;
    if (existing_trans == NULL) {
        arp_trans_t *arpt = new arp_trans_t(hw_address, protocol_address, ctx);

        arp_trans_t::process_transaction(arpt, ARP_ADD,
                                         (fsm_event_data)(&event_data));
    }
    return HAL_RET_OK;
}

static hal_ret_t validate_arp_packet(const unsigned char *ether_pkt,
                                     fte::ctx_t &ctx) {
    struct ether_arp *arphead = (struct ether_arp *)(ether_pkt + 14);
    hal::ep_t *sep = ctx.sep();
    uint16_t opcode = ntohs(arphead->ea_hdr.ar_op);

    /* Checking ARP spoofing */
    if (((opcode == ARPOP_REQUEST) &&
         memcmp(sep->l2_key.mac_addr, arphead->arp_sha, ETHER_ADDR_LEN)) ||
        ((opcode == ARPOP_REPLY) &&
         memcmp(sep->l2_key.mac_addr, arphead->arp_tha, ETHER_ADDR_LEN))) {
        HAL_TRACE_ERR(
            "Hardware address does not match the endpoint hardware address");
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

hal_ret_t arp_process_packet(fte::ctx_t &ctx) {
    const unsigned char *ether_pkt = ctx.pkt();
    struct ether_arp *arphead;
    hal_ret_t ret;

    arphead = (struct ether_arp *)(ether_pkt + L2_ETH_HDR_LEN);

    uint16_t opcode = ntohs(arphead->ea_hdr.ar_op);
    if (opcode == ARPOP_REQUEST || opcode == ARPOP_REPLY) {
        ret = validate_arp_packet(ether_pkt, ctx);
        if (ret != HAL_RET_OK) {
            return ret;
        }
        arp_process_entry(arphead->arp_sha, arphead->arp_spa, ctx);
        arp_process_entry(arphead->arp_tha, arphead->arp_tpa, ctx);
    }

    return HAL_RET_OK;
}

}  // namespace network
}  // namespace hal
