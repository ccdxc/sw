#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_flow_k.h"

struct flow_k_  k;
struct flow_d   d;
struct phv_     p;

#define FLOW_HASH_MSB 31:21

#define CHECK_FLOW_HASH(_hash, _hint)                       \
    seq         c1, r1[FLOW_HASH_MSB], d.flow_hash_d._hash; \
    sne         c2, d.flow_hash_d._hint, r0;                \
    bcf         [c1&c2], label_flow_hash_hit;               \
    add         r2, r0, d.flow_hash_d._hint;

%%

flow_hash:
    bbne        d.flow_hash_d.entry_valid, TRUE, label_flow_miss
    nop
    bcf         [c1], label_flow_hit
    // Check hash1 and hint1
    CHECK_FLOW_HASH(hash1, hint1);
    // Check hash2 and hint2
    CHECK_FLOW_HASH(hash2, hint2);
    // Check hash3 and hint3
    CHECK_FLOW_HASH(hash3, hint3);
    // Check hash3 and hint3
    CHECK_FLOW_HASH(hash3, hint3);
    // Check hash4 and hint4
    CHECK_FLOW_HASH(hash4, hint4);
    // Check hash5 and hint5
    CHECK_FLOW_HASH(hash5, hint5);
    // Check for more hashes
    seq         c1, d.flow_hash_d.more_hashes, TRUE
    sne         c2, d.flow_hash_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.flow_hash_d.more_hints
label_flow_miss:
    phvwr       p.control_metadata_flow_miss, TRUE
    phvwr.e     p.p4i_to_p4e_header_index, 0
    phvwr       p.ingress_recirc_header_flow_done, TRUE

label_flow_hit:
    phvwr       p.p4i_to_p4e_header_direction, k.control_metadata_direction
    phvwr       p.p4i_to_p4e_header_index, d.flow_hash_d.index
    phvwr.e     p.p4i_to_p4e_header_index_type, d.flow_hash_d.index_type
    phvwr       p.ingress_recirc_header_flow_done, TRUE

label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr.e     p.ingress_recirc_header_flow_ohash, r2
    phvwr       p.control_metadata_flow_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
