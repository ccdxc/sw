#include "artemis_txdma.h"
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
    add         r3, d.{read_qstate_info_d.ring1_base}.dx, \
                    d.read_qstate_info_d.sw_cindex0, 6

    // update sw_cindex0, unlock
    tblmincri.f d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.ring_size}.hx, 1

    phvwr       p.app_header_table3_valid, TRUE
    phvwr       p.txdma_control_rxdma_cindex_addr, d.{read_qstate_info_d.rxdma_cindex_addr}.dx
    phvwr       p.txdma_control_cindex, d.read_qstate_info_d.sw_cindex0
    phvwr       p.txdma_control_control_addr, r3
    add.e       r2, r2, d.{read_qstate_info_d.ring0_base}.dx
    phvwr.f     p.txdma_control_payload_addr, r2

txdma_q_empty:
    phvwr.e     p.capri_intr_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_qstate_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
