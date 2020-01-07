//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __LEARN_APULU_PKT_UTILS_HPP__
#define __LEARN_APULU_PKT_UTILS_HPP__

#include <arpa/inet.h>
#include "nic/apollo/packet/apulu/p4_cpu_hdr.h"
#include "nic/apollo/p4/include/apulu_defines.h"

#define LEARN_LIF_NAME          "net_ionic1"
#define LEARN_ARM_TO_P4_HDR_SZ  APULU_ARM_TO_P4_HDR_SZ
#define LEARN_P4_TO_ARM_HDR_SZ  APULU_P4_TO_ARM_HDR_SZ

static inline void
learn_p4_tx_hdr_fill (char *pkt)
{
    p4_tx_cpu_hdr_t *tx_hdr;

    tx_hdr = (p4_tx_cpu_hdr_t *)pkt;
    memset(tx_hdr, 0, sizeof(p4_tx_cpu_hdr_t));
    tx_hdr->pad = 0;
    tx_hdr->nexthop_valid = 0;
    // TODO: set lif info from rx hdr
    tx_hdr->lif_sbit0_ebit7 = 0x46 & 0xff;
    tx_hdr->lif_sbit8_ebit10 = 0x46 >> 0x8;

    tx_hdr->lif_flags = htons(tx_hdr->lif_flags);
    // TODO: set nexthop info after gleaning from rx hdr
    //tx_hdr->nexthop_type = 0;
    //tx_hdr->nexthop_id = 0;
}

#endif  //__LEARN_APULU_PKT_UTILS_HPP__
