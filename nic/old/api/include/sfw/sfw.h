#ifndef __SFW_H__
#define __SFW_H__

#include <policy.hpp>

//------------------------------------------------------------------------------
// IP address range node used on the rule policy
//------------------------------------------------------------------------------
// forward declaration
typedef struct ipvx_range_elem_s ipvx_range_list_elem_t;

#define NUM_IP_RANGES_PER_NODE                       4

typedef struct ipvx_range_elem_s {
    ipvx_range_t             ip_ranges[NUM_IP_RANGES_PER_NODE];
    ipvx_range_list_elem_t   *next;
} __PACK__ ipvx_range_elem_t;

//------------------------------------------------------------------------------
// stateful firewall rule match condition(s)
//------------------------------------------------------------------------------
typedef struct sfw_match_s {
    vif_id_t             svif;
    vif_id_t             dvif;
    uint8_t              ip_proto;
    ipvx_range_elem_t    *sip_list;
    ipvx_range_elem_t    *dip_list;
    ipvx_range_elem_t    *sip_exclude_list;
    ipvx_range_elem_t    *dip_exclude_list;
    port_range_t         sport_range;
    port_range_t         dport_range;
} __PACK__ sfw_match_t;

//------------------------------------------------------------------------------
// stateful firewall actions
// ALLOW  - accept the packet
// DROP   - drop is silent drop
// REJECT - send RST for TCP, ICMP administratively prohibited for UDP, no-op
//          for others
//------------------------------------------------------------------------------
#define SFW_ACTION_ALLOW                             0
#define SFW_ACTION_DROP                              1
#define SFW_ACTION_REJECT                            2
typedef struct sfw_action_s {
    uint8_t        log:1;            // TRUE, if we need to log when rule is hit
    uint8_t        log_level:3;      // log level to be used if log === TRUE
    uint8_t        fwd_action:2;     // allow | drop | reject
} __PACK__ sfw_action_t;

//------------------------------------------------------------------------------
// stateful firewall rule
// NOTE: these rules are grouped under VRF/BD, ingress/egress etc. so not
// capturing those fields here but they will be part of the incoming policy from
// user
//------------------------------------------------------------------------------
#define MAX_RULE_NAME_LEN                            32
typedef struct sfw_rule_s {
    uint32_t             name[MAX_RULE_NAME_LEN];    // rule name
    uint32_t             id;                         // unique rule id
    uint16_t             prio;                       // rule priority
    sfw_match_t          match;                      // rule match condition
    sfw_action_t         action;                     // rule action(s)
} __PACK__ sfw_rule_t;

typedef struct sfw_rules_s {
    uint32_t          num_rules;
    sfw_rule_t        sfw_rules[0];
} __PACK__ sfw_rules_t;

#endif    // __SFW_H__

