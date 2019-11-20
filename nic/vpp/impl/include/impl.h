//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __IMPL_INCLUDE_IMPL_H__
#define __IMPL_INCLUDE_IMPL_H__

#include "nic/sdk/include/sdk/eth.hpp"

// Function prototypes
int pds_arp_pkt_offset_get(void *p4_rx_meta, uint32_t *offset_addr);
int pds_vr_mac_get(void *p4_rx_meta, mac_addr_t *mac_addr);

#endif    // __IMPL_INCLUDE_IMPL_H__
