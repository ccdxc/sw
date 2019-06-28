#include "artemis_txdma.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_txdma_flow_k.h"

struct txdma_flow_k_ k;
struct txdma_flow_d  d;
struct phv_ p;

#define FLOW_HASH_MSB 31:23
#define FLOW_FLIT_TO_PHV_RANGE(flit) p[(((flit+1)*512)-1):(flit*512)]
#define FLOW_PARENT_FLIT    FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_PARENT_FLIT)
#define FLOW_LEAF_FLIT      FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_LEAF_FLIT)
%%

flow_hash:
    /* Without predicate bits, we need to validate if the key is indeed valid 
        and is v6
    */
    sne         c2, k.key3_flow_lkp_type, 6
    b.c2        label_key_invalid
    nop

    bbne        d.txdma_flow_hash_d.entry_valid, TRUE, label_flow_miss
    nop

    bcf         [c1], label_flow_hit
    // Check hash1 and hint1
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_flow_hash_d.hash1
    sne         c2, d.txdma_flow_hash_d.hint1, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_flow_hash_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_flow_hash_d.hash2
    sne         c2, d.txdma_flow_hash_d.hint2, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_flow_hash_d.hint2
    // Check hash3 and hint3
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_flow_hash_d.hash3
    sne         c2, d.txdma_flow_hash_d.hint3, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_flow_hash_d.hint3
    // Check hash4 and hint4
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_flow_hash_d.hash4
    sne         c2, d.txdma_flow_hash_d.hint4, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_flow_hash_d.hint4
    // Check for more hashes
    seq         c1, d.txdma_flow_hash_d.more_hashes, TRUE
    sne         c2, d.txdma_flow_hash_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_flow_hash_d.more_hints

    /* Entry valid but, no key match, no hint match, no more hints, nor linked hints (Hint entry scenario)
            - Relay Parent entry (D-vector) 
            - Relay new flow key (K-vector)
    */

    /* All done with the tables, set flow key to invalid */
    phvwr       p.key3_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE

    phvwr       FLOW_PARENT_FLIT, d.{txdma_flow_hash_d.entry_valid...txdma_flow_hash_d.__pad_to_512b}

    tblwr.l     d[511:504], k.key3_epoch
    tblwr.l     d[471:464], k.key1_proto
    tblwr.l     d[463:416], k.key1_src[111:64]
    tblwr.l     d[415:352], k.key1_src[63:0]
    tblwr.l     d[351:336], k.key2_src[15:0]
    tblwr.l     d[335:272], k.key2_dst[79:16]
    tblwr.l     d[271:256], k.key2_dst[15:0]
    tblwr.l     d[255:208], k.key3_dst[47:0]
    tblwr.l     d[207:192], k.key3_dport
    tblwr.l     d[191:176], k.key3_sport
    tblwr.l     d[175:168], k.key3_vpc_id
    tblwr.l     d[167:166], k.key3_ktype

    phvwr       FLOW_LEAF_FLIT, d.{txdma_flow_hash_d.entry_valid...txdma_flow_hash_d.__pad_to_512b}

    add         r2, r0, k.key1_flow_ohash
    andi        r3, r2, 0x80000000
    sne         c2, r3, 0

    /* Hint entry specific */
    //phvwr.c2    p.txdma_to_arm_meta_rflow_parent_is_hint, TRUE
    andi.c2     r3, r2, ~(0x80000000)
    phvwrpair.c2 p.txdma_to_arm_meta_rflow_parent_is_hint, TRUE, \
                p.txdma_to_arm_meta_rflow_parent_index, r3

    phvwrpair   p.txdma_to_arm_meta_rflow_hash, r1, \
                p.txdma_to_arm_meta_rflow_ipaf, 1

    seq         c2, r0, d.txdma_flow_hash_d.hint1
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 1

    seq         c2, r0, d.txdma_flow_hash_d.hint2
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 2

    seq         c2, r0, d.txdma_flow_hash_d.hint3
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 3

    seq         c2, r0, d.txdma_flow_hash_d.hint4
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 4
    nop


label_flow_miss:
    /* All done with the tables, set flow key to invalid */
    phvwr       p.key3_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE
    /* 
        Setup assist informaton for rflow
        - Key information
        - Main table miss indication
    */
    tblwr.l     d[511:504], k.key3_epoch
    tblwr.l     d[471:464], k.key1_proto
    tblwr.l     d[463:416], k.key1_src[111:64]
    tblwr.l     d[415:352], k.key1_src[63:0]
    tblwr.l     d[351:336], k.key2_src[15:0]
    tblwr.l     d[335:272], k.key2_dst[79:16]
    tblwr.l     d[271:256], k.key2_dst[15:0]
    tblwr.l     d[255:208], k.key3_dst[47:0]
    tblwr.l     d[207:192], k.key3_dport
    tblwr.l     d[191:176], k.key3_sport
    tblwr.l     d[175:168], k.key3_vpc_id
    tblwr.l     d[167:166], k.key3_ktype
    phvwr       FLOW_PARENT_FLIT, d.{txdma_flow_hash_d.entry_valid...txdma_flow_hash_d.__pad_to_512b}
    //phvwr       p.txdma_to_arm_meta_rflow_parent_index, r1
    phvwrpair.e   p.txdma_to_arm_meta_rflow_hash, r1, \
                  p.txdma_to_arm_meta_rflow_ipaf, 1
    nop

label_flow_hit:
    /* TODO: Possibly due to epoch check failure? */
    nop.e
    nop

label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr.e     p.key1_flow_ohash, r2
    nop

label_key_invalid:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
