#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "INGRESS_rfc_p3_k.h"

struct phv_                p;
struct rfc_p3_k_           k;

%%

rfc_p3:
    /* Compute the index into the classid array */
    mod        r7, k.txdma_control_rfc_index, SACL_P3_ENTRIES_PER_CACHE_LINE
    mul        r7, r7, SACL_P3_ENTRY_WIDTH
    /* Access the classid at the index */
    tblrdp     r7, r7, 0, SACL_P3_ENTRY_WIDTH
    /* Priority = r7 >> SACL_P3_ENTRY_PRIORITY_SHIFT */
    srl        r1, r7, SACL_P3_ENTRY_PRIORITY_SHIFT
    /* Action = r7 & SACL_P3_ENTRY_ACTION_MASK */
    and        r2, r7, SACL_P3_ENTRY_ACTION_MASK
    /* Is Priority higher (numerically less) than that in PHV? */
    slt        c1, r1, k.txdma_control_rule_priority
    /* If so, update PHV with new priority and action */
    phvwr.c1   p.txdma_control_rule_priority, r1
    phvwr.c1   p.txdma_to_p4e_drop, r2
    phvwr.c1   p.txdma_to_p4e_sacl_action, r2
    phvwr.c1   p.txdma_to_p4e_sacl_root_num, k.txdma_control_recirc_count[3:1]

    /* Load sacl base addr to r1 */
    add        r1, r0, k.rx_to_tx_hdr_sacl_base_addr0
    /* Is this an even numbered pass? */
    seq        c1, k.txdma_control_recirc_count[0:0], r0
    /* If so, add SACL_P1_2_TABLE_OFFSET to sacl base address. */
    addi.c1    r1, r1, SACL_P1_2_TABLE_OFFSET
    /* P1 table index = (dip_classid0 | (stag_classid << 10)). */
    add.c1     r2, k.rx_to_tx_hdr_dip_classid0, k.txdma_control_stag_classid, \
                                               SACL_DIP_CLASSID_WIDTH
    /* Else, add SACL_P1_4_TABLE_OFFSET to sacl base address. */
    addi.!c1   r1, r1, SACL_P1_4_TABLE_OFFSET
    /* P1 table index = (sport_classid0 | (stag_classid << 7)). */
    add.!c1    r2, k.rx_to_tx_hdr_sport_classid0, k.txdma_control_stag_classid, \
                                                 SACL_SPORT_CLASSID_WIDTH
    /* Write P1 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P1 table index */
    div        r2, r2, SACL_P1_ENTRIES_PER_CACHE_LINE
    mul        r2, r2, SACL_CACHE_LINE_SIZE
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
