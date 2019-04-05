#include "../../p4/include/lpm_defines.h"

#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct route_k             k;
struct route_d             d;

// Define Table Name and Action Names
#define table_name         route
#define action_keys32b     search_ipv4
#define action_keys64b     search_ipv6
#define action_data32b     search_ipv4_retrieve
#define action_data64b     search_ipv6_retrieve

// Define table field names for the selected actions
#define keys32b(a)         d.u.search_ipv4_d.key ## a
#define keys64b(a)         d.u.search_ipv6_d.key ## a
#define keys32b_(a)        d.u.search_ipv4_retrieve_d.key ## a
#define keys64b_(a)        d.u.search_ipv6_retrieve_d.key ## a
#define data32b(a)         d.u.search_ipv4_retrieve_d.data ## a
#define data64b(a)         d.u.search_ipv6_retrieve_d.data ## a

// Define key field names
#define key                k.p4_to_txdma_header_lpm_dst
#define base_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}
#define curr_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.txdma_control_lpm_addr

// Define result register and handler function name
#define res_reg            r7
#define res_handler        route_ipv4_handler

%%

#include "../include/lpm.h"

res_handler:
    phvwr.e         p.txdma_to_p4e_header_nexthop_index, res_reg
    phvwr           p.app_header_table0_valid, FALSE       // LPM bypass
