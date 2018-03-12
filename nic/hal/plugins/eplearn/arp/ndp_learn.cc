#include "arp_trans.hpp"
#include "ndp_learn.hpp"
#include "nic/include/eth.h"
#include "nic/include/pkt_hdrs.hpp"

namespace hal {
namespace eplearn {

bool is_neighbor_discovery_flow(const hal::flow_key_t *key)
{
    return (key->flow_type == hal::FLOW_TYPE_V6 &&
            key->proto == IP_PROTO_ICMPV6 &&
            (key->icmp_type == ICMP_NEIGHBOR_SOLICITATION ||
             key->icmp_type == ICMP_NEIGHBOR_ADVERTISEMENT));
}

#if 0
bool is_ndp_broadcast(const hal::flow_key_t *key) {
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if ((((key->dmac[i] + 1) & key->dmac[i]) != 0)) {
            return false;
        }
    }
    return true;
}
#endif

static const icmpv6_option_info_t *
icmpv6_get_option(const uint8_t *options,
        uint32_t length, icmpv6_option_t type)
{
    uint32_t i = 0;
    const icmpv6_option_info_t *option;

    while ((i + sizeof(icmpv6_option_info_t)) <= length) {
        option = (const icmpv6_option_info_t*)(options + i);
        if (option->length == 0) {
           break;
        }
       if ((i + option->length * 8) > length) {
          break;
       }
       if (option->type == type)
          return option;
       i += option->length * 8;
    }
    return NULL;
}

static hal_ret_t
ndp_process_entry(const unsigned char *ether_pkt,
                  fte::ctx_t &ctx)
{
    arp_trans_key_t trans_key;
    arp_trans_t *trans;
    arp_event_data_t event_data;
    const uint8_t *hw_address;
    ip_addr_t ip_addr = {0};
    const icmpv6_option_info_t *option;
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    const icmpv6_neigh_solict_t *neigh_solic;
    const icmpv6_neigh_advert_t *neight_advert;
    const ipv6_header_t *ipv6_header;
    const icmp_header_t *icmp_header;
    const uint8_t *options;
    hal::ep_t *sep = ctx.sep();
    hal_ret_t ret = HAL_RET_OK;
    icmpv6_option_t option_type;

    ipv6_header = (const ipv6_header_t*)(ether_pkt + cpu_hdr->l3_offset);
    icmp_header = (const icmp_header_t*)(ether_pkt + cpu_hdr->l4_offset);

    if (icmp_header->type == ICMP_NEIGHBOR_SOLICITATION) {
        neigh_solic = (const icmpv6_neigh_solict_t*)(ether_pkt + cpu_hdr->l4_offset);
        options = neigh_solic->options;
        option_type = NDP_OPT_SOURCE_LINK_LAYER_ADDR;
    } else if (icmp_header->type == ICMP_NEIGHBOR_ADVERTISEMENT) {
        neight_advert = (const icmpv6_neigh_advert_t*)(ether_pkt + cpu_hdr->l4_offset);
        options = neight_advert->options;
        option_type = NDP_OPT_TARGET_LINK_LAYER_ADDR;
    } else {
        HAL_ASSERT(0);
    }

    option = icmpv6_get_option(options, 8, option_type);
    if (option == nullptr || option->length == 0) {
        ret = HAL_RET_ERR;
        goto out;
    }
    hw_address = option->value;

    /* Checking spoofing */
     if (memcmp(sep->l2_key.mac_addr, hw_address, ETHER_ADDR_LEN)) {
         HAL_TRACE_ERR(
             "Hardware address {} does not match the endpoint hardware address {}",
             macaddr2str(hw_address), macaddr2str(sep->l2_key.mac_addr));
         ret = HAL_RET_ERR;
         goto out;
     }

    ip_addr.af = IP_AF_IPV6;
    memcpy(&ip_addr.addr.v6_addr, ipv6_header->saddr, IP6_ADDR8_LEN);

    arp_trans_t::init_arp_trans_key(hw_address,
                                    ctx.sep(), ARP_TRANS_IPV6, &trans_key);
    trans = arp_trans_t::find_arptrans_by_id(trans_key);
    if (trans == NULL) {
        trans = new arp_trans_t(hw_address, ARP_TRANS_IPV6, ctx);
    }
    event_data.ip_addr = ip_addr;
    event_data.fte_ctx = &ctx;
    event_data.in_fte_pipeline = true;
    arp_trans_t::process_transaction(trans, ARP_ADD,
                                 (fsm_event_data)(&event_data));

 out:
    return ret;
}

hal_ret_t
neighbor_disc_process_packet(fte::ctx_t &ctx)
{
    const unsigned char *ether_pkt = ctx.pkt();
    hal_ret_t ret = HAL_RET_OK;


    if (ether_pkt == nullptr) {
        /* Skipping as not packet to process */
        goto out;
    }
    ret = ndp_process_entry(ether_pkt, ctx);

out:
    return ret;
}

}  // namespace eplearn
}  // namspace hal
