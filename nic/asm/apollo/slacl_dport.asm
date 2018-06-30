#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct slacl_dport_k k;
struct slacl_dport_d d;
struct phv_ p;

%%

slacl_dport:
    add             r1, d.slacl_dport_d.class_id, \
                        k.{slacl_metadata_class_id1_sbit0_ebit7, \
                        slacl_metadata_class_id1_sbit8_ebit9}, 8
    div             r7, r1, 51
    phvwr           p.slacl_metadata_class_id3, d.slacl_dport_d.class_id
    add             r1, r0, k.slacl_metadata_base_addr
    addi            r1, r1, SLACL_P1_C1C3_TABLE_OFFSET
    add.e           r1, r1, r7
    phvwr           p.slacl_metadata_addr2, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_dport_error:
    nop.e
    nop
