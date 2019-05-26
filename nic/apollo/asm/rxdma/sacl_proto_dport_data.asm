#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_proto_dport_data_k.h"

struct phv_                       p;
struct sacl_proto_dport_data_k_   k;
struct sacl_proto_dport_data_d    d;

// Define Table Name and Action Names
#define table_name         sacl_proto_dport_keys
#define action_data32b     match_proto_dport_retrieve

// Define table field names for the selected key-widths
#define keys32b_(a)        d.match_proto_dport_retrieve_d.key ## a
#define data32b(a)         d.match_proto_dport_retrieve_d.data ## a

// Define key field names
#define key                k.sacl_metadata_proto_dport

// Define result register and handler function name
#define res_reg            r7
#define result_handler32b  sacl_proto_dport_handler

%%

#include "../include/lpm.h"

sacl_proto_dport_handler:
    add             r1, r0, k.p4_to_rxdma_header_sacl_base_addr
    add             r1, r1, SACL_P2_TABLE_OFFSET
    add             r1, r1, k.sacl_metadata_p1_class_id, 6
    phvwr.e         p.sacl_metadata_p2_table_addr, r1
    phvwr           p.sacl_metadata_proto_dport_class_id, res_reg
