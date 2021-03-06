#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_ipv4_flow_k.h"

struct ipv4_flow_k_ k;
struct ipv4_flow_d  d;
struct phv_ p;

#define FLOW_HASH_MSB 31:23

%%

.assert(offsetof(k, key_metadata_ipv4_dst) == (512 - 40 - 32))
.assert(offsetof(k, key_metadata_proto)    == (512 - 40 - (32 + 8)))
.assert(offsetof(k, key_metadata_ipv4_src) == (512 - 40 - (32 + 8 + 32)))
.assert(offsetof(k, key_metadata_dport)    == (512 - 40 - (32 + 8 + 32 + 16)))
.assert(offsetof(k, key_metadata_sport)    == (512 - 40 - (32 + 8 + 32 + 16 + 16)))
.assert(offsetof(k, vnic_metadata_vpc_id)  == (512 - 40 - (32 + 8 + 32 + 16 + 16 + 8)))

ipv4_flow_hash:
    bbne        d.ipv4_flow_hash_d.entry_valid, TRUE, label_flow_miss
    phvwr       p.p4i_i2e_entropy_hash, r1
    bcf         [c1], label_flow_hit
    // Check hash1 and hint1
    seq         c1, r1[FLOW_HASH_MSB], d.ipv4_flow_hash_d.hash1
    sne         c2, d.ipv4_flow_hash_d.hint1, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.ipv4_flow_hash_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[FLOW_HASH_MSB], d.ipv4_flow_hash_d.hash2
    sne         c2, d.ipv4_flow_hash_d.hint2, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.ipv4_flow_hash_d.hint2
    // Check for more hashes
    seq         c1, d.ipv4_flow_hash_d.more_hashes, TRUE
    sne         c2, d.ipv4_flow_hash_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.ipv4_flow_hash_d.more_hints

label_2nd_level_flow_miss:
    seq         c1, k.control_metadata_flow_ohash_lkp, 1
    phvwr.c1    p.p4_to_rxdma3_parent_is_hint, 1
    phvwr.c1    p.p4_to_rxdma3_parent_hint_index, \
                    k.ingress_recirc_flow_ohash[21:0]
    phvwr       p.p4_to_rxdma3_parent_valid, 1

    seq         c2, r0, d.ipv4_flow_hash_d.hint1
    phvwr.c2    p.p4_to_rxdma3_parent_hint_slot, 1
    b.c2        label_flow_miss

    seq         c2, r0, d.ipv4_flow_hash_d.hint2
    phvwr.c2    p.p4_to_rxdma3_parent_hint_slot, 2
    b.c2        label_flow_miss

    seq         c2, r0, d.ipv4_flow_hash_d.more_hints
    phvwr.c2    p.p4_to_rxdma3_parent_hint_slot, 5

label_flow_miss:
    phvwr       p.p4_to_rxdma3_flow_nrecircs, 1 // TODO : use correct value
    phvwr       p.p4_to_rxdma3_epoch, k.control_metadata_epoch
    phvwrpair   p.p4_to_rxdma3_flow_hash, r1, p.p4_to_rxdma3_ipaf, 0
    phvwr       p.p4_to_rxdma_tag_root, r5
    phvwr.e     p.control_metadata_pipe_id, PIPE_CPS
    phvwr.f     p.ingress_recirc_flow_done, TRUE

label_flow_hit:
    slt         c1, d.ipv4_flow_hash_d.epoch, k.control_metadata_epoch
    bcf         [c1], label_flow_miss
    phvwr.!c1   p.p4i_i2e_session_index, d.ipv4_flow_hash_d.session_index
    phvwr.e     p.control_metadata_pipe_id, PIPE_EGRESS
    phvwr.f     p.p4i_i2e_flow_role, d.ipv4_flow_hash_d.flow_role

label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr       p.ingress_recirc_flow_ohash, r2
    phvwr.e     p.control_metadata_pipe_id, PIPE_INGRESS
    phvwr.f     p.control_metadata_flow_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipv4_flow_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
