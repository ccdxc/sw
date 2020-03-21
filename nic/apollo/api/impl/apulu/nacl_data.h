//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline NACL opaque data declaration
///
//----------------------------------------------------------------------------

#ifndef __APULU_NACL_DATA_H__
#define __APULU_NACL_DATA_H__

typedef enum {
    NACL_DATA_ID_L2_MISS_ARP,
    NACL_DATA_ID_ARP_REPLY,
    NACL_DATA_ID_L2_MISS_DHCP,
    NACL_DATA_ID_FLOW_MISS_ARP,
    NACL_DATA_ID_FLOW_MISS_DHCP_HOST,
    NACL_DATA_ID_FLOW_MISS_DHCP_UPLINK,
    NACL_DATA_ID_FLOW_MISS_IP4_IP6,
} nacl_data;

#endif    // __APULU_NACL_DATA_H__
