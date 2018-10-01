#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct remote_vnic_mapping_rx_k k;
struct remote_vnic_mapping_rx_d d;
struct phv_ p;

%%

remote_vnic_mapping_rx_info:
    bbne        d.remote_vnic_mapping_rx_info_d.entry_valid, TRUE, \
                    remote_vnic_mapping_rx_miss
    // Set bit 31 for overflow hash lookup
    ori         r7, r0, 0x80000000
    bcf         [c1], remote_vnic_mapping_rx_hit

    //hint1
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash1
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint1, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint1
    //hint2
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash2
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint2, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint2
    //hint3
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash3
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint3, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint3
    //hint4
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash4
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint4, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint4
    //hint5
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash5
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint5, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint5
    //hint6
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash6
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint6, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint6
    //hint7
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash7
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint7, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint7
    //hint8
    seq         c1, r1[31:21], d.remote_vnic_mapping_rx_info_d.hash8
    sne         c2, d.remote_vnic_mapping_rx_info_d.hint8, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.hint8
    //hintn
    seq         c1, d.remote_vnic_mapping_rx_info_d.more_hashes, TRUE
    sne         c2, d.remote_vnic_mapping_rx_info_d.more_hints, r0
    bcf         [c1&c2], remote_vnic_mapping_rx_hash_hit
    add         r7, r7, d.remote_vnic_mapping_rx_info_d.more_hints
remote_vnic_mapping_rx_miss:
    phvwr.e     p.service_header_remote_vnic_mapping_rx_done, TRUE
    nop

remote_vnic_mapping_rx_hit:
    sne         c1, k.vnic_metadata_vcn_id, \
                    d.remote_vnic_mapping_rx_info_d.vcn_id
    or          r1, d.remote_vnic_mapping_rx_info_d.overlay_mac_sbit5_ebit47, \
                    d.remote_vnic_mapping_rx_info_d.overlay_mac_sbit0_ebit4, 43
    phvwr.c1.e  p.p4i_apollo_i2e_rvpath_subnet_id, \
                    d.remote_vnic_mapping_rx_info_d.subnet_id
    phvwr.c1    p.p4i_apollo_i2e_rvpath_overlay_mac, r1
    phvwr.!c1.e p.capri_intrinsic_drop, TRUE
    nop

remote_vnic_mapping_rx_hash_hit:
    phvwr.e     p.service_header_remote_vnic_mapping_rx_ohash, r7
    phvwr       p.control_metadata_remote_vnic_mapping_rx_ohash_lkp, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_rx_error:
    nop.e
    nop
