#include "remote_vnic_mapping_tx.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_tx_ohash_error:
    nop.e
    nop
