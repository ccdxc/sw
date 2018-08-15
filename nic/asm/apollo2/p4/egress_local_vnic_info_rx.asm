#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct egress_local_vnic_info_rx_k k;
struct egress_local_vnic_info_rx_d d;
struct phv_                     p;

%%

egress_local_vnic_info_rx:
    .assert(offsetof(p, ipv4_1_valid) - offsetof(p, ethernet_1_valid) == 3)
    phvwr       p.{ipv6_1_valid...ethernet_1_valid}, 0
    phvwr       p.gre_1_valid, FALSE
    phvwr       p.{mpls_1_valid...mpls_0_valid}, 0


    .assert(offsetof(p, ctag_0_valid) - offsetof(p, ethernet_0_valid) == 1)
    phvwr       p.{ctag_0_valid, ethernet_0_valid}, 0x3

    phvwr       p.ethernet_0_dstAddr, d.egress_local_vnic_info_rx_d.overlay_mac
    crestore    [c7-c4], 0, 0xf
    seq         c4, d.egress_local_vnic_info_rx_d.subnet_id, k.apollo_i2e_metadata_rvpath_subnet_id
    seq.c4      c5, k.apollo_i2e_metadata_rvpath_overlay_mac, 0
    phvwr.!c5   p.ethernet_0_srcAddr, k.apollo_i2e_metadata_rvpath_overlay_mac
    phvwr.c5    p.ethernet_0_srcAddr, d.egress_local_vnic_info_rx_d.vr_mac

    phvwr       p.ethernet_0_etherType, ETHERTYPE_CTAG
    phvwr       p.ctag_0_vid, d.egress_local_vnic_info_rx_d.overlay_vlan_id

    seq         c5, k.ipv4_2_valid, 1
    phvwr.c5    p.ctag_0_etherType, ETHERTYPE_IPV4
    seq.!c5     c6, k.ipv6_2_valid, 1
    phvwr.c6    p.ctag_0_etherType, ETHERTYPE_IPV6
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_local_vnic_info_rx_error:
    nop.e
    nop
