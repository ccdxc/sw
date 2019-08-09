#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_l2_mapping_k k;
struct local_l2_mapping_d d;
struct phv_ p;

%%

local_l2_mapping_info:
    bbne        d.local_l2_mapping_info_d.entry_valid, TRUE, \
                    local_l2_mapping_miss

    // Set bit 31 for overflow hash lookup to work
    ori         r2, r0, 0x80000000
    bcf         [c1], local_l2_mapping_hit

    // Check hash1 and hint1
    seq         c1, r1[31:18], d.local_l2_mapping_info_d.hash1
    sne         c2, d.local_l2_mapping_info_d.hint1, r0
    bcf         [c1&c2], local_l2_mapping_hash_hit
    add         r2, r2, d.local_l2_mapping_info_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[31:18], d.local_l2_mapping_info_d.hash2
    sne         c2, d.local_l2_mapping_info_d.hint2, r0
    bcf         [c1&c2], local_l2_mapping_hash_hit
    add         r2, r2, d.local_l2_mapping_info_d.hint2
    // Check hash3 and hint3
    seq         c1, r1[31:18], d.local_l2_mapping_info_d.hash3
    sne         c2, d.local_l2_mapping_info_d.hint3, r0
    bcf         [c1&c2], local_l2_mapping_hash_hit
    add         r2, r2, d.local_l2_mapping_info_d.hint3
    // Check hash4 and hint4
    seq         c1, r1[31:18], d.local_l2_mapping_info_d.hash4
    sne         c2, d.local_l2_mapping_info_d.hint4, r0
    bcf         [c1&c2], local_l2_mapping_hash_hit
    add         r2, r2, d.local_l2_mapping_info_d.hint4
    // Check for more hashes
    seq         c1, d.local_l2_mapping_info_d.more_hashes, 1
    sne         c2, d.local_l2_mapping_info_d.more_hints, r0
    bcf         [c1&c2], local_l2_mapping_hash_hit
    add         r2, r2, d.local_l2_mapping_info_d.more_hints
local_l2_mapping_miss:
    seq         c1, k.vnic_metadata_skip_src_dst_check, FALSE
    phvwr.c1    p.control_metadata_p4i_drop_reason[P4I_DROP_SRC_DST_CHECK_FAIL], 1
    phvwr.e     p.service_header_local_mapping_done, TRUE
    phvwr.c1    p.capri_intrinsic_drop, TRUE

local_l2_mapping_hit:
    phvwr       p.service_header_local_mapping_done, TRUE
    sne         c1, d.local_l2_mapping_info_d.local_vnic_tag, r0
    phvwr.c1    p.vnic_metadata_local_vnic_tag, \
                    d.local_l2_mapping_info_d.local_vnic_tag
    seq.e       c1, d.local_l2_mapping_info_d.vpc_id_valid, TRUE
    phvwr.c1    p.vnic_metadata_vpc_id, d.local_l2_mapping_info_d.vpc_id

local_l2_mapping_hash_hit:
    phvwr.e     p.service_header_local_mapping_ohash, r2
    phvwr       p.control_metadata_local_mapping_ohash_lkp, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_l2_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
