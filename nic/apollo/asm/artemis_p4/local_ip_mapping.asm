#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_local_ip_mapping_k.h"

struct local_ip_mapping_k_ k;
struct local_ip_mapping_d  d;
struct phv_ p;

%%

local_ip_mapping_info:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_ip_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
