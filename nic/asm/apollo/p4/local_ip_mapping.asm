#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_ip_mapping_k k;
struct local_ip_mapping_d d;
struct phv_ p;

%%

local_ip_mapping_info:
    bbne        d.local_ip_mapping_info_d.entry_valid, TRUE, \
                    local_ip_mapping_miss
    // Set bit 31 for overflow hash lookup to work
    ori         r2, r0, 0x80000000
    bcf         [c1], local_ip_mapping_hit
    // Check hash1 and hint1
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash1
    sne         c2, d.local_ip_mapping_info_d.hint1, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint1
    // Check hash2 and hint2
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash2
    sne         c2, d.local_ip_mapping_info_d.hint2, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint2
    // Check hash3 and hint3
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash3
    sne         c2, d.local_ip_mapping_info_d.hint3, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint3
    // Check hash4 and hint4
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash4
    sne         c2, d.local_ip_mapping_info_d.hint4, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint4
    // Check hash5 and hint5
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash5
    sne         c2, d.local_ip_mapping_info_d.hint5, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint5
    // Check hash6 and hint6
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash6
    sne         c2, d.local_ip_mapping_info_d.hint6, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint6
    // Check hash7 and hint7
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash7
    sne         c2, d.local_ip_mapping_info_d.hint7, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint7
    // Check hash8 and hint8
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash8
    sne         c2, d.local_ip_mapping_info_d.hint8, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint8
    // Check hash9 and hint9
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash9
    sne         c2, d.local_ip_mapping_info_d.hint9, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint9
    // Check hash10 and hint10
    seq         c1, r1[31:17], d.local_ip_mapping_info_d.hash10
    sne         c2, d.local_ip_mapping_info_d.hint10, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.hint10
    // Check for more hashes
    seq         c1, d.local_ip_mapping_info_d.more_hashes, 1
    sne         c2, d.local_ip_mapping_info_d.more_hints, r0
    bcf         [c1&c2], local_ip_mapping_hash_hit
    add         r2, r2, d.local_ip_mapping_info_d.more_hints
local_ip_mapping_miss:
    seq         c1, k.vnic_metadata_skip_src_dst_check, FALSE
    phvwr.c1    p.control_metadata_p4i_drop_reason[DROP_SRC_DST_CHECK_FAIL], 1
    nop.e
    phvwr.c1    p.capri_intrinsic_drop, TRUE

local_ip_mapping_hit:
    seq         c1, d.local_ip_mapping_info_d.vcn_id_valid, TRUE
    phvwr.c1    p.vnic_metadata_vcn_id, d.local_ip_mapping_info_d.vcn_id
    seq         c2, k.control_metadata_direction, RX_FROM_SWITCH
    seq.c2      c2, k.mpls_0_valid, TRUE
    seq.c2      c2, d.local_ip_mapping_info_d.ip_type, IP_TYPE_PUBLIC
    phvwr.c2    p.p4i_apollo_i2e_dnat_required, TRUE
    or          r1, d.local_ip_mapping_info_d.xlate_index_sbit14_ebit16, \
                    d.local_ip_mapping_info_d.xlate_index_sbit0_ebit13, 3
    phvwr.e     p.service_header_local_ip_mapping_done, TRUE
    phvwr       p.p4i_apollo_i2e_xlate_index, r1

local_ip_mapping_hash_hit:
    phvwr.e     p.service_header_local_ip_mapping_ohash, r2
    phvwr       p.control_metadata_local_ip_mapping_ohash_lkp, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_ip_mapping_error:
    nop.e
    nop
