#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                        p;
struct sacl_proto_dport_keys_k     k;
struct sacl_proto_dport_keys_d     d;

// Define Table Name and Action Names
#define table_name         sacl_proto_dport_keys
#define action_keys32b     match_proto_dport

// Define table field names for the selected key-widths
#define keys32b(a)         d.match_proto_dport_d.key ## a

// Define key field names
#define key                k.sacl_metadata_proto_dport
#define base_addr          k.{sacl_metadata_proto_dport_table_addr_sbit0_ebit1,\
                              sacl_metadata_proto_dport_table_addr_sbit2_ebit33}
#define curr_addr          k.{sacl_metadata_proto_dport_table_addr_sbit0_ebit1,\
                              sacl_metadata_proto_dport_table_addr_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.sacl_metadata_proto_dport_table_addr_next

%%

#include "../include/lpm.h"
