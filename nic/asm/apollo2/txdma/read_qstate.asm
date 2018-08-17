#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct read_qstate_k        k;
struct read_qstate_d        d;

%%

read_qstate_info:
    seq         c2, d.read_qstate_info_d.sw_cindex0, \
                    d.read_qstate_info_d.p_index0
    bcf         [c2], txdma_q_empty

    // Compute control offset based on current cindex
    mul         r2, d.read_qstate_info_d.sw_cindex0, PKTQ_PAGE_SIZE

    // Increment sw_cindex0
    // update sw_cindex0, unlock
    tblmincri.f d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.ring_sz_mask0}.hx, 1

    add         r2, r2, d.{read_qstate_info_d.ring_base0}.dx
    phvwr       p.txdma_control_control_addr, r2
    // payload_addr will be after predicate header and p4_to_txdma_header
    addi        r3, r2, \
                    1 + CAPRI_PHV_END_OFFSET(predicate_header_txdma_drop_event) - \
                    CAPRI_PHV_START_OFFSET(p4_to_txdma_header_vcn_id)
    phvwr.e     p.txdma_control_payload_addr, r3
    phvwr       p.txdma_control_cindex, d.read_qstate_info_d.sw_cindex0

txdma_q_empty:
    phvwr.e     p.predicate_header_txdma_drop_event, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_qstate_error:
    nop.e
    nop
