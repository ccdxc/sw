#include "../../p4/include/lpm_defines.h"

#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct route_ipv4_k        k;
struct route_ipv4_d        d;

#define action_keys        search_routes32b
#define action_data        search_routes32b_retrieve
#define keys(a)            d.u.search_routes32b_d.key ## a
#define keys_(a)           d.u.search_routes32b_retrieve_d.key ## a
#define data(a)            d.u.search_routes32b_retrieve_d.data ## a
#define key                k.p4_to_txdma_header_lpm_dst
#define base_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}
#define curr_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}
#define next_addr          p.txdma_control_lpm_addr
#define res_field          p.txdma_to_p4e_header_nexthop_index
#define res_handler        route_ipv4_handler

#include "../include/lpm32b.h"

res_handler:
    phvwr.e         res_field, r7
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(action_name):
    phvwr.e         p.capri_intr_drop, 1
    nop
