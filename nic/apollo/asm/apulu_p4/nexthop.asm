#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_k.h"

struct nexthop_k_   k;
struct nexthop_d    d;
struct phv_         p;

%%

nexthop_info:
    seq             c1, k.p4e_i2e_nexthop_id, r0
    bcf             [c1], nexthop_invalid
    // r1 : packet length
    add             r1, r0, k.capri_p4_intrinsic_packet_len
    seq             c1, d.nexthop_info_d.port, TM_PORT_DMA
    bcf             [!c1], nexthop_rewrite
    phvwr           p.capri_intrinsic_tm_oport, d.nexthop_info_d.port
    phvwr           p.capri_intrinsic_lif, d.nexthop_info_d.lif
    phvwr           p.capri_rxdma_intrinsic_qtype, d.nexthop_info_d.qtype
    phvwr           p.capri_rxdma_intrinsic_qid, d.nexthop_info_d.qid

nexthop_rewrite:
    bbeq            k.control_metadata_rx_packet, TRUE, nexthop_rx_rewrite
nexthop_tx_rewrite:
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_DMAC_BITS], \
                        TX_REWRITE_DMAC_FROM_MAPPING
    phvwr.c1        p.ethernet_1_dstAddr, k.rewrite_metadata_dmaci
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_DMAC_BITS], \
                        TX_REWRITE_DMAC_FROM_NEXTHOP
    phvwr.c1        p.ethernet_1_dstAddr, d.nexthop_info_d.dmaci
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_DMAC_BITS], \
                        TX_REWRITE_DMAC_FROM_TUNNEL
    phvwrpair.c1    p.ethernet_1_dstAddr[47:8], \
                        k.rewrite_metadata_tunnel_dmaci_s0_e39, \
                        p.ethernet_1_dstAddr[7:0], \
                        k.rewrite_metadata_tunnel_dmaci_s40_e47
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_SMAC_BITS], \
                        TX_REWRITE_SMAC_FROM_VRMAC
    phvwr.c1        p.ethernet_1_srcAddr, k.rewrite_metadata_vrmac
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_ENCAP_BITS], \
                        TX_REWRITE_ENCAP_VXLAN
    bcf             [c1], vxlan_encap
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_ENCAP_BITS], \
                        TX_REWRITE_ENCAP_VLAN
    bcf             [c1], vlan_encap
    nop.!c1.e

vxlan_encap:
    seq             c1, k.ctag_1_valid, TRUE
    sub.c1          r1, r1, 4
    phvwr.c1        p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.ethernet_0_dstAddr, d.nexthop_info_d.dmaco
    phvwr           p.ethernet_0_srcAddr, d.nexthop_info_d.smaco
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_VNI_BITS], \
                        TX_REWRITE_VNI_FROM_TUNNEL
    cmov            r7, c1, k.rewrite_metadata_tunnel_vni, \
                        k.rewrite_metadata_vni
    or              r7, r7, 0x8, 48
    or              r7, r0, r7, 8
    phvwr           p.{vxlan_0_flags,vxlan_0_reserved,vxlan_0_vni, \
                        vxlan_0_reserved2}, r7
    bbeq            k.rewrite_metadata_ip_type, IPTYPE_IPV6, ipv6_vxlan_encap
