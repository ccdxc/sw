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

#endif    // __VPP_INFRA_UTILS_H__
