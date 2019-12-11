//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __IMPL_APULU_IMPL_H__
#define __IMPL_APULU_IMPL_H__

#include <nic/vpp/infra/utils.h>
#include <nic/vpp/infra/apulu/p4_cpu_hdr.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
pds_ingress_bd_id_get (void *hdr)
{
    return (((p4_rx_cpu_hdr_t*)hdr)->ingress_bd_id);
}

// get arp packet header offset
static inline int
pds_arp_pkt_offset_get (void *hdr)
{
    return (((p4_rx_cpu_hdr_t*)hdr)->l2_offset);
}

// Function prototypes
int pds_dst_mac_get(void *p4_rx_meta, mac_addr_t mac_addr, bool remote,
                    uint32_t dst_addr);
void pds_mapping_table_init();

#ifdef __cplusplus
}
#endif
#endif    // __IMPL_APULU_IMPL_H__
