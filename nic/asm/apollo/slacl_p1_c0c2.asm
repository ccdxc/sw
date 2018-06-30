#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_p1_c0c2_k k;
struct slacl_p1_c0c2_d d;
struct phv_ p;

%%

slacl_p1_c0c2:
    add             r7, k.slacl_metadata_class_id2, \
                        k.slacl_metadata_class_id0, 8
    mod             r7, r7, 51
    mul             r7, r7, 10
    tblrdp.e        r2, r7, 0, 10
    phvwr           p.slacl_metadata_class_id5, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p1_c0c2_error:
    nop.e
    nop
