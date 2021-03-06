#include "ingress.h"
#include "INGRESS_p.h"
#include "athena.h"
#include "INGRESS_dnat_k.h"


struct dnat_k_      k;
struct dnat_d       d;
struct phv_         p;

#define DNAT_HASH_MSB   31:14

#define CHECK_HASH(_hash, _hint)                            \
    seq         c1, r1[DNAT_HASH_MSB], d.dnat_d._hash;      \
    sne         c2, d.dnat_d._hint, r0;                     \
    bcf         [c1&c2], label_flow_hash_hit;               \
    add         r2, r0, d.dnat_d._hint;
    


%%

dnat:
    bbne        d.dnat_d.entry_valid, TRUE, label_flow_miss
    nop
    bcf         [c1], label_flow_hit


    // Check hash1 and hint1
    CHECK_HASH(hash1, hint1);

    // Check hash2 and hint2
    CHECK_HASH(hash2, hint2);

    // Check hash3 and hint3
    CHECK_HASH(hash3, hint3);

    // Check hash4 and hint4
    CHECK_HASH(hash4, hint4);

    // Check hash5 and hint5
    CHECK_HASH(hash5, hint5);

    // Check for more hashes
    seq         c1, d.dnat_d.more_hashes, TRUE
    sne         c2, d.dnat_d.more_hints, r0
    bcf         [c1&c2], label_flow_hash_hit
    add         r2, r0, d.dnat_d.more_hints

label_flow_miss:
    phvwr.e     p.ingress_recirc_header_dnat_done, TRUE
    nop


label_flow_hit:
    phvwr       p.p4i_to_p4e_header_dnat_epoch, d.dnat_d.epoch
    phvwr.e     p.ingress_recirc_header_dnat_done, TRUE
    phvwr       p.key_metadata_src, d.dnat_d.addr


label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr.e     p.ingress_recirc_header_dnat_ohash, r2
    phvwr       p.control_metadata_dnat_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
dnat_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr           p.capri_p4_intrinsic_valid, TRUE
