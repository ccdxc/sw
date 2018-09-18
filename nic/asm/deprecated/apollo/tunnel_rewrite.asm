#include "apollo.h"

%%

nop:
    nop.e
    nop

.align
encap_vxlan:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_rewrite_error:
    nop.e
    nop
