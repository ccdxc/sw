#include "../../p4/include/artemis_sacl_defines.h"
#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "INGRESS_rfc_p1_1_k.h"

struct phv_                p;
struct rfc_p1_1_k_         k;

%%

rfc_p1_1:
    /* Compute the index into the classid array */
    mod        r7, k.txdma_control_rfc_index, 51
    mul        r7, r7, 10
    /* Access the classid at the index */
    tblrdp     r7, r7, 0, 10
    /* Write the classid to phv */
    phvwr      p.txdma_control_rfc_p1_classid, r7
    /* Load sacl base addr to r1 */
    add        r1, r0, k.rx_to_tx_hdr_sacl_base_addr
    /* Is this the first pass? */
    seq        c1, k.capri_p4_intr_recirc_count, 0
    /* If so, add SACL_P2_2_TABLE_OFFSET to sacl base address. */
    addi.c1    r1, r1, SACL_P2_2_TABLE_OFFSET
    /* P2 table index = ((sport_classid << 8) | proto_dport_classid). */
    add.c1     r2, k.rx_to_tx_hdr_dport_classid, k.rx_to_tx_hdr_sport_classid, 8
    /* Else, add SACL_P2_4_TABLE_OFFSET to sacl base address. */
    addi.!c1   r1, r1, SACL_P2_4_TABLE_OFFSET
    /* P2 table index = ((tag_classid << 8) | proto_dport_classid). */
    add.!c1    r2, k.rx_to_tx_hdr_dport_classid, k.rx_to_tx_hdr_tag_classid, 8
    /* Write P2 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P2 table index */
    div        r2, r2, 51
    mul        r2, r2, 64
    /* Add the byte offset to table base */
    add.e      r1, r1, r2
    /* Write the address back to phv for P2 lookup */
    phvwr      p.txdma_control_rfc_table_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rfc_p1_1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
