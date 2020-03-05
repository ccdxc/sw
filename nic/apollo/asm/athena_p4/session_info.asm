#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_session_info_k.h"

struct session_info_k_          k;
struct session_info_d           d;
struct phv_                     p;

%%

session_info:
    /* Do nothing for now */
    nop.e
    nop

session_info_failed:
    phvwr.e         p.control_metadata_flow_miss, TRUE
    nop
    


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
