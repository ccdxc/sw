#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct slacl_sport_lpm_s1_k k;
struct slacl_sport_lpm_s1_d d;
struct phv_                 p;

#define keys(a)             d.slacl_sport_lpm_s1_d.key ## a
#define data(a)             d.slacl_sport_lpm_s1_d.data ## a
#define key                 k.p4_to_rxdma_header_flow_sport

%%

slacl_sport_lpm_s1:
    slt             c1, key, keys(7)
    bcf             [c1], lessthan7
    slt             c1, key, keys(11)
    bcf             [c1], lessthan11
    slt             c1, key, keys(13)
    bcf             [c1], lessthan13
    slt             c1, key, keys(14)
    cmov.e          r7, c1, data(14), data(15)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan13:
    slt             c1, key, keys(12)
    cmov.e          r7, c1, data(12), data(13)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan11:
    slt             c1, key, keys(9)
    bcf             [c1], lessthan9
    slt             c1, key, keys(10)
    cmov.e          r7, c1, data(10), data(11)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan9:
    slt             c1, key, keys(8)
    cmov.e          r7, c1, data(8), data(9)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan7:
    slt             c1, key, keys(3)
    bcf             [c1], lessthan3
    slt             c1, key, keys(5)
    bcf             [c1], lessthan5
    slt             c1, key, keys(6)
    cmov.e          r7, c1, data(6), data(7)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan5:
    slt             c1, key, keys(4)
    cmov.e          r7, c1, data(4), data(5)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan3:
    slt             c1, key, keys(1)
    bcf             [c1], lessthan1
    slt             c1, key, keys(2)
    cmov.e          r7, c1, data(2), data(3)
    phvwr           p.slacl_metadata_sport_class_id, r7

lessthan1:
    slt             c1, key, keys(0)
    cmov.e          r7, c1, data(0), data(1)
    phvwr           p.slacl_metadata_sport_class_id, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_sport_lpm_s1_error:
    nop.e
    nop
