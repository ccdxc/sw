#include "remote_vnic_mapping_rx.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_rx_ohash_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
