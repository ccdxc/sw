#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_p2_k           k;
struct slacl_p2_d           d;

%%

slacl_p2:
    add             r7, r0, k.slacl_metadata_proto_dport_class_id, 1
    tblrdp.e        r1, r7, 0, 2
    phvwr           p.p4_to_rxdma_header_slacl_result, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p2_error:
    nop.e
    nop
