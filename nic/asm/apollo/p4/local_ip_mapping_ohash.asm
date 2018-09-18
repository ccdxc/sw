#include "local_ip_mapping.asm"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_ip_mapping_ohash_error:
    nop.e
    nop
