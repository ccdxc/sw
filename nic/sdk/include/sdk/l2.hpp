//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// generic layer 2 packet headers and utilities
///
//----------------------------------------------------------------------------

#ifndef __L2_HPP__
#define __L2_HPP__

#include <stdint.h>
#include "include/sdk/eth.hpp"
#include "include/sdk/ip.hpp"

typedef struct arp_hdr_s {
    uint16_t htype;
#define ARP_HRD_TYPE_ETHER   1
    uint16_t ptype;
    uint8_t  hlen;
    uint8_t  plen;
    uint16_t op;
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2
} __PACK__ arp_hdr_t;

typedef struct arp_data_ipv4_s {
    mac_addr_t  smac;
    ipv4_addr_t sip;
    mac_addr_t  tmac;
    ipv4_addr_t tip;
} __PACK__ arp_data_ipv4_t;

#define ARP_PKT_LEN                 (sizeof(eth_hdr_t) + sizeof(arp_hdr_t) + \
                                     sizeof(arp_data_ipv4_t))

// ARP pkt length is 42 bytes, so use min Ethernet frame len - FCS
#define ARP_PKT_ETH_FRAME_LEN       (ETH_MIN_FRAME_LEN - 4)

#endif    // __L2_HPP__
