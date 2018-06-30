#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct init_config_k k;
struct phv_ p;

%%

init_config:
    div             r1, k.slacl_metadata_ip_31_16, 51
    div             r2, k.slacl_metadata_ip_15_00, 51
    add             r1, k.slacl_metadata_addr1, r1, 6
    add             r2, k.slacl_metadata_addr2, r2, 6
    phvwr.e         p.slacl_metadata_addr1, r1
    phvwr           p.slacl_metadata_addr2, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
init_config_error:
    nop.e
    nop
