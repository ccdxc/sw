#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_ip_data_k.h"

struct phv_                 p;
struct sacl_ip_data_k_      k;
struct sacl_ip_data_d       d;

// Define Table Name and Action Names
#define table_name         sacl_ip_data
#define action_data32b     match_ipv4_retrieve

// Define table field names for the selected key-widths
#define keys32b_(a)        d.match_ipv4_retrieve_d.key ## a
#define data32b(a)         d.match_ipv4_retrieve_d.data ## a

// Define key field names
#define key                k.sacl_metadata_ip
#define base_addr          k.{sacl_metadata_ipv4_table_addr_sbit0_ebit1,\
                              sacl_metadata_ipv4_table_addr_sbit2_ebit33}
#define curr_addr          k.{sacl_metadata_ipv4_table_addr_next_sbit0_ebit1,\
                              sacl_metadata_ipv4_table_addr_next_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.sacl_metadata_ipv4_table_addr_next

// Define result register and handler function name
#define res_reg            r7
#define res_handler        sacl_ip_handler

%%

#include "../include/lpm.h"

res_handler:
    or              r2, res_reg, k.sacl_metadata_sport_class_id, 10
    div             r6, r2, 51
    phvwr           p.sacl_metadata_ip_sport_class_id, r2
    add             r1, r0, k.p4_to_rxdma_header_sacl_base_addr
    add             r1, r1, SACL_P1_TABLE_OFFSET
    add.e           r1, r1, r6, 6
    phvwr           p.sacl_metadata_p1_table_addr, r1
