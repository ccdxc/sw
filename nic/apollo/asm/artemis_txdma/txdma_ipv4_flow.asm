#include "artemis_txdma.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_txdma_ipv4_flow_k.h"

struct txdma_ipv4_flow_k_ k;
struct txdma_ipv4_flow_d  d;
struct phv_ p;

#define FLOW_HASH_MSB 31:23
#define FLOW_HASH_LSB 22:0
#define FLOW_FLIT_TO_PHV_RANGE(flit) p[(((flit+1)*512)-1):(flit*512)]
#define FLOW_PARENT_FLIT    FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_PARENT_FLIT)
#define FLOW_LEAF_FLIT      FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_LEAF_FLIT)

// Support max of 6 flow lookups (so max 4 nrecircs)
#define MAX_FLOW_LOOKUPS    5

%%

ipv4_flow_hash:
    /* Without predicate bits, we need to validate if the key is indeed valid */
    sne         c2, k.key2_ipv4_flow_lkp_type, 4
    b.c2        label_key_invalid
    nop

    // Set hash only in the first lookup
    seq         c2, k.key2_ipv4_num_flow_lkps, 0
    phvwr.c2    p.txdma_to_arm_meta_rflow_hash, r1.wx

    bbne        d.txdma_ipv4_flow_hash_d.entry_valid, TRUE, label_flow_miss
    nop

    // manually compare keys as we are not using hw overflow assist
    seq         c1, k[471:408], d[471:408]
    seq.c1      c1, k[407:360], d[407:360]
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
    /* Entry valid but, no key match, no hint match, no more hints, nor linked
     * hints (Hint entry scenario)
     *      - Relay Parent entry (D-vector) 
     *      - Relay new flow key (K-vector)
     */

    phvwr       FLOW_PARENT_FLIT, d.{txdma_ipv4_flow_hash_d.entry_valid...txdma_ipv4_flow_hash_d.__pad_to_512b}

    seq         c2, k.key_ipv4_flow_ohash_lkp, 1

    /* Hint entry specific */
    add         r4, r0, k.key_ipv4_flow_ohash[FLOW_HASH_LSB].wx
    phvwrpair.c2 p.txdma_to_arm_meta_rflow_parent_is_hint, TRUE, \
                p.txdma_to_arm_meta_rflow_parent_index, r4

    slt         c1, k.key2_ipv4_num_flow_lkps, 2
    add.c1      r1, r0, r0
    sub.!c1     r1, k.key2_ipv4_num_flow_lkps, 1
    phvwr       p.txdma_to_arm_meta_rflow_nrecircs, r1

    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint1
    phvwr.c2    p.txdma_to_arm_meta_rflow_parent_hint_slot, 1

    seq         c2, r0, d.txdma_ipv4_flow_hash_d.hint2
    phvwr.c2    p.txdma_to_arm_meta_rflow_parent_hint_slot, 2
    nop

label_flow_miss:
    /* All done with the tables, set flow key to invalid */
    phvwr       p.key2_ipv4_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE

    /* 
     *  Setup assist informaton for rflow
     *  - Key information
     *  - Main table miss indication
     */

    // zero flit first
    phvwr       FLOW_LEAF_FLIT, 0

    // Fill rflow key in leaf entry (flit 5)
    //  v4 entries start at flit offset 256 (since they are half wide)
    //  check ipv4_flow_k for offset from bottom where the key starts
    add             r1, 0, (TXDMA_RFLOW_LEAF_FLIT * 512) + 256 + 96 + 8
    phvwrp          r1, 0, 8, k.key_ipv4_vpc_id
    add             r1, r1, 8
    phvwrp          r1, 0, 32, k.key_ipv4_ipv4_dst[31:0]
    add             r1, r1, 32
    phvwrp          r1, 0, 32, k.key_ipv4_ipv4_src[31:0]
    add             r1, r1, 32
    phvwrp          r1, 0, 16, k.key_ipv4_sport
    add             r1, r1, 16
    phvwrp          r1, 0, 16, k.key_ipv4_dport
    add             r1, r1, 16
    phvwrp          r1, 0, 8, k.key_ipv4_proto

    // Fill iflow d (has to match ipv4_flow_d
    add             r1, 0, (TXDMA_RFLOW_LEAF_FLIT * 512) + 512 - 8
    phvwrp.e        r1, 0, 8, k.key_ipv4_epoch
    nop

label_flow_hit:
    /* TODO: Possibly due to epoch check failure? */
    nop.e
    nop

label_flow_hash_hit:
    // increment number of flow lookups
    add         r1, k.key2_ipv4_num_flow_lkps, 1

    seq         c2, r1, MAX_FLOW_LOOKUPS
    b.c2        max_recircs_reached
    phvwr       p.key2_ipv4_num_flow_lkps, r1

    // indicate that this is an overflow lookup
    phvwr.e     p.key_ipv4_flow_ohash, r2
    phvwr       p.key_ipv4_flow_ohash_lkp, 1

label_key_invalid:
    nop.e
    nop

max_recircs_reached:
    // error, should not reach here
    phvwr       FLOW_FLIT_TO_PHV_RANGE(TXDMA_IFLOW_PARENT_FLIT), 0
    phvwr       FLOW_FLIT_TO_PHV_RANGE(TXDMA_IFLOW_LEAF_FLIT), 0
    phvwr       FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_PARENT_FLIT), 0
    phvwr       FLOW_FLIT_TO_PHV_RANGE(TXDMA_RFLOW_LEAF_FLIT), 0

    phvwr       p.key2_ipv4_flow_lkp_type, 0
    phvwr       p.{txdma_predicate_flow_enable...txdma_predicate_cps_path_en}, 0
    phvwr       p.capri_p4_intr_recirc, FALSE
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
