#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_ip_k.h"

struct phv_                 p;
struct sacl_ip_k_           k;
struct sacl_ip_d            d;

// Define Table Name and Action Names
#define table_name         sacl_ip
#define action_keys32b     match_ipv4
#define action_data32b     match_ipv4_retrieve
#define action_keys128b    match_ipv6
#define action_data128b    match_ipv6_retrieve

// Define table field names for the selected key-widths
#define keys32b(a)         d.u.match_ipv4_d.key ## a
#define keys32b_(a)        d.u.match_ipv4_retrieve_d.key ## a
#define data32b(a)         d.u.match_ipv4_retrieve_d.data ## a
#define keys128bhi(a)      d.u.match_ipv6_d.key ## a[127:64]
#define keys128blo(a)      d.u.match_ipv6_d.key ## a[63:0]
#define keys128bhi_(a)     d.u.match_ipv6_retrieve_d.key ## a[127:64]
#define keys128blo_(a)     d.u.match_ipv6_retrieve_d.key ## a[63:0]
#define data128b(a)        d.u.match_ipv6_retrieve_d.data ## a

// Define key field names
#define key                k.sacl_metadata_ip
#define keylo              k.sacl_metadata_ip[63:0]
#define keyhi              k.sacl_metadata_ip[127:64]
#define base_addr          k.sacl_metadata_ip_table_addr
#define curr_addr          k.sacl_metadata_ip_table_addr_next

// Define PHV field names
#define next_addr          p.sacl_metadata_ip_table_addr_next

// Define result register and handler function name
#define res_reg            r7
#define res_handler        sacl_ip_handler

%%

#include "../include/lpm.h"

res_handler:
    or              r2, res_reg, k.sacl_metadata_sport_class_id, 10
    div             r6, r2, 51
    phvwr           p.sacl_metadata_ip_sport_class_id, r2
    add             r1, r0, k.p4_to_rxdma_header_sacl_base_addr_s0_e25, 8
    add             r1, r1, k.p4_to_rxdma_header_sacl_base_addr_s26_e33
    add             r1, r1, SACL_P1_TABLE_OFFSET
    add.e           r1, r1, r6, 6
    phvwr           p.sacl_metadata_p1_table_addr, r1
