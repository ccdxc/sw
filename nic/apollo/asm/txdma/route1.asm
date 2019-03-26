#include "../../p4/include/lpm_defines.h"

#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct route1_k            k;
struct route1_d            d;

#define table_name         route1
#define action_keys32b     search1_ipv4
#define action_keys64b     search1_ipv6
#define action_data32b     search1_ipv4_retrieve
#define action_data64b     search1_ipv6_retrieve
#define keys32b(a)         d.u.search1_ipv4_d.key ## a
#define keys64b(a)         d.u.search1_ipv6_d.key ## a
#define keys32b_(a)        d.u.search1_ipv4_retrieve_d.key ## a
#define keys64b_(a)        d.u.search1_ipv6_retrieve_d.key ## a
#define data32b(a)         d.u.search1_ipv4_retrieve_d.data ## a
#define data64b(a)         d.u.search1_ipv6_retrieve_d.data ## a
#define key                k.p4_to_txdma_header_lpm_dst
#define base_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}
#define curr_addr          k.{txdma_control_lpm_addr_sbit0_ebit1...\
                              txdma_control_lpm_addr_sbit2_ebit33}
#define next_addr          p.txdma_control_lpm_addr
#define res_field          p.txdma_to_p4e_header_nexthop_index
#define ctrl_field         p.app_header_table0_valid // LPM bypass
#define res_handler        route_ipv4_handler

%%

#include "../include/lpm.h"

res_handler:
    phvwr.e         res_field, r7
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(action_keys32b):
    phvwr.e         p.capri_intr_drop, 1
    nop
