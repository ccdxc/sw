#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct sacl_ip_keys1_k      k;
struct sacl_ip_keys1_d      d;

// Define Table Name and Action Names
#define table_name         sacl_ip_keys1
#define action_keys32b     match1_ipv4

// Define table field names for the selected key-widths
#define keys32b(a)         d.match1_ipv4_d.key ## a

// Define key field names
#define key                k.sacl_metadata_ip
#define base_addr          k.{sacl_metadata_ipv4_table_addr_sbit0_ebit1,\
                              sacl_metadata_ipv4_table_addr_sbit2_ebit33}
#define curr_addr          k.{sacl_metadata_ipv4_table_addr_next_sbit0_ebit1,\
                              sacl_metadata_ipv4_table_addr_next_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.sacl_metadata_ipv4_table_addr_next

%%

#include "../include/lpm.h"
