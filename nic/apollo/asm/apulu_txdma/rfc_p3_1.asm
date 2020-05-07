#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "INGRESS_rfc_p3_1_k.h"

struct phv_                p;
struct rfc_p3_1_k_         k;

%%

rfc_p3_1:
    /* Compute the index into the classid array */
    mod        r7, k.txdma_control_rfc_index, SACL_P3_ENTRIES_PER_CACHE_LINE
    mul        r7, r7, SACL_P3_ENTRY_WIDTH
    /* Access the classid at the index */
    tblrdp     r7, r7, 0, SACL_P3_ENTRY_WIDTH
    /* Priority = r7 >> SACL_P3_ENTRY_PRIORITY_SHIFT */
    srl        r1, r7, SACL_P3_ENTRY_PRIORITY_SHIFT
    /* Action = r7 & SACL_P3_ENTRY_ACTION_MASK */
    and        r2, r7, SACL_P3_ENTRY_ACTION_MASK

    /* Set c2 if table constant is FW_ACTION_XPOSN_ANY_DENY */
    seq        c2, r5, FW_ACTION_XPOSN_ANY_DENY
    /* Set c3 if current action is deny */
    seq        c3, r2, SACL_P3_ENTRY_ACTION_DENY
    /* Set c4 if previous action is deny */
    seq        c4, k.txdma_to_p4e_drop, SACL_P3_ENTRY_ACTION_DENY
    /* Set c5 if current priority is higher that previous */
    slt        c5, r1, k.txdma_control_rule_priority

    /* previous is allow OR better priority */
    setcf      c6, [!c4 | c5]
    /* NOT any_deny_is_deny AND better priority? */
    setcf      c1, [!c2 & c5]
    /* any_deny_is_deny, current is deny, previous is allow OR better priority */
    orcf       c1, [c2 & c3 & c6]
    /* any_deny_is_deny, current is allow, previous is allow AND better priority */
    orcf       c1, [c2 & !c3 & !c4 & c5]

    /* Update PHV with new priority and action if c1 is set */
    phvwr.c1   p.txdma_control_rule_priority, r1
    phvwr.c1   p.txdma_to_p4e_drop, r2
    phvwr.c1   p.txdma_to_p4e_sacl_action, r2
    phvwr.c1   p.txdma_to_p4e_sacl_root_num, k.txdma_control_recirc_count[3:1]

    /* Are we done with SACLs to process... ? */
    seq        c1, k.rx_to_tx_hdr_sacl_base_addr0, r0
    phvwr.c1.e p.txdma_predicate_rfc_enable, FALSE

    /* Load sacl base addr to r1 */
    add        r1, k.rx_to_tx_hdr_sacl_base_addr0, r0
    /* Is this an even numbered pass? */
    seq        c1, k.txdma_control_recirc_count[0:0], r0
    /* if so, add SACL_P1_3_TABLE_OFFSET to sacl base address. */
    addi.c1    r1, r1, SACL_P1_3_TABLE_OFFSET
    /* P1 table index = (dtag_classid | (sip_classid0 << 7)). */
    add.c1     r2, k.txdma_control_dtag_classid, k.rx_to_tx_hdr_sip_classid0, \
                                                 SACL_TAG_CLASSID_WIDTH
    /* Else, add SACL_P1_1_TABLE_OFFSET to sacl base address. */
    addi.!c1   r1, r1, SACL_P1_1_TABLE_OFFSET
    /* P1 table index = (sport_classid0 | (sip_classid0 << 7)). */
    add.!c1    r2, k.rx_to_tx_hdr_sport_classid0, k.rx_to_tx_hdr_sip_classid0, \
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
rfc_p3_1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
