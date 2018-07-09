#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_p1_c1c3_k k;
struct slacl_p1_c1c3_d d;
struct phv_ p;

%%

slacl_p1_c1c3:
    add             r7, k.{slacl_metadata_class_id3_sbit0_ebit5, \
                           slacl_metadata_class_id3_sbit6_ebit7}, \
                        k.{slacl_metadata_class_id1_sbit0_ebit7, \
                           slacl_metadata_class_id1_sbit8_ebit9}, 8
    mod             r7, r7, 51
    mul             r7, r7, 10
    tblrdp          r2, r7, 0, 10
    phvwr           p.slacl_metadata_class_id6, r2
    add             r2, k.slacl_metadata_class_id4, r2, 4
    div             r2, r2, 51
    add             r1, r0, k.slacl_metadata_base_addr
    addi            r1, r1, SLACL_P2_C6C4_TABLE_OFFSET
    add.e           r1, r1, r2, 6
    phvwr           p.slacl_metadata_addr1, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_p1_c1c3_error:
    nop.e
    nop
