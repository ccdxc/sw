#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_ipv4_data_k.h"

struct phv_                p;
struct sacl_ipv4_data_k_   k;
struct sacl_ipv4_data_d    d;

#define action_name        search_sacl_ip32b_retrieve
#define key                k.sacl_metadata_ip
#define keys(a)            d.search_sacl_ip32b_retrieve_d.key ## a
#define data(a)            d.search_sacl_ip32b_retrieve_d.data ## a
#define res_handler        sacl_ipv4_handler

%%

#include "../include/lpm32b_data.h"

res_handler:
    or              r2, r7, k.sacl_metadata_sport_class_id, 10
    div             r6, r2, 51
    phvwr           p.sacl_metadata_ip_sport_class_id, r2
    add             r1, r0, k.p4_to_rxdma_header_sacl_base_addr
    add             r1, r1, SACL_P1_TABLE_OFFSET
    add.e           r1, r1, r6, 6
    phvwr           p.sacl_metadata_p1_table_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(action_name):
    phvwr.e         p.capri_intr_drop, 1
    nop
