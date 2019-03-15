#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                        p;
struct sacl_proto_dport_keys_k     k;
struct sacl_proto_dport_keys_d     d;

#define action_name         search_sacl_proto_dport
#define keys(a)             d.search_sacl_proto_dport_d.key ## a
#define key                 k.sacl_metadata_proto_dport
#define base_addr           k.{sacl_metadata_proto_dport_table_addr_sbit0_ebit1,\
                               sacl_metadata_proto_dport_table_addr_sbit2_ebit33}
#define curr_addr           k.{sacl_metadata_proto_dport_table_addr_sbit0_ebit1,\
                               sacl_metadata_proto_dport_table_addr_sbit2_ebit33}
#define next_addr           p.sacl_metadata_proto_dport_table_addr_next

#include "../include/lpm32b_keys.h"
