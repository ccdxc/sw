#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_         p;
struct sacl_p2_k    k;
struct sacl_p2_d    d;

%%

sacl_p2:
    add             r7, r0, k.sacl_metadata_proto_dport_class_id, 1
    tblrdp.e        r1, r7, 0, 1
    phvwr           p.p4_to_rxdma_header_sacl_result, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
sacl_p2_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
