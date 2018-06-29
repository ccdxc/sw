#include "apollo.h"

%%

nop:
    nop.e
    nop

native_ipv4_packet:
    nop.e
    nop

native_ipv6_packet:
    nop.e
    nop

native_nonip_packet:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_native_error:
    nop.e
    nop
