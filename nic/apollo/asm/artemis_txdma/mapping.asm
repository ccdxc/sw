#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct mapping_k  k;
struct mapping_d  d;
struct phv_       p;

%%

mapping_info:
    bbne        d.mapping_info_d.entry_valid, TRUE, mapping_miss
    // Set bit 31 for overflow hash lookup
    ori         r7, r0, 0x80000000
    bcf         [c1], mapping_hit

    //hint1
    seq         c1, r1[31:21], d.mapping_info_d.hash1
    //hint1 if formed into r2
    or          r2, d.mapping_info_d.hint1_sbit4_ebit17, \
                    d.mapping_info_d.hint1_sbit0_ebit3, 18
    sne         c2, r2, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, r2
    //hint2
    seq         c1, r1[31:21], d.mapping_info_d.hash2
    sne         c2, d.mapping_info_d.hint2, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint2
    //hint3
    seq         c1, r1[31:21], d.mapping_info_d.hash3
    sne         c2, d.mapping_info_d.hint3, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint3
    //hint4
    seq         c1, r1[31:21], d.mapping_info_d.hash4
    sne         c2, d.mapping_info_d.hint4, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint4
    //hint5
    seq         c1, r1[31:21], d.mapping_info_d.hash5
    sne         c2, d.mapping_info_d.hint5, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint5
    //hint6
    seq         c1, r1[31:21], d.mapping_info_d.hash6
    sne         c2, d.mapping_info_d.hint6, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint6
    //hint7
    seq         c1, r1[31:21], d.mapping_info_d.hash7
    sne         c2, d.mapping_info_d.hint7, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint7
    //hint8
    seq         c1, r1[31:21], d.mapping_info_d.hash8
    sne         c2, d.mapping_info_d.hint8, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.hint8
    //hintn
    seq         c1, d.mapping_info_d.more_hashes, TRUE
    sne         c2, d.mapping_info_d.more_hints, r0
    bcf         [c1&c2], mapping_hash_hit
    add         r7, r7, d.mapping_info_d.more_hints
mapping_miss:
    nop.e
    nop

mapping_hit:
    phvwr.e     p.txdma_control_nexthop_group_index, \
                d.mapping_info_d.nexthop_group_index
    nop

mapping_hash_hit:
    phvwr.e     p.txdma_control_mapping_ohash, r7
    phvwr       p.txdma_control_mapping_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mapping_info_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
