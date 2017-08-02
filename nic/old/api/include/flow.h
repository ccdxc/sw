#ifndef __FLOW_H__
#define __FLOW_H__

#include <base.h>
#include <l2.hpp>
#include <l3.hpp>

//------------------------------------------------------------------------------
// flow key types
//------------------------------------------------------------------------------
#define FLOW_KEY_TYPE_MAC             0
#define FLOW_KEY_TYPE_IPV4            1
#define FLOW_KEY_TYPE_IPV6            2

//------------------------------------------------------------------------------
// IPv4 flow key
//------------------------------------------------------------------------------
typedef struct flow_key_v4_s {
    uint16_t       vrf:VRF_BIT_WIDTH;
    uint16_t       dir:1;
    uint8_t        proto;
    ipv4_addr_t    sip;
    ipv4_addr_t    dip;
    uint16_t       sport;
    uint16_t       dport;
} __PACK__ flow_key_v4_t;

//------------------------------------------------------------------------------
// IPv6 flow key
//------------------------------------------------------------------------------
typedef struct flow_key_v6_s {
    uint16_t       vrf:VRF_BIT_WIDTH;
    uint16_t       dir:1;
    uint16_t       mbz:5;
    uint8_t        proto;
    ipv6_addr_t    sip;
    ipv6_addr_t    dip;
    uint16_t       sport;
    uint16_t       dport;
} __PACK__ flow_key_v6_t;

//------------------------------------------------------------------------------
// L2 flow key
//------------------------------------------------------------------------------
typedef struct flow_key_l2_s {
    uint16_t       bd:BD_BIT_WIDTH;
    uint16_t       mbz:6;
    etype_t        etype;
    mac_addr_t     smac;
    mac_addr_t     dmac;
} __PACK__ flow_key_l2_t;

//------------------------------------------------------------------------------
// unified flow key
//------------------------------------------------------------------------------
typedef union flow_key_s {
    flow_key_v4_t    v4_key;
    flow_key_v6_t    v6_key;
    flow_key_l2_t    l2_key;
} __PACK__ flow_key_t;

#define FLOW_ACTION_ALLOW                // this should cover punt-to-cpu as well !!
#define FLOW_ACTION_DROP
#define FLOW_ACTION_REJECT
#define FLOW_ACTION_COPY_TO_REMOTE       // include NPU
#define FLOW_ACTION_NORMALIZE
#define FLOW_ACTION_ALERT_ON_CLOSE

// flow entry
typedef struct flow_s {
} flow_t;

#endif    // __FLOW_H__

