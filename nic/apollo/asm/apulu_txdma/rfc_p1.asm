#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "INGRESS_rfc_p1_k.h"

struct phv_                p;
struct rfc_p1_k_           k;

%%

rfc_p1:
    /* Compute the index into the classid array */
    mod        r7, k.txdma_control_rfc_index, SACL_P1_ENTRIES_PER_CACHE_LINE
    mul        r7, r7, SACL_P1_CLASSID_WIDTH
    /* Access the classid at the index */
    tblrdp     r7, r7, 0, SACL_P1_CLASSID_WIDTH
    /* Write the classid to phv */
    phvwr      p.txdma_control_rfc_p1_classid, r7
    /* Load sacl base addr to r1 */
    add        r1, r0, k.rx_to_tx_hdr_sacl_base_addr0
    /* Is this an even numbered pass? */
    seq        c1, k.txdma_control_recirc_count[0:0], r0
    /* If so, add SACL_P2_1_TABLE_OFFSET to sacl base address. */
    addi.c1    r1, r1, SACL_P2_1_TABLE_OFFSET
    /* P2 table index = (proto_dport_classid0 | (dip_classid0 << 8)). */
    add.c1     r2, k.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_dip_classid0, \
                                                 SACL_PROTO_DPORT_CLASSID_WIDTH
    /* Else, add SACL_P2_3_TABLE_OFFSET to sacl base address. */
    addi.!c1   r1, r1, SACL_P2_3_TABLE_OFFSET
    /* P2 table index = (proto_dport_classid0 | (sport_classid0 << 8)). */
    add.!c1    r2, k.rx_to_tx_hdr_dport_classid0, k.rx_to_tx_hdr_sport_classid0, \
                                                 SACL_PROTO_DPORT_CLASSID_WIDTH
    /* Write P2 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P2 table index */
    div        r2, r2, SACL_P2_ENTRIES_PER_CACHE_LINE
    mul        r2, r2, SACL_CACHE_LINE_SIZE
    /* Add the byte offset to table base */
    add.e      r1, r1, r2
    /* Write the address back to phv for P2 lookup */
    phvwr      p.txdma_control_rfc_table_addr, r1


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rfc_p1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
