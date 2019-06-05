#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_nexthop_k.h"

struct nexthop_k_ k;
struct nexthop_d  d;
struct phv_ p;

%%

nexthop_info:
    phvwr           p.txdma_to_p4e_valid, FALSE
    phvwr           p.predicate_header_valid, FALSE
    phvwr           p.p4e_i2e_valid, FALSE
    phvwr           p.capri_intrinsic_tm_oport, d.nexthop_info_d.port
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_DMAC_BITS], \
                        TX_REWRITE_DMAC_FROM_NEXTHOP
    phvwr.c1        p.ethernet_1_dstAddr, d.nexthop_info_d.dmaci
    seq             c1, k.rewrite_metadata_flags[TX_REWRITE_ENCAP_BITS], \
                        TX_REWRITE_ENCAP_VXLAN
    nop.!c1.e
    add             r1, r0, k.capri_p4_intrinsic_packet_len
    seq             c1, k.ctag_1_valid, TRUE
    sub.c1          r1, r1, 4
    phvwr.c1        p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.ethernet_0_dstAddr, d.nexthop_info_d.dmaco
    or              r7, d.nexthop_info_d.vni, 0x8, 48
    or              r7, r0, r7, 8
    phvwr           p.{vxlan_0_flags,vxlan_0_reserved,vxlan_0_vni, \
                        vxlan_0_reserved2}, r7
    bbeq            d.nexthop_info_d.ip_type, IPTYPE_IPV6, ipv6_vxlan_encap
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
    or              r7, ETHERTYPE_IPV4, k.rewrite_metadata_pa_mac, 16
    phvwr           p.{ethernet_0_srcAddr,ethernet_0_etherType}, r7
    add             r1, r1, 36
    phvwr           p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwr           p.ipv4_0_srcAddr, k.rewrite_metadata_encap_src_ip
    phvwr           p.ipv4_0_dstAddr, d.nexthop_info_d.dipo
    phvwr           p.{ipv4_0_ttl,ipv4_0_protocol}, (64 << 8) | IP_PROTO_UDP
    phvwr           p.ipv4_0_totalLen, r1
    sub             r1, r1, 20
    or              r7, k.p4e_i2e_entropy_hash, 0xC000
    or              r7, UDP_PORT_VXLAN, r7, 16
    phvwr.e         p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr.f         p.udp_0_len, r1
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
    or              r7, ETHERTYPE_IPV6, k.rewrite_metadata_pa_mac, 16
    phvwr           p.{ethernet_0_srcAddr,ethernet_0_etherType}, r7
    add             r1, r1, 16
    phvwr           p.ipv6_0_version, 0x6
    phvwr           p.ipv6_0_srcAddr, k.rewrite_metadata_encap_src_ip
    phvwr           p.ipv6_0_dstAddr, d.nexthop_info_d.dipo
    phvwr           p.{ipv6_0_nextHdr,ipv6_0_hopLimit}, (IP_PROTO_UDP << 8) | 64
    phvwr           p.ipv6_0_payloadLen, r1
    or              r7, k.p4e_i2e_entropy_hash, 0xC000
    or              r7, UDP_PORT_VXLAN, r7, 16
    phvwr.e         p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr.f         p.udp_0_len, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nexthop_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
