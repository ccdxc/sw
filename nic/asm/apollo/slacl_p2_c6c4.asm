#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_p2_c6c4_k k;
struct slacl_p2_c6c4_d d;
struct phv_ p;

%%

slacl_p2_c6c4:
    add             r7, k.slacl_metadata_class_id4, \
                        k.{slacl_metadata_class_id6_sbit0_ebit7, \
                           slacl_metadata_class_id6_sbit8_ebit9}, 4
    mod             r7, r7, 51
    mul             r7, r7, 10
    tblrdp          r2, r7, 0, 10
    phvwr           p.slacl_metadata_class_id7, r2
    add             r1, r2, k.slacl_metadata_class_id5, 10
    add             r1, r1, k.slacl_metadata_base_addr
    addi.e          r1, r1, SLACL_P3_TABLE_OFFSET
    phvwr           p.slacl_metadata_addr1, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p2_c6c4_error:
    nop.e
    nop
