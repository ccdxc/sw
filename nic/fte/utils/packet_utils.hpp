//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Misc Packet utils to inject and modify packets.
//-----------------------------------------------------------------------------

#ifndef HAL_SRC_UTILS_PACKET_UTILS_HPP_
#define HAL_SRC_UTILS_PACKET_UTILS_HPP_

#define ARP_PKT_SIZE sizeof(ether_header_t) + sizeof(struct ether_arp)
#define ARP_DOT1Q_PKT_SIZE sizeof(vlan_header_t) + sizeof(struct ether_arp)

namespace fte {
namespace utils {

void hal_build_arp_request_pkt(const mac_addr_t src_mac,
        const ip_addr_t *ip_addr, const uint16_t *vlan_tag, uint8_t *pkt);
void hal_build_arp_response_pkt(const mac_addr_t src_mac,
        const ip_addr_t *src_ip_addr,
        const mac_addr_t dst_mac, const ip_addr_t *dst_ip_addr,
        const uint16_t *vlan_tag, uint8_t *pkt);
hal_ret_t hal_inject_arp_request_pkt(const hal::l2seg_t *segment,
        mac_addr_t src_mac);
hal_ret_t hal_inject_arp_response_pkt(const hal::l2seg_t *segment,
        const mac_addr_t src_mac, const ip_addr_t *src_ip_addr,
        const mac_addr_t dst_mac, const ip_addr_t *dst_ip_addr);
} // namespace utils
} // namespace fte
#endif /* HAL_SRC_UTILS_PACKET_UTILS_HPP_ */
