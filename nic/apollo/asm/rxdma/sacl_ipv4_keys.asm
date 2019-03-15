#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct sacl_ipv4_keys_k     k;
struct sacl_ipv4_keys_d     d;

#define action_name         search_sacl_ip32b
#define keys(a)             d.search_sacl_ip32b_d.key ## a
#define key                 k.sacl_metadata_ip
#define base_addr           k.{sacl_metadata_ipv4_table_addr_sbit0_ebit1,\
                               sacl_metadata_ipv4_table_addr_sbit2_ebit33}
#define curr_addr           k.{sacl_metadata_ipv4_table_addr_sbit0_ebit1,\
                               sacl_metadata_ipv4_table_addr_sbit2_ebit33}
#define next_addr           p.sacl_metadata_ipv4_table_addr_next

#include "../include/lpm32b_keys.h"
