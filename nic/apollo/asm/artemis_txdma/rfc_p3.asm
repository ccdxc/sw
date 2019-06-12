#include "../../p4/include/artemis_sacl_defines.h"
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
    phvwr.c1   p.session_info_hint_drop, r2

    /* Load sacl base addr to r1 */
    add        r1, r0, k.rx_to_tx_hdr_sacl_base_addr
    /* Is this the first pass? */
    seq        c1, k.capri_p4_intr_recirc_count, 0
    /* If so, add SACL_P1_2_TABLE_OFFSET to sacl base address. */
    addi.c1    r1, r1, SACL_P1_2_TABLE_OFFSET
    /* P1 table index = ((stag_classid << 10) | dip_classid). */
    add.c1     r2, k.rx_to_tx_hdr_dip_classid, k.rx_to_tx_hdr_stag_classid, 10
    /* Else, add SACL_P1_4_TABLE_OFFSET to sacl base address. */
    addi.!c1   r1, r1, SACL_P1_4_TABLE_OFFSET
    /* P1 table index = ((stag_classid << 7) | sport_classid). */
    add.!c1    r2, k.rx_to_tx_hdr_sport_classid, k.rx_to_tx_hdr_stag_classid, 7
    /* Write P1 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P1 table index */
    div        r2, r2, 51
    mul        r2, r2, 64
    /* Add the byte offset to table base */
    add.e      r1, r1, r2
    /* Write the address back to phv for P1 lookup */
    phvwr      p.txdma_control_rfc_table_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rfc_p3_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
