#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_ipv4_lpm_s2_k  k;
struct slacl_ipv4_lpm_s2_d  d;

%%

slacl_ipv4_lpm_s2:
    add     r1, r0, k.{p4_to_rxdma_header_slacl_base_addr_sbit0_ebit1... \
                       p4_to_rxdma_header_slacl_base_addr_sbit2_ebit33}
    add     r2, r1, SLACL_P1_TABLE_OFFSET
    add     r2, r2, k.slacl_metadata_sport_class_id, 10
    add     r2, r2, d.slacl_ipv4_lpm_s2_d.data[9:0]
    phvwr   p.slacl_metadata_p1_addr, r2
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ipv4_lpm_s2_error:
    nop.e
    nop
