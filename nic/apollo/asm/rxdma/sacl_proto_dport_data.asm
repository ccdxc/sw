#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_proto_dport_data_k.h"

struct phv_                       p;
struct sacl_proto_dport_data_k_   k;
struct sacl_proto_dport_data_d    d;

#define action_name        search_sacl_proto_dport_retrieve
#define key                k.sacl_metadata_proto_dport
#define keys(a)            d.search_sacl_proto_dport_retrieve_d.key ## a
#define data(a)            d.search_sacl_proto_dport_retrieve_d.data ## a
#define res_handler        sacl_proto_dport_handler

%%

#include "../include/lpm32b_data.h"

res_handler:
    add             r1, r0, k.p4_to_rxdma_header_sacl_base_addr
    add             r1, r1, SACL_P2_TABLE_OFFSET
    add             r1, r1, k.sacl_metadata_p1_class_id, 6
    phvwr.e         p.sacl_metadata_p2_table_addr, r1
    phvwr           p.sacl_metadata_proto_dport_class_id, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(action_name):
    phvwr.e         p.capri_intr_drop, 1
    nop
