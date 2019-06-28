#include "artemis_txdma.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_txdma_ipv4_flow_k.h"

struct txdma_ipv4_flow_k_ k;
struct txdma_ipv4_flow_d  d;
struct phv_ p;

#define FLOW_HASH_MSB 31:23
#define FLOW_FLIT_TO_PHV_RANGE(flit) p[(((flit+1)*512)-1):(flit*512)]
#define FLOW_PARENT_FLIT    FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_PARENT_FLIT)
#define FLOW_LEAF_FLIT      FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_LEAF_FLIT)

%%

ipv4_flow_hash:
    /* Without predicate bits, we need to validate if the key is indeed valid */
    sne         c2, k.key2_ipv4_flow_lkp_type, 4
    b.c2        label_key_invalid
    nop

    bbne        d.txdma_ipv4_flow_hash_d.entry_valid, TRUE, label_flow_miss
    nop

    bcf         [c1], label_flow_hit
    // Check hash1 and hint1
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_ipv4_flow_hash_d.hash1
    sne         c2, d.txdma_ipv4_flow_hash_d.hint1, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_ipv4_flow_hash_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[FLOW_HASH_MSB], d.txdma_ipv4_flow_hash_d.hash2
    sne         c2, d.txdma_ipv4_flow_hash_d.hint2, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_ipv4_flow_hash_d.hint2
    // Check for more hashes
    seq         c1, d.txdma_ipv4_flow_hash_d.more_hashes, TRUE
    sne         c2, d.txdma_ipv4_flow_hash_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.txdma_ipv4_flow_hash_d.more_hints

label_2nd_level_flow_miss:
#if 0
    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint1
    phvwr.c2    p.p4_to_rxdma3_parent_hint_slot, 1

    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint2
    phvwr.c2    p.p4_to_rxdma3_parent_hint_slot, 2

    seq         c1, k.control_metadata_flow_ohash_lkp, 1
    phvwr.c1    p.p4_to_rxdma3_parent_is_hint, 1
    phvwr.c1    p.p4_to_rxdma3_parent_hint_index, k.service_header_flow_ohash[21:0].wx
    phvwr       p.p4_to_rxdma3_parent_valid, 1
#else
    /* Entry valid but, no key match, no hint match, no more hints, nor linked hints (Hint entry scenario)
            - Relay Parent entry (D-vector) 
            - Relay new flow key (K-vector)
    */

    /* All done with the tables, set flow key to invalid */
    phvwr       p.key2_ipv4_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE

    phvwr       FLOW_PARENT_FLIT, d.{txdma_ipv4_flow_hash_d.entry_valid...txdma_ipv4_flow_hash_d.__pad_to_512b}

    tblwr.l     d[511:504], k.key_ipv4_epoch
    tblwr.l     d[471:464], k.key_ipv4_proto
    tblwr.l     d[463:448], k.key_ipv4_dport
    tblwr.l     d[447:432], k.key_ipv4_sport
    tblwr.l     d[431:400], k.key_ipv4_ipv4_src
    tblwr.l     d[399:368], k.key_ipv4_ipv4_dst
    tblwr.l     d[367:360], k.key_ipv4_vpc_id

    phvwr       FLOW_LEAF_FLIT, d.{txdma_ipv4_flow_hash_d.entry_valid...txdma_ipv4_flow_hash_d.__pad_to_512b}

    add         r2, r0, k.key_ipv4_flow_ohash
    andi        r3, r2, 0x80000000
    sne         c2, r3, 0

    /* Hint entry specific */
    //phvwr.c2    p.txdma_to_arm_meta_rflow_parent_is_hint, TRUE
    andi.c2     r3, r2, ~(0x80000000)
    //phvwr.c2    p.txdma_to_arm_meta_rflow_parent_index, r3
    phvwrpair.c2 p.txdma_to_arm_meta_rflow_parent_is_hint, TRUE, \
                p.txdma_to_arm_meta_rflow_parent_index, r3

    phvwrpair   p.txdma_to_arm_meta_rflow_hash, r1, \
                p.txdma_to_arm_meta_rflow_ipaf, 0

    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint1
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 1

    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint2
    phvwr.c2.e  p.txdma_to_arm_meta_rflow_parent_hint_slot, 2
    nop
#endif
label_flow_miss:
    /* All done with the tables, set flow key to invalid */
    phvwr       p.key2_ipv4_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE
    /* 
        Setup assist informaton for rflow
        - Key information
        - Main table miss indication
    */
    tblwr.l     d[511:504], k.key_ipv4_epoch
    tblwr.l     d[471:464], k.key_ipv4_proto
    tblwr.l     d[463:448], k.key_ipv4_dport
    tblwr.l     d[447:432], k.key_ipv4_sport
    tblwr.l     d[431:400], k.key_ipv4_ipv4_src
    tblwr.l     d[399:368], k.key_ipv4_ipv4_dst
    tblwr.l     d[367:360], k.key_ipv4_vpc_id

    phvwr       FLOW_PARENT_FLIT, d.{txdma_ipv4_flow_hash_d.entry_valid...txdma_ipv4_flow_hash_d.__pad_to_512b}
    phvwrpair.e p.txdma_to_arm_meta_rflow_hash, r1, \
                p.txdma_to_arm_meta_rflow_ipaf, 0
    nop

label_flow_hit:
    /* TODO: Possibly due to epoch check failure? */
    nop.e
    nop

label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr.e     p.key_ipv4_flow_ohash, r2
    nop

label_key_invalid:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipv4_flow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
