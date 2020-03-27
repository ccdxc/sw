//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_BD_H__
#define __VPP_IMPL_APULU_BD_H__

#include <impl_db.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPV4_MASK 0xFFFFFFFF
#define IPV4_PFX_LEN 32

int pds_l2vnid_get(uint16_t bd_id, uint32_t *l2_vni);

// check if ipv4 address is within subnet pfx
static inline bool
pds_subnet_check(u16 bd_id, u32 ip_addr)
{
    pds_impl_db_subnet_entry_t *subnet;
    u32 mask = 0;

    subnet = pds_impl_db_subnet_get(bd_id);
    if (PREDICT_FALSE(subnet == NULL)) {
        return -1;
    }
    mask = IPV4_MASK << (IPV4_PFX_LEN - subnet->prefix_len);
    if (PREDICT_FALSE((ip_addr & mask) != (subnet->vr_ip.ip4.as_u32 & mask))) {
        return -1;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif      //__VPP_IMPL_APULU_BD_H
