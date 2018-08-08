#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct service_header_k k;
struct phv_ p;

%%

service_header_info:
    xor             r1, k.service_header_local_ip_mapping_done, 0x1
    phvwr           p.control_metadata_local_ip_mapping_ohash_lkp, r1
    xor.e           r1, k.service_header_flow_done, 0x1
    phvwr           p.control_metadata_flow_ohash_lkp, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
service_header_error:
    nop.e
    nop
