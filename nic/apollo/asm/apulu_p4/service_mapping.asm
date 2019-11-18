#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_service_mapping_k.h"

struct service_mapping_k_   k;
struct service_mapping_d    d;
struct phv_                 p;


%%

service_mapping_info:
    nop.e
    phvwr.c1    p.p4i_to_arm_service_xlate_id, d.service_mapping_info_d.xlate_id

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
service_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
