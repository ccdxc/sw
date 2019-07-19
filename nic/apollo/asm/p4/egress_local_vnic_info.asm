#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_local_vnic_info_k.h"

struct egress_local_vnic_info_k_ k;
struct egress_local_vnic_info_d d;
struct phv_ p;

%%

egress_local_vnic_info:
    seq         c1, k.control_metadata_direction, TX_FROM_HOST
    bcf         [c1], egress_local_vnic_info_tx
    nop
egress_local_vnic_info_rx:
    seq         c1, d.egress_local_vnic_info_d.mirror_en, TRUE
    phvwr.c1    p.control_metadata_mirror_session, \
                    d.egress_local_vnic_info_d.mirror_session
    seq         c1, d.egress_local_vnic_info_d.lif, r0
    phvwr.!c1   p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr.!c1   p.capri_intrinsic_lif, d.egress_local_vnic_info_d.lif
    phvwr.!c1   p.capri_rxdma_intrinsic_qtype, d.egress_local_vnic_info_d.qtype
    phvwr.!c1   p.capri_rxdma_intrinsic_qid, d.egress_local_vnic_info_d.qid
    phvwr.c1    p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_0
    .assert(offsetof(p, ethernet_2_valid) - offsetof(p, ethernet_1_valid) == 12)
    phvwr       p.{ethernet_2_valid...ethernet_1_valid}, 0
    seq         c1, k.ethernet_2_valid, 1
    phvwr.c1    p.ethernet_0_dstAddr, k.ethernet_2_dstAddr
    phvwr.c1    p.ethernet_0_etherType, k.ethernet_2_etherType
    bcf         [c1], egress_local_vnic_rx_l2payload
    phvwr.c1    p.ethernet_0_srcAddr, d.egress_local_vnic_info_d.vr_mac
    phvwr       p.ethernet_0_dstAddr, d.egress_local_vnic_info_d.overlay_mac
    seq         c1, d.egress_local_vnic_info_d.subnet_id, \
                    k.p4e_apollo_i2e_rvpath_subnet_id
    sne.c1      c1, k.p4e_apollo_i2e_rvpath_overlay_mac, 0
    phvwr.c1    p.ethernet_0_srcAddr, k.p4e_apollo_i2e_rvpath_overlay_mac
    phvwr.!c1   p.ethernet_0_srcAddr, d.egress_local_vnic_info_d.vr_mac

egress_local_vnic_rx_l2payload:
    seq         c1, k.vxlan_1_valid, 1
    sub.c1      r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_2  
    sub.!c1     r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l3_2
    add.!c1     r1, r1, 14
    seq         c1, d.egress_local_vnic_info_d.overlay_vlan_id, r0
    add.!c1     r1, r1, 4
    bcf         [c1], egress_local_vnic_rx_native
    phvwr       p.capri_p4_intrinsic_packet_len, r1

egress_local_vnic_rx_vlan_tag:
    .assert(offsetof(p, ctag_0_valid) - offsetof(p, ethernet_0_valid) == 1)
    phvwr.!c1   p.{ctag_0_valid, ethernet_0_valid}, 0x3
    phvwr       p.ethernet_0_etherType, ETHERTYPE_CTAG
    phvwr       p.ctag_0_vid, d.egress_local_vnic_info_d.overlay_vlan_id
    seq         c1, k.ipv4_2_valid, 1
    phvwr.c1    p.ctag_0_etherType, ETHERTYPE_IPV4
    seq.e       c1, k.ipv6_2_valid, 1
    phvwr.c1    p.ctag_0_etherType, ETHERTYPE_IPV6

egress_local_vnic_rx_native:
    phvwr       p.ethernet_0_valid, 1
    seq         c1, k.ipv4_2_valid, 1
    phvwr.c1    p.ethernet_0_etherType, ETHERTYPE_IPV4
    seq.e       c1, k.ipv6_2_valid, 1
    phvwr.c1    p.ethernet_0_etherType, ETHERTYPE_IPV6

egress_local_vnic_info_tx:
    phvwr.e     p.rewrite_metadata_src_slot_id, \
                    d.egress_local_vnic_info_d.src_slot_id
    phvwr       p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_local_vnic_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
