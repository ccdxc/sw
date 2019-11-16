// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs header file for metaswitch stub programming 

#ifndef __PDSA_MGMT_UTILS_HPP__
#define __PDSA_MGMT_UTILS_HPP__
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
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"

extern unsigned int g_node_a_ip;
extern unsigned int g_node_b_ip;
extern unsigned int g_node_a_ac_ip;
extern unsigned int g_node_b_ac_ip;
extern unsigned int g_evpn_if_index;

#define NODE_A_IP       g_node_a_ip
#define NODE_B_IP       g_node_b_ip
#define NODE_A_AC_IP    g_node_a_ac_ip
#define NODE_B_AC_IP    g_node_b_ac_ip
#define EVPN_IF_INDEX   g_evpn_if_index

NBB_VOID  pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr);

NBB_VOID pdsa_convert_long_to_pdsa_ipv4_addr (NBB_ULONG ip,
                                              ip_addr_t *pdsa_ip_addr); 

#endif /*__PDSA_MGMT_UTILS_HPP__*/
