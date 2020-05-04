#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_mapping_k.h"

struct mapping_k_   k;
struct mapping_d    d;
struct phv_         p;

#define HASH_MSB 31:22

%%

mapping_info:
    seq             c7, k.p4e_i2e_nexthop_type, NEXTHOP_TYPE_VPC
    phvwr.c7        p.p4e_i2e_vpc_id, k.p4e_i2e_mapping_lkp_id

    bbeq            k.p4e_i2e_mapping_bypass, TRUE, mapping_done
    phvwr           p.rewrite_metadata_nexthop_type, k.p4e_i2e_nexthop_type

    seq             c7, k.p4e_to_arm_valid, TRUE
    seq.c7          c7, k.txdma_to_p4e_nexthop_type, NEXTHOP_TYPE_VPC
    phvwr.c7        p.p4e_to_arm_nexthop_id, k.{txdma_to_p4e_nexthop_id}.hx
    bcf             [c7], mapping_done
    phvwr.c7        p.p4e_to_arm_nexthop_type, k.txdma_to_p4e_nexthop_type

    bbne            d.mapping_info_d.entry_valid, TRUE, mapping_miss
    // Set bit 31 for overflow hash lookup to work
    ori             r2, r0, 0x80000000
    bcf             [c1], mapping_hit

    // Check hash1 and hint1
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash1
    sne             c2, d.mapping_info_d.hint1, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint1
    // Check hash2 and hint2
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash2
    sne             c2, d.mapping_info_d.hint2, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint2
    // Check hash3 and hint3
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash3
    sne             c2, d.mapping_info_d.hint3, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint3
    // Check hash4 and hint4
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash4
    sne             c2, d.mapping_info_d.hint4, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint4
    // Check hash5 and hint5
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash5
    sne             c2, d.mapping_info_d.hint5, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint5
    // Check hash6 and hint6
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash6
    sne             c2, d.mapping_info_d.hint6, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint6
    // Check hash7 and hint7
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash7
    sne             c2, d.mapping_info_d.hint7, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint7
    // Check hash8 and hint8
    seq             c1, r1[HASH_MSB], d.mapping_info_d.hash8
    sne             c2, d.mapping_info_d.hint8, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.hint8
    // Check for more hashes
    seq             c1, d.mapping_info_d.more_hashes, 1
    sne             c2, d.mapping_info_d.more_hints, r0
    bcf             [c1&c2], mapping_hash_hit
    add             r2, r2, d.mapping_info_d.more_hints
mapping_miss:
    seq             c7, k.p4e_to_arm_valid, TRUE
    phvwr.c7        p.p4e_to_arm_nexthop_id, k.{txdma_to_p4e_nexthop_id}.hx
    phvwr.c7        p.p4e_to_arm_nexthop_type, k.txdma_to_p4e_nexthop_type

mapping_done:
    phvwr.e         p.egress_recirc_mapping_done, TRUE
    phvwr.f         p.control_metadata_mapping_done, TRUE

mapping_hit:
    phvwr           p.vnic_metadata_egress_bd_id, d.mapping_info_d.egress_bd_id
    phvwr           p.vnic_metadata_rx_vnic_id, d.mapping_info_d.rx_vnic_id
    phvwr           p.rewrite_metadata_dmaci, d.mapping_info_d.dmaci
    phvwr           p.egress_recirc_mapping_done, TRUE
    bbeq            k.p4e_to_arm_valid, TRUE, mapping_hit_arm
    phvwr           p.control_metadata_mapping_done, TRUE
    seq             c7, d.mapping_info_d.nexthop_valid, TRUE
    sle.c7          c7, r5, k.p4e_i2e_priority
    phvwr.c7        p.rewrite_metadata_nexthop_type, \
                        d.mapping_info_d.nexthop_type
    nop.e
    phvwr.c7        p.p4e_i2e_nexthop_id, d.mapping_info_d.nexthop_id
mapping_hit_arm:
    phvwr           p.p4e_to_arm_nexthop_id, k.{txdma_to_p4e_nexthop_id}.hx
    phvwr           p.p4e_to_arm_nexthop_type, k.txdma_to_p4e_nexthop_type
    phvwr           p.control_metadata_is_local, d.mapping_info_d.is_local
    phvwr.e         p.p4e_to_arm_is_local, d.mapping_info_d.is_local
    phvwr.f         p.p4e_to_arm_mapping_hit, TRUE

mapping_hash_hit:
    phvwr.e         p.egress_recirc_mapping_ohash, r2
    phvwr.f         p.control_metadata_mapping_ohash_lkp, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
