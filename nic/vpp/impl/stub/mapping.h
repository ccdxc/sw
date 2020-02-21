//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __IMPL_STUB_IMPL_H__
#define __IMPL_STUB_IMPL_H__

#include <nic/vpp/infra/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
pds_ingress_bd_id_get (void *hdr)
{
    return 0;
}

// get arp packet header offset
static inline int
pds_arp_pkt_offset_get (void *hdr)
{
    return 0;
}

static inline void
pds_mapping_table_init (void)
{
    return;
}

// get virtual router mac
static inline int
pds_dst_mac_get (void *hdr, mac_addr_t mac_addr, uint32_t dst_addr)
{
    return 0;
}

#ifdef __cplusplus
}
#endif
#endif    // __IMPL_STUB_IMPL_H__
