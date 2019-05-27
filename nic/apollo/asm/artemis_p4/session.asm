#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_session_k.h"

struct session_k_ k;
struct session_d  d;
struct phv_ p;

%%

session_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