ipv4_vxlan_encap:
    /*
    phvwr           p.vxlan_0_valid, 1
    phvwr           p.udp_0_valid, 1
    phvwr           p.ipv4_0_valid, 1
    phvwr           p.ipv4_0_csum, 1
    phvwr           p.ethernet_0_valid, 1
    bitmap ==> 1 1010 0101
    */
    phvwr           p.ctag_1_valid, 0
    phvwr           p.{vxlan_0_valid, \
                        udp_0_valid, \
                        ipv6_0_valid, \
                        ipv4_0_valid, \
                        ipv4_0_udp_csum, \
                        ipv4_0_tcp_csum, \
                        ipv4_0_csum, \
                        ctag_0_valid, \
                        ethernet_0_valid}, 0x1A5
    phvwr           p.capri_deparser_len_ipv4_0_hdr_len, 20
    phvwr           p.ethernet_0_etherType, ETHERTYPE_IPV4
    add             r1, r1, 36
    phvwr           p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwr           p.ipv4_0_srcAddr, k.rewrite_metadata_device_ipv4_addr
    phvwr           p.{ipv4_0_ttl,ipv4_0_protocol}, (64 << 8) | IP_PROTO_UDP
    phvwr           p.ipv4_0_totalLen, r1
    sub             r1, r1, 20
    or              r7, k.p4e_i2e_entropy_hash, 0xC000
    or              r7, UDP_PORT_VXLAN, r7, 16
    phvwr           p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr           p.udp_0_len, r1
    add.e           r1, r1, (20+14)
    phvwr.f         p.capri_p4_intrinsic_packet_len, r1

ipv6_vxlan_encap:
    /*
    phvwr           p.vxlan_0_valid, 1
    phvwr           p.udp_0_valid, 1
    phvwr           p.ipv6_0_valid, 1
    phvwr           p.ethernet_0_valid, 1
    bitmap ==> 1 1100 0001
    */
    phvwr           p.ctag_1_valid, 0
    phvwr           p.{vxlan_0_valid, \
                        udp_0_valid, \
                        ipv6_0_valid, \
                        ipv4_0_valid, \
                        ipv4_0_udp_csum, \
                        ipv4_0_tcp_csum, \
                        ipv4_0_csum, \
                        ctag_0_valid, \
                        ethernet_0_valid}, 0x1C1
    phvwr           p.ethernet_0_etherType, ETHERTYPE_IPV6
    add             r1, r1, 16
    phvwr           p.ipv6_0_version, 0x6
    phvwr           p.ipv6_0_srcAddr, k.rewrite_metadata_device_ipv6_addr
    phvwr           p.{ipv6_0_nextHdr,ipv6_0_hopLimit}, (IP_PROTO_UDP << 8) | 64
    phvwr           p.ipv6_0_payloadLen, r1
    or              r7, k.p4e_i2e_entropy_hash, 0xC000
    or              r7, UDP_PORT_VXLAN, r7, 16
    phvwr           p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr           p.udp_0_len, r1
    add.e           r1, r1, (40+14)
    phvwr.f         p.capri_p4_intrinsic_packet_len, r1

nexthop_rx_rewrite:
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_DMAC_BITS], \
                        RX_REWRITE_DMAC_FROM_MAPPING
    phvwr.c1        p.ethernet_1_dstAddr, k.rewrite_metadata_dmaci
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_DMAC_BITS], \
                        RX_REWRITE_DMAC_FROM_NEXTHOP
    phvwr.c1        p.ethernet_1_dstAddr, d.nexthop_info_d.dmaci
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_SMAC_BITS], \
                        RX_REWRITE_SMAC_FROM_VRMAC
    phvwr.c1        p.ethernet_1_srcAddr, k.rewrite_metadata_vrmac
    seq             c1, k.rewrite_metadata_flags[RX_REWRITE_ENCAP_BITS], \
                        RX_REWRITE_ENCAP_VLAN
    bcf             [c1], vlan_encap
    nop.!c1.e
    nop

vlan_encap:
    seq             c1, k.ctag_1_valid, FALSE
    nop.!c1.e
    phvwr           p.{ctag_1_pcp,ctag_1_dei,ctag_1_vid}, d.nexthop_info_d.vlan
    phvwr           p.ctag_1_valid, TRUE
    phvwr           p.ctag_1_etherType, k.ethernet_1_etherType
    phvwr           p.{ctag_1_pcp,ctag_1_dei,ctag_1_vid}, d.nexthop_info_d.vlan
    phvwr           p.ethernet_1_etherType, ETHERTYPE_VLAN
    add.e           r1, r1, 4
    phvwr.f         p.capri_p4_intrinsic_packet_len, r1

nexthop_invalid:
    phvwr.e         p.control_metadata_p4e_drop_reason[P4E_DROP_NEXTHOP_INVALID], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
