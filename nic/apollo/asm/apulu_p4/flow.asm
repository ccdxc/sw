#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_flow_k.h"

struct flow_k_ k;
struct flow_d  d;
struct phv_ p;

#define FLOW_HASH_MSB 31:21

%%

flow_hash:
    bbne            d.flow_hash_d.entry_valid, TRUE, label_flow_miss
    phvwr           p.p4i_i2e_entropy_hash, r1
    bcf             [c1], label_flow_hit
    // Check hash1 and hint1
    seq             c1, r1[FLOW_HASH_MSB], d.flow_hash_d.hash1
    sne             c2, d.flow_hash_d.hint1, r0
    bcf             [c1&c2], label_flow_hash_hit
    add             r2, r0, d.flow_hash_d.hint1
    // Check hash2 and hint2
    seq             c1, r1[FLOW_HASH_MSB], d.flow_hash_d.hash2
    sne             c2, d.flow_hash_d.hint2, r0
    bcf             [c1&c2], label_flow_hash_hit
    add             r2, r0, d.flow_hash_d.hint2
    // Check hash3 and hint3
    seq             c1, r1[FLOW_HASH_MSB], d.flow_hash_d.hash3
    sne             c2, d.flow_hash_d.hint3, r0
    bcf             [c1&c2], label_flow_hash_hit
    add             r2, r0, d.flow_hash_d.hint3
    // Check hash4 and hint4
    seq             c1, r1[FLOW_HASH_MSB], d.flow_hash_d.hash4
    sne             c2, d.flow_hash_d.hint4, r0
    bcf             [c1&c2], label_flow_hash_hit
    add             r2, r0, d.flow_hash_d.hint4
    // Check for more hashes
    seq             c1, d.flow_hash_d.more_hashes, TRUE
    sne             c2, d.flow_hash_d.more_hints, r0
    bcf             [c1&c2], label_flow_hash_hit
    add             r2, r0, d.flow_hash_d.more_hints
    sne             c7, r0, r0

label_flow_miss:
    phvwrpair.c7    p.p4i_to_arm_flow_hit, TRUE, \
                        p.p4i_to_arm_flow_role, d.flow_hash_d.flow_role
    phvwr.c7        p.p4i_to_arm_session_id, d.{flow_hash_d.session_index}.wx
    phvwr.c7        p.p4i_to_arm_defunct_flow, k.ingress_recirc_defunct_flow
    phvwr.e         p.control_metadata_flow_miss, TRUE
    phvwr.f         p.control_metadata_flow_done, TRUE

label_flow_hit:
    phvwr           p.control_metadata_flow_epoch, d.flow_hash_d.epoch
    seq             c7, k.ingress_recirc_defunct_flow, TRUE
    seq.!c7         c7, d.flow_hash_d.force_flow_miss, TRUE
    bcf             [c7], label_flow_miss
    phvwr           p.control_metadata_flow_done, TRUE
    smneb           c1, k.tcp_flags, (TCP_FLAG_FIN|TCP_FLAG_RST), 0
    bcf             [c1], label_force_flow_miss
    seq             c1, d.flow_hash_d.nexthop_valid, TRUE
    bcf             [!c1], label_flow_hit_nexthop_done
    phvwr.c1        p.p4i_i2e_nexthop_type, d.flow_hash_d.nexthop_type
    seq             c1, d.flow_hash_d.nexthop_type, NEXTHOP_TYPE_VPC
    phvwr.c1        p.p4i_i2e_mapping_lkp_id, d.flow_hash_d.nexthop_id
    phvwr.!c1       p.p4i_i2e_mapping_bypass, TRUE
    phvwr.!c1       p.p4i_i2e_nexthop_id, d.flow_hash_d.nexthop_id
label_flow_hit_nexthop_done:
    phvwr.e         p.p4i_i2e_session_id, d.flow_hash_d.session_index
    phvwr.f         p.p4i_i2e_flow_role, d.flow_hash_d.flow_role

label_force_flow_miss:
    phvwrpair       p.p4i_to_arm_flow_hit, TRUE, \
                        p.p4i_to_arm_flow_role, d.flow_hash_d.flow_role
    phvwr           p.control_metadata_flow_miss, TRUE
    phvwr           p.control_metadata_force_flow_miss, TRUE
    phvwr           p.p4i_to_arm_session_id, d.{flow_hash_d.session_index}.wx
    nop.!c1.e
    seq             c1, d.flow_hash_d.nexthop_type, NEXTHOP_TYPE_VPC
    phvwr.c1        p.p4i_i2e_mapping_lkp_id, d.flow_hash_d.nexthop_id
    phvwr.!c1       p.p4i_i2e_mapping_bypass, TRUE
    phvwr.e         p.p4i_to_arm_nexthop_type, d.flow_hash_d.nexthop_type
    phvwr.!c1       p.p4i_to_arm_nexthop_id, d.{flow_hash_d.nexthop_id}.hx

label_flow_hash_hit:
    // Set bit 31 for overflow hash lookup to work
    ori             r2, r2, 0x80000000
    phvwr.e         p.ingress_recirc_flow_ohash, r2
    phvwr.f         p.control_metadata_flow_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
