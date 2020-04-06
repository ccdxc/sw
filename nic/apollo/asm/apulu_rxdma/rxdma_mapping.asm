#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_rxdma_mapping_k.h"

struct rxdma_mapping_k_ k;
struct rxdma_mapping_d  d;
struct phv_             p;

#define HASH_MSB 31:22

%%

rxdma_mapping_info:
    bbne            d.rxdma_mapping_info_d.entry_valid, 1, rxdma_mapping_miss
    // Set bit 31 for overflow hash lookup to work
    ori             r2, r0, 0x80000000
    bcf             [c1], rxdma_mapping_hit

    // Check hash1 and hint1
    seq             c1, r1[HASH_MSB], d.rxdma_mapping_info_d.hash1
    sne             c2, d.rxdma_mapping_info_d.hint1, r0
    bcf             [c1&c2], rxdma_mapping_hash_hit
    add             r2, r2, d.rxdma_mapping_info_d.hint1
    // Check hash2 and hint2
    seq             c1, r1[HASH_MSB], d.rxdma_mapping_info_d.hash2
    sne             c2, d.rxdma_mapping_info_d.hint2, r0
    bcf             [c1&c2], rxdma_mapping_hash_hit
    add             r2, r2, d.rxdma_mapping_info_d.hint2
    // Check for more hashes
    seq             c1, d.rxdma_mapping_info_d.more_hashes, TRUE
    sne             c2, d.rxdma_mapping_info_d.more_hints, r0
    bcf             [c1&c2], rxdma_mapping_hash_hit
    add             r2, r2, d.rxdma_mapping_info_d.more_hints
rxdma_mapping_miss:
    nop.e
    phvwr.f         p.p4_to_rxdma_mapping_done, TRUE

rxdma_mapping_hit:
    phvwr.e         p.lpm_metadata_mapping_tag_idx, \
                        d.rxdma_mapping_info_d.tag_idx
    phvwr.f         p.p4_to_rxdma_mapping_done, TRUE

rxdma_mapping_hash_hit:
    phvwr.e         p.lpm_metadata_mapping_ohash, r2
    phvwr.f         p.p4_to_rxdma_mapping_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rxdma_mapping_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
