#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct sacl_ip1_k           k;
struct sacl_ip1_d           d;

// Define Table Name and Action Names
#define table_name         sacl_ip1
#define action_keys32b     match1_ipv4
#define action_data32b     match1_ipv4_retrieve
#define action_keys128b    match1_ipv6

// Define table field names for the selected key-widths
#define keys32b(a)         d.u.match1_ipv4_d.key ## a
#define keys32b_(a)        d.u.match1_ipv4_retrieve_d.key ## a
#define data32b(a)         d.u.match1_ipv4_retrieve_d.data ## a
#define keys128bhi(a)      d.u.match1_ipv6_d.key ## a[127:64]
#define keys128blo(a)      d.u.match1_ipv6_d.key ## a[63:0]

// Define key field names
#define key                k.sacl_metadata_ip
#define keylo              k.sacl_metadata_ip[63:0]
#define keyhi              k.sacl_metadata_ip[127:64]
#define base_addr          k.{sacl_metadata_ip_table_addr_sbit0_ebit1,\
                              sacl_metadata_ip_table_addr_sbit2_ebit33}
#define curr_addr          k.{sacl_metadata_ip_table_addr_sbit0_ebit1,\
                              sacl_metadata_ip_table_addr_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.sacl_metadata_ip_table_addr_next

// Define result register and handler function name
#define res_reg            r7
#define res_handler        sacl_ip_handler

%%

#include "../include/lpm.h"

res_handler:
    nop.e
    nop
