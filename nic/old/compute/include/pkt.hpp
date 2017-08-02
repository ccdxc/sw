#ifndef __PKT_HPP__
#define __PKT_HPP__

#include <base.hpp>
#include <l2.hpp>
#include <l3.hpp>
#include <action.hpp>
#include <svc.hpp>

//------------------------------------------------------------------------------
// packet meta expected from ASIC when it sends packets to compute CPUs
//------------------------------------------------------------------------------
typedef struct pkt_meta_s {
    uint16_t       ifindex;           // source if index (VF# or physical port
                                      // or tunnel etc.)
                                      // this should tell us if it came on
                                      // uplink or tunnel or local VF etc. all
    uint8_t        encap_type_o:3;    // encap type
    uint32_t       encap_o;           // vlan or vnid (wire encap)
    mac_addr_t     smac_o;            // src MAC
    mac_addr_t     dmac_o;            // dst MAC
    uint8_t        proto_o;           // IP protocol
    uint16_t       sport_o;           // L4 sport (or equivalent for ICMP etc.)
    uint16_t       dport_o;           // L4 dport (or equivalent for ICMP etc.)
    ipvx_addr_t    sip_o;             // src IP address
    ipvx_addr_t    dip_o;             // dst IP address

    uint16_t       flow_key_type:2;   // IPv4 or IPv6 or L2
    uint8_t        encap_type_i:3;    // encap type
    uint32_t       encap_i;           // vlan/vnid wire encap
    mac_addr_t     smac_i;            // src MAC
    mac_addr_t     dmac_i;            // dst MAC
    uint8_t        proto_i;           // IP protocol
    uint16_t       sport_i;           // L4 sport (or equivalent for ICMP/AH/ESP etc.)
    uint16_t       dport_i;           // L4 dport (or equivalent for ICMP/AH/ESP etc.)
    ipvx_addr_t    sip_i;             // src IP address
    ipvx_addr_t    dip_i;             // dst IP address

    uint16_t       dir:1;             // ingress or egress
    uint16_t       tunnel_terminate:1;// TRUE if we are terminating the tunnel
    uint16_t       policer_id:10;     // policer id  ===> TBD : do we dervie this in P4 by this time ?
    uint8_t        cos;               // cos bits ==> TBD - which table in P4 is giving this ? input properties ?
    uint8_t        ip_options;        // one bit per detected option ?
    uint8_t        tcp_options;       // one bit per detected option ?
    uint32_t       reason_bmap;       // redirect reason bitmap (flow-miss and/or TCP FIN/RST etc.)
    uint32_t       tcp_seq_num;       // TCP sequence number
    uint32_t       tcp_ack_num;       // TCP acknowledgement number
    uint8_t        tcp_flags;         // TCP flags
    uint16_t       tcp_win_sz;        // TCP window size
    uint8_t        tcp_ws:4;          // TCP window scale option
    uint8_t        l3_offset;         // L3 header offset
    uint8_t        l4_offset;         // L4 header offset
    uint8_t        l7_offset;
} __PACK__ pkt_meta_t;

//------------------------------------------------------------------------------
// action list that all packets will be using
//------------------------------------------------------------------------------
typedef struct action_list_s {

    // per service preprocess functions, if any
    // packet is expected to be parsed and/or filtered in this stage and packet
    // context is setup for use in following stages
    action_func    pre_process_act[SVC_ID_MAX];

    // generic flow lookup
    action_func    flow_lookup_act;

    // per service policy lookup functions, if any
    action_func    policy_lookup_act[SVC_ID_MAX];

    // flow_setup will set use the policy lookup results from previous stage and
    // setsup flow/session after policy transposition/resolution
    action_func    flow_setup_act;

    // fast_path will do all the necessary rewrites and send packet out
    action_func    fast_path_act;
} action_list_t;

//------------------------------------------------------------------------------
// global action list
//------------------------------------------------------------------------------
extern action_list_t g_pkt_actions;

#endif    // __PKT_HPP__

