// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __OCI_API_HPP__
#define __OCI_API_HPP__

//------------------------------------------------------------------------------
// NOTE:
// for packets coming from fabric, following packet formats are assumed:
// 1. Eth-IP-GRE-MPLS-IP
// 2. Eth-IP-UDP-MPLS-IP
// 3. Eth-IP (untagged)
//
// There is no inner Eth header in all these cases
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// TODO:
// 1. epoch (i.e. update operations)
// 2. vr_mac
// 3. resource pools
// 4. policers
//------------------------------------------------------------------------------

typedef uint32_t    vcn_id_t;
typedef uint32_t    subnet_id_t;
typedef uint32_t    vnic_id_t;
typedef uint32_t    rule_id_t;
typedef uint16_t    vlan_id_t;
typedef uint32_t    slot_id_t;    // 20 bit id

#define MAX_IP_PER_VNIC        31

// global switch port configuration for this NIC
typedef struct switch_port_cfg_s {
    uint32_t       flags;
    ipv4_addr_t    mytep_ip;    // local linux IP in the substrate
    mac_addr_t     my_mac;      // local MAC
} switch_port_cfg_t;

// GRE tunnels
typedef struct tep_s {
    uint32_t       flags;
    ipv4_addr_t    tep_ip;    // IP address of remote server in the substrate
} tep_t;


// VCN representation
typedef struct vcn_s {
    uint32_t       flags;
    vcn_id_t       vcn_id;     // VCN's unique id
    ip_prefix_t    vcn_pfx;    // VCN's CIDR block
} vcn_t;

// subnet representation
typedef struct subnet_s {
    uint32_t       flags;
    subnet_id_t    subnet_id;     // subnet's unique id
    vcn_id_t       vcn_id;        // VCN this subnet is part of
    ip_prefix_t    subnet_pfx;    // subnet's CIDR block
} subnet_t;

typedef struct vnic_ip_info_s {
    ip_addr_t      private_ip;
    ip_addr_t      public_ip;
    ip_addr_t      substrate_ip;
    uint8_t        public_ip_valid:1;
    uint8_t        substrate_ip_valid:1;
} vnic_ip_info_t;

// VNIC representation
typedef struct vnic_s {
    uint32_t          flags;
    vnic_id_t         vnic_id;          // vnic's unique id
    subnet_id_t       subnet_id;        // subnet id of this vnic
    vlan_id_t         vlan_id;          // VLAN tag assigned to this vnic
    slot_id_t         slot;             // mpls tag for this vnic
    mac_addr_t        mac_addr;         // overlay MAC address of the vnic
    vnic_ip_info_t    ip_info[MAX_IP_PER_VNIC];    // IP address information of this VNIC
    bool              src_dst_check;    // true if src/dst check is enabled
    // shall we use a tep-hndl instead ?? 
    tep_t             tep;   // if tep == mytep local vnic, else remote vnic
} vnic_t;

// single route
typedef struct route_s {
    ip_prefix_t    ip_pfx;    // route prefix
    ip_addr_t      nh_ip;     // nexthop's IP address
} route_t;

// route rules per subnet
typedef struct routes_s {
    uint32_t       flags;
    subnet_id_t    subnet_id;
    uint32_t       num_routes;
    route_t        routes[0];
} routes_t;

typedef struct port_match_s {
    port_range_t    sport_range;
    port_range_t    dport_range;
} port_match_t;

typedef icmp_match_s {
    uint8_t     icmp_type;
    uint8_t     icmp_code;
} icmp_match_t;

typedef struct l4_match_s {
    union {
        port_match_t    port_match;
        icmp_match_t    icmp_match;
    };
} l4_match_t;

// security rule match criteria
typedef struct rule_match_s {
    subnet_id_t     subnet;
    ip_prefix_t     sip;
    ip_prefix_t     dip;
    uint8_t         ip_proto;
    l4_match_t      l4_match;
} rule_match_t;

// security rule actions
typedef enum security_rule_action_e {
    FW_ACTION_ALLOW = 0,
    FW_ACTION_DENY  = 1,    // default deny
} security_rule_action_t;

typedef struct security_rule_s {
    bool                      stateful;
    rule_match_t              match;
    security_rule_action_t    action;
} security_rule_t;

typedef struct security_list_s {
    uint32_t        flags;
    subnet_id_t        subnet_id;
    uint32_t           num_rules;
    security_rule_t    rules[0];
} security_list_t;

ret_code_t vcn_create(vcn_t *vcn);
ret_code_t vcn_delete(vcn_t *vcn);
ret_code_t subnet_create(subnet_t *subnet);
ret_code_t subnet_delete(subnet_t *subnet);
ret_code_t tep_create(tep_t *tep);
ret_code_t tep_delete(tep_t *tep);
ret_code_t vnic_create(vnic_t *vnic);
ret_code_t vnic_delete(vnic_t *vnic);
ret_code_t security_list_create(security_list_t *security_list);
ret_code_t security_list_delete(security_list_t *security_list);

#endif    // __OCI_API_HPP__

