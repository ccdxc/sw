#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct flow_k   k;
struct flow_d   d;
struct phv_     p;

%%

flow_hash:
    bbne        d.flow_hash_d.entry_valid, TRUE, label_flow_miss
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r0, 0x80000000
    bcf         [c1], label_flow_hit
    // Check hash1 and hint1
    seq         c1, r1[31:24], d.flow_hash_d.hash1
    sne         c2, d.flow_hash_d.hint1, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[31:24], d.flow_hash_d.hash2
    sne         c2, d.flow_hash_d.hint2, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.hint2
    // Check hash3 and hint3
    seq         c1, r1[31:24], d.flow_hash_d.hash3
    sne         c2, d.flow_hash_d.hint3, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.hint3
    // Check hash4 and hint4
    seq         c1, r1[31:24], d.flow_hash_d.hash4
    sne         c2, d.flow_hash_d.hint4, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.hint4
    // Check hash5 and hint5
    seq         c1, r1[31:24], d.flow_hash_d.hash5
    sne         c2, d.flow_hash_d.hint5, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.hint5
    // Check for more hashes
    seq         c1, d.flow_hash_d.more_hashes, TRUE
    sne         c2, d.flow_hash_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r2, d.flow_hash_d.more_hints
label_flow_miss:
    phvwr.e     p.control_metadata_flow_index, 0
    phvwr       p.service_header_flow_done, TRUE

label_flow_hit:
    phvwr.e     p.control_metadata_flow_index, d.flow_hash_d.flow_index
    phvwr       p.service_header_flow_done, TRUE

label_flow_hash_hit:
    phvwr.e     p.service_header_flow_ohash, r2
    phvwr       p.control_metadata_flow_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_error:
    nop.e
    nop
