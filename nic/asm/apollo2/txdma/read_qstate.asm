#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct read_qstate_k        k;
struct read_qstate_d        d;

%%

read_qstate_info:
    seq         c2, d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.p_index0}.hx
    bcf         [c2], txdma_q_empty

    // Compute control offset based on current cindex
    mul         r2, d.read_qstate_info_d.sw_cindex0, PKTQ_PAGE_SIZE

    // Increment sw_cindex0
    // update sw_cindex0, unlock
    tblmincri.f d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.ring_size0}.hx, 1

    add         r2, r2, d.{read_qstate_info_d.ring_base0}.dx
    phvwr       p.txdma_control_control_addr, r2
    // payload_addr will be after predicate header and p4_to_txdma_header
    addi        r3, r2, (APOLLO_PREDICATE_HDR_SZ + APOLLO_P4_TO_TXDMA_HDR_SZ)
    phvwr.e     p.txdma_control_payload_addr, r3
    phvwr       p.txdma_control_cindex, d.read_qstate_info_d.sw_cindex0

txdma_q_empty:
    phvwr.e     p.predicate_header_txdma_drop_event, TRUE
    phvwr       p.capri_intr_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_qstate_error:
    nop.e
    nop
