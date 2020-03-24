//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_ARP_PROXY_ARP_PROXY_H
#define __VPP_ARP_PROXY_ARP_PROXY_H

typedef int (*arp_proxy_vendor_dst_mac_get_cb)(u16 vpc_id, u16 bd_id,
             mac_addr_t mac_addr, uint32_t dst_addr);

void arp_proxy_register_vendor_dst_mac_get_cb(arp_proxy_vendor_dst_mac_get_cb cb);

#endif    // __VPP_ARP_PROXY_ARP_PROXY_H
