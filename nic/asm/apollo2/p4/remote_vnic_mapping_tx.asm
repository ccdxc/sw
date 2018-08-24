#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct remote_vnic_mapping_tx_k k;
struct remote_vnic_mapping_tx_d d;
struct phv_                     p;

%%

remote_vnic_mapping_tx_info:
    // There are issues with this code. Needs to be revisited
    nop.e
    nop

    seq     c2, d.remote_vnic_mapping_tx_info_d.entry_valid, 1
    bcf     [c1&c2], remote_vnic_hit
    ori      r7, r0, 0x80000000  // Set bit 31 for overflow hash lookup to work

    phvwr   p.egress_service_header_valid, 1 // Set recirc. This can be cleared out in oflow action if there is match.
    //hint1
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash1
    or      r7, d.remote_vnic_mapping_tx_info_d.hint1_sbit10_ebit17, d.remote_vnic_mapping_tx_info_d.hint1_sbit0_ebit9, 10
    sne     c3, r7, r0 // check hint is != 0
    bcf     [c1&c2&c3], remote_vnic_hint1
    //hint2
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash2
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint2, r0
    bcf     [c1&c2&c3], remote_vnic_hint2
    //hint3
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash3
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint3, r0
    bcf     [c1&c2&c3], remote_vnic_hint3
    //hint4
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash4
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint4, r0
    bcf     [c1&c2&c3], remote_vnic_hint4
    //hint5
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash5
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint5, r0
    bcf     [c1&c2&c3], remote_vnic_hint5
    //hint6
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash6
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint6, r0
    bcf     [c1&c2&c3], remote_vnic_hint6
    //hint7
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash7
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint7, r0
    bcf     [c1&c2&c3], remote_vnic_hint7
    //hint8
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hash8
    sne     c3, d.remote_vnic_mapping_tx_info_d.hint8, r0
    bcf     [c1&c2&c3], remote_vnic_hint8
    //hintn
    seq     c1, r1[31:21], d.remote_vnic_mapping_tx_info_d.hashn
    sne     c3, d.remote_vnic_mapping_tx_info_d.hintn, r0
    bcf     [c1&c2&c3], remote_vnic_hintn

remote_vnic_hit:
    phvwr p.egress_service_header_valid, 0
    phvwr.e p.txdma_to_p4e_header_nexthop_index, d.remote_vnic_mapping_tx_info_d.nexthop_index
    phvwr p.control_metadata_remote_vnic_mapping_ohash_lkp, 1

remote_vnic_hint1:
    or      r2, d.remote_vnic_mapping_tx_info_d.hint1_sbit10_ebit17, d.remote_vnic_mapping_tx_info_d.hint1_sbit0_ebit9, 10
    or      r7, r7, r2
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint2:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint2
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint3:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint3
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint4:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint4
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint5:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint5
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint6:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint6
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint7:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint7
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hint8:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hint8
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7
remote_vnic_hintn:
    or      r7, r7, d.remote_vnic_mapping_tx_info_d.hintn
    phvwr.e p.egress_service_header_remote_vnic_mapping_ohash, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_tx_error:
    nop.e
    nop
