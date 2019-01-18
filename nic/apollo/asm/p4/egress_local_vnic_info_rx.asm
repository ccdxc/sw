#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct egress_local_vnic_info_rx_k k;
struct egress_local_vnic_info_rx_d d;
struct phv_ p;

%%

egress_local_vnic_info_rx:
    .assert(offsetof(p, vxlan_1_valid) - offsetof(p, ethernet_1_valid) == 8)
    phvwr       p.{vxlan_1_valid...ethernet_1_valid}, 0

    phvwr       p.ethernet_0_dstAddr, d.egress_local_vnic_info_rx_d.overlay_mac
    seq         c1, d.egress_local_vnic_info_rx_d.subnet_id, \
                    k.p4e_apollo_i2e_rvpath_subnet_id
    sne.c1      c1, k.p4e_apollo_i2e_rvpath_overlay_mac, 0
    phvwr.c1    p.ethernet_0_srcAddr, k.p4e_apollo_i2e_rvpath_overlay_mac
    phvwr.!c1   p.ethernet_0_srcAddr, d.egress_local_vnic_info_rx_d.vr_mac

    seq         c1, d.egress_local_vnic_info_rx_d.overlay_vlan_id, r0
    bcf         [c1], egress_local_vnic_native

egress_local_vnic_vlan_tag:
    .assert(offsetof(p, ctag_0_valid) - offsetof(p, ethernet_0_valid) == 1)
    phvwr.!c1   p.{ctag_0_valid, ethernet_0_valid}, 0x3
    phvwr       p.ethernet_0_etherType, ETHERTYPE_CTAG
    phvwr       p.ctag_0_vid, d.egress_local_vnic_info_rx_d.overlay_vlan_id
    seq         c1, k.ipv4_2_valid, 1
    phvwr.c1    p.ctag_0_etherType, ETHERTYPE_IPV4
    seq.e       c1, k.ipv6_2_valid, 1
    phvwr.c1    p.ctag_0_etherType, ETHERTYPE_IPV6

egress_local_vnic_native:
    phvwr       p.ethernet_0_valid, 0x1
    seq         c1, k.ipv4_2_valid, 1
    phvwr.c1    p.ethernet_0_etherType, ETHERTYPE_IPV4
    seq.e       c1, k.ipv6_2_valid, 1
    phvwr.c1    p.ethernet_0_etherType, ETHERTYPE_IPV6

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_local_vnic_info_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
