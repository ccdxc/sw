#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct remote_vnic_mapping_tx_k k;
struct remote_vnic_mapping_tx_d d;
struct phv_                     p;

%%

remote_vnic_mapping_tx_info:
    bbne        d.remote_vnic_mapping_tx_info_d.entry_valid, TRUE, \
                    remote_vnic_mapping_tx_miss
    // Set bit 31 for overflow hash lookup
    ori         r7, r0, 0x80000000
    bcf         [c1], remote_vnic_mapping_tx_hit

    //hint1
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash1
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint1, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint1
    //hint2
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash2
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint2, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint2
    //hint3
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash3
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint3, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint3
    //hint4
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash4
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint4, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint4
    //hint5
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash5
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint5, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint5
    //hint6
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash6
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint6, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint6
    //hint7
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash7
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint7, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint7
    //hint8
    seq         c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash8
    sne         c2, d.remote_vnic_mapping_tx_info_d.hint8, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.hint8
    //hintn
    seq         c1, d.remote_vnic_mapping_tx_info_d.more_hashes, TRUE
    sne         c2, d.remote_vnic_mapping_tx_info_d.more_hints, r0
    bcf         [c1&c2], remote_vnic_mapping_tx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_tx_info_d.more_hints
remote_vnic_mapping_tx_miss:
    phvwr.e     p.egress_service_header_remote_vnic_mapping_tx_done, TRUE
    nop

remote_vnic_mapping_tx_hit:
    phvwr       p.rewrite_metadata_dst_slot_id_valid, \
                    d.remote_vnic_mapping_tx_info_d.dst_slot_id_valid
    phvwr       p.rewrite_metadata_dst_slot_id, \
                    d.remote_vnic_mapping_tx_info_d.dst_slot_id
    phvwr.e     p.txdma_to_p4e_header_nexthop_index, \
                    d.remote_vnic_mapping_tx_info_d.nexthop_index
    phvwr       p.egress_service_header_remote_vnic_mapping_tx_done, TRUE

remote_vnic_mapping_tx_hash_hit:
    phvwr.e     p.egress_service_header_remote_vnic_mapping_tx_ohash, r7
    phvwr       p.control_metadata_remote_vnic_mapping_tx_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
