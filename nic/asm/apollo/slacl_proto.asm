#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_proto_k k;
struct slacl_proto_d d;
struct phv_ p;

%%

slacl_proto:
    seq             c1, k.key_metadata_proto[0], TRUE
    phvwr.c1        p.slacl_metadata_class_id4, d.slacl_proto_d.class_id[7:4]
    phvwr.!c1       p.slacl_metadata_class_id4, d.slacl_proto_d.class_id[3:0]
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_proto_error:
    nop.e
    nop
