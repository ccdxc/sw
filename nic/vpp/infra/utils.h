//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_UTILS_H__
#define __VPP_INFRA_UTILS_H__

#define ETH_HDR_LEN                                 14
// MAC address
#define ETH_ADDR_LEN                                 6
typedef uint8_t    mac_addr_t[ETH_ADDR_LEN];

#define BIT(pos)                (1UL << (pos))
#define BIT_SET(flags, pos)     ((flags) |= (pos))
#define BIT_ISSET(flags, pos)   ((flags) & (pos))

static inline void
pds_host_to_net_u24 (uint8_t *data)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    // swap bytes
    if (data[0] != data[2]) {
        data[0] ^= data[2];
        data[2] ^= data[0];
        data[0] ^= data[2];
    }
#endif
    return;
}

#endif    // __VPP_INFRA_UTILS_H__
