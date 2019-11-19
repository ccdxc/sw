// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Defines data structure to read/store init config

#ifndef __PDSA_CONFIG_HPP__
#define __PDSA_CONFIG_HPP__

#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0glob.h>
#include <a0mib.h>
#include <ambips.h>
#include <a0stubs.h>
#include <a0cpif.h>
#include "smsiincl.h"
#include "smsincl.h"
}
#include "include/sdk/ip.hpp"

typedef struct pdsa_config_s {
    NBB_ULONG   oid_len;
    NBB_ULONG   data_len;
    NBB_ULONG   correlator;               
    NBB_LONG    row_status;
    NBB_LONG    admin_status;
    NBB_ULONG   entity_index;
    NBB_LONG    sm_css_replay;
    NBB_LONG    location_capable;
    NBB_LONG    ft_capable;
    NBB_ULONG   bd_type;
    NBB_ULONG   bd_index;
    NBB_ULONG   bd_sub_index;
    NBB_LONG    stateful;
    NBB_LONG    if_index;
    NBB_ULONG   interface_id;
    NBB_ULONG   partner_type;
    NBB_ULONG   partner_index;
    NBB_ULONG   sub_index;
    NBB_LONG    prefix_len;
    NBB_LONG    enable;
    NBB_LONG    ipv4_enabled;
    NBB_LONG    ipv4_forwarding;
    NBB_LONG    ipv6_enabled;
    NBB_LONG    ipv6_forwarding;
    NBB_LONG    forwarding_mode;
    NBB_LONG    addr_family;
    NBB_ULONG   i3_index;
    NBB_ULONG   slave_entity_index;
    NBB_LONG    slave_type;
    NBB_ULONG   local_as;
    NBB_LONG    restart_supported;
    NBB_LONG    router_id;
    NBB_ULONG   sck_index;
    NBB_ULONG   rm_index;
    NBB_LONG    afi;
    NBB_ULONG   safi;
    NBB_LONG    next_hop_safi;
    NBB_LONG    state_kept;
    NBB_ULONG   local_port;
    NBB_ULONG   remote_port;
    NBB_ULONG   local_addr_scope_id;
    NBB_ULONG   connect_retry_interval;
    NBB_ULONG   remote_as;
    NBB_ULONG   local_nm;
    NBB_LONG    config_passive;
    NBB_LONG    disable_afi_safi;
    NBB_LONG    join;
    NBB_LONG    rd_cfg_or_auto;
    NBB_LONG    encapsulation;
    NBB_ULONG   evi_index;
    NBB_ULONG   vni;
    NBB_LONG    vrf_name_len;
    NBB_LONG    vlan;
    NBB_LONG    if_type;
    NBB_BYTE    cfg_rd[AMB_EVPN_EXT_COMM_LENGTH];
    NBB_BYTE    mac_address[AMB_MAC_ADDR_LEN];
    NBB_BYTE    vrf_name[AMB_VRF_NAME_MAX_LEN];
    NBB_BYTE    route_distinguisher[AMB_EVPN_EXT_COMM_LENGTH];
    ip_addr_t   ip_addr;
    ip_addr_t   remote_addr;
    // TODO: Temporary for init purpose
    unsigned int g_node_a_ip;
    unsigned int g_node_b_ip;
    unsigned int g_node_a_ac_ip;
    unsigned int g_node_b_ac_ip;
    unsigned int g_evpn_if_index;
}__PACK__ pdsa_config_t;

#endif /*__PDSA_CONFIG_HPP__*/
