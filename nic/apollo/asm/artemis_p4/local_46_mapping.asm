#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_local_46_mapping_k.h"

struct local_46_mapping_k_ k;
struct local_46_mapping_d  d;
struct phv_ p;

%%

local_46_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_46_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
