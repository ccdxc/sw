#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_ipv4_lpm_s1_k  k;

%%

slacl_ipv4_lpm_s1:
    add    r1, k.{slacl_metadata_ipv4_addr_sbit0_ebit1, \
                  slacl_metadata_ipv4_addr_sbit2_ebit33}, 1, 6
    phvwr  p.slacl_metadata_ipv4_addr, r1
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ipv4_lpm_s1_error:
    nop.e
    nop
