#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "INGRESS_rfc_p3_k.h"

struct phv_                p;
struct rfc_p3_k_           k;

%%

rfc_p3:
    /* Compute the index into the classid array */
    mod        r7, k.txdma_control_rfc_index, 46
    mul        r7, r7, 11
    /* Access the classid at the index */
    tblrdp     r7, r7, 0, 11
    /* Priority = r7 >> 1 */
    srl        r1, r7, 1
    /* Result = r7 & 0x01 */
    and        r2, r7, 1
    /* Is Priority higher (numerically less) than that in PHV? */
    slt        c1, r1, k.txdma_control_rule_priority
    /* If so, update PHV with new priority and result */
    phvwr.c1   p.txdma_control_rule_priority, r1
    phvwr.c1   p.txdma_control_sacl_result, r2
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rfc_p3_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
