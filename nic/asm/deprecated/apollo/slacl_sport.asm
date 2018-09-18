#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_sport_k k;
struct slacl_sport_d d;
struct phv_ p;

%%

slacl_sport:
    add             r1, d.slacl_sport_d.class_id, k.slacl_metadata_class_id0, 8
    div             r7, r1, 51
    phvwr           p.slacl_metadata_class_id2, d.slacl_sport_d.class_id
    add             r1, r0, k.slacl_metadata_base_addr
    addi            r1, r1, SLACL_P1_C0C2_TABLE_OFFSET
    add.e           r1, r1, r7, 6
    phvwr           p.slacl_metadata_addr1, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_sport_error:
    nop.e
    nop
