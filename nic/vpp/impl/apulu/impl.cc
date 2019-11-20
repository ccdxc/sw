//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "nic/vpp/impl/include/impl.h"

// get arp packet header offset
// TODO: to be filled by sai by moving code from arp_proxy/
int
pds_arp_pkt_offset_get (void *p4_rx_meta, uint32_t *offset_addr)
{
    return 0;
}

// get virtual router mac
// TODO: to be filled by sai by moving code from arp_proxy/
int
pds_vr_mac_get (void *p4_rx_meta, mac_addr_t *mac_addr)
{
    return 0;
}
