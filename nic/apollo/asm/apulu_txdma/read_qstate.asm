#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s0_t0_k.h"

struct phv_                 p;
struct tx_table_s0_t0_k_    k;
struct read_qstate_d        d;

%%

read_qstate_info:
    sne         c1, k.capri_p4_intr_recirc_count, 0
    nop.c1.e
    seq         c2, d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.p_index0}.hx
    bcf         [c2], txdma_q_empty

    // Compute pktdesc offset based on current cindex
    mul         r2, d.read_qstate_info_d.sw_cindex0, PKTQ_PAGE_SIZE
    add         r3, d.{read_qstate_info_d.ring1_base}.dx, \
                    d.read_qstate_info_d.sw_cindex0, APULU_PKT_DESC_SHIFT

    // update sw_cindex0, unlock
    tblmincri.f d.read_qstate_info_d.sw_cindex0, \
                    d.{read_qstate_info_d.ring_size}.hx, 1

    phvwr       p.txdma_predicate_cps_path_en, TRUE
    phvwr       p.txdma_control_rxdma_cindex_addr, d.{read_qstate_info_d.rxdma_cindex_addr}.dx
    phvwr       p.txdma_control_cindex, d.read_qstate_info_d.sw_cindex0
    phvwr       p.txdma_control_pktdesc_addr1, r3

    addi        r3, r3, 64
    phvwr       p.txdma_control_pktdesc_addr2, r3

    add.e       r2, r2, d.{read_qstate_info_d.ring0_base}.dx
    phvwr.f     p.txdma_control_payload_addr, r2

txdma_q_empty:
    nop.e
    phvwr.f     p.capri_intr_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_qstate_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
