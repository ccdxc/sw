#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_local_mapping_k.h"

struct local_mapping_k_ k;
struct local_mapping_d  d;
struct phv_             p;

#define HASH_MSB 31:17

%%

local_mapping_info:
    bbne        d.local_mapping_info_d.entry_valid, TRUE, local_mapping_miss

    // Set bit 31 for overflow hash lookup to work
    ori         r2, r0, 0x80000000
    bcf         [c1], local_mapping_hit

    // Check hash1 and hint1
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash1
    sne         c2, d.local_mapping_info_d.hint1, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash2
    sne         c2, d.local_mapping_info_d.hint2, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint2
    // Check hash3 and hint3
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash3
    sne         c2, d.local_mapping_info_d.hint3, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint3
    // Check hash4 and hint4
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash4
    sne         c2, d.local_mapping_info_d.hint4, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint4
    // Check hash5 and hint5
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash5
    sne         c2, d.local_mapping_info_d.hint5, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint5
    // Check hash6 and hint6
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash6
    sne         c2, d.local_mapping_info_d.hint6, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint6
    // Check hash7 and hint7
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash7
    sne         c2, d.local_mapping_info_d.hint7, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint7
    // Check hash8 and hint8
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash8
    sne         c2, d.local_mapping_info_d.hint8, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint8
    // Check hash9 and hint9
    seq         c1, r1[HASH_MSB], d.local_mapping_info_d.hash9
    sne         c2, d.local_mapping_info_d.hint9, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.hint9
    // Check for more hashes
    seq         c1, d.local_mapping_info_d.more_hashes, 1
    sne         c2, d.local_mapping_info_d.more_hints, r0
    bcf         [c1&c2], local_mapping_hash_hit
    add         r2, r2, d.local_mapping_info_d.more_hints
local_mapping_miss:
    seq         c1, k.arm_to_p4i_local_mapping_override, 1
    phvwr.e     p.control_metadata_local_mapping_done, TRUE
    phvwr.!c1.f p.control_metadata_local_mapping_miss, TRUE

local_mapping_hit:
    phvwr       p.p4i_to_arm_local_mapping_ip_type, \
                    d.local_mapping_info_d.ip_type
    sne         c1, d.local_mapping_info_d.vnic_id, r0
    phvwr.c1    p.vnic_metadata_vnic_id, d.local_mapping_info_d.vnic_id
    phvwr       p.control_metadata_binding_check_enabled, \
                    d.local_mapping_info_d.binding_check_enabled
    phvwr       p.vnic_metadata_binding_id, d.local_mapping_info_d.binding_id1
    seq         c1, k.control_metadata_rx_packet, FALSE
    seq.c1      c1, d.local_mapping_info_d.allow_tagged_pkts, FALSE
    seq.c1      c1, k.ctag_1_valid, TRUE
    phvwr       p.p4i_i2e_xlate_id, d.local_mapping_info_d.xlate_id
    nop.!c1.e
    phvwr       p.control_metadata_local_mapping_done, TRUE
local_mapping_drop_tagged_pkt:
    phvwr.e     p.control_metadata_p4i_drop_reason[P4I_DROP_TAGGED_PKT_FROM_VNIC], 1
    phvwr.f     p.capri_intrinsic_drop, 1

local_mapping_hash_hit:
    phvwr.e     p.ingress_recirc_local_mapping_ohash, r2
    phvwr.f     p.control_metadata_local_mapping_ohash_lkp, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
