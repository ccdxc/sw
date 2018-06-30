#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_ip_31_16_k k;
struct slacl_ip_31_16_d d;
struct phv_ p;

%%

slacl_ip_31_16:
    mod             r7, k.slacl_metadata_ip_31_16, 51
    add             r1, k.slacl_metadata_base_addr, k.key_metadata_sport
    addi            r1, r1, SLACL_SPORT_TABLE_OFFSET
    phvwr           p.slacl_metadata_addr1, r1
    mul             r7, r7, 10
    tblrdp.e        r2, r7, 0, 10
    phvwr           p.slacl_metadata_class_id0, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ip_31_16_error:
    nop.e
    nop
