#include "apollo.h"

%%

nop:
    nop.e
    nop

.align
tunneled_ipv4_packet:
    nop.e
    nop

.align
tunneled_ipv6_packet:
    nop.e
    nop

.align
tunneled_nonip_packet:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    nop.e
    nop
