#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel2_k.h"

struct tunnel2_k_   k;
struct tunnel2_d    d;
struct phv_         p;

%%

/*
    r7 : packet length
    r6 : ether type
    c7 : vlan encap
    c6 : encap type

    vxlan_00_valid
    mpls_00_valid
    udp_00_valid
    ipv6_00_valid
    ipv4_00_valid
    ipv4_00_udp_csum
    ipv4_00_tcp_csum
    ipv4_00_csum
    ctag_00_valid
    ethernet_00_valid

    IPv4 MPLSoUDP
    phvwr           p.mpls_00_valid, 1
    phvwr           p.udp_00_valid, 1
    phvwr           p.ipv4_00_valid, 1
    phvwr           p.ipv4_00_csum, 1
    phvwr           p.ethernet_00_valid, 1
    bitmap ==> 01 1010 0101 (0x1A5)

    IPv6 MPLSoUDP
    phvwr           p.mpls_00_valid, 1
    phvwr           p.udp_00_valid, 1
    phvwr           p.ipv6_00_valid, 1
    phvwr           p.ethernet_00_valid, 1
    bitmap ==> 01 1100 0001 (0x1C1)

    IPv4 VxLAN
    phvwr           p.vxlan_00_valid, 1
    phvwr           p.udp_00_valid, 1
    phvwr           p.ipv4_00_valid, 1
    phvwr           p.ipv4_00_csum, 1
    phvwr           p.ethernet_00_valid, 1
    bitmap ==> 10 1010 0101 (0x2A5)

    IPv6 VxLAN
    phvwr           p.vxlan_00_valid, 1
    phvwr           p.udp_00_valid, 1
    phvwr           p.ipv6_00_valid, 1
    phvwr           p.ethernet_00_valid, 1
    bitmap ==> 10 1100 0001 (0x2C1)
*/

tunnel2_info:
    add             r7, r0, k.capri_p4_intrinsic_packet_len
    seq             c6, d.tunnel2_info_d.encap_type, TX_REWRITE_ENCAP_VXLAN
    bbeq            d.tunnel2_info_d.ip_type, IPTYPE_IPV6, tunnel2_ipv6_encap
    sne             c7, d.tunnel2_info_d.vlan, r0

tunnel2_ipv4_encap:
    // header valid bits
    phvwr.!c6       p.ethernet_0_valid, 0
    cmov            r1, c6, 0x2A5, 0x1A5
    or.c7           r1, r1, 0x20
    phvwr           p.{vxlan_00_valid, \
                        mpls_00_valid, \
                        udp_00_valid, \
                        ipv6_00_valid, \
                        ipv4_00_valid, \
                        ipv4_00_udp_csum, \
                        ipv4_00_tcp_csum, \
                        ipv4_00_csum, \
                        ctag_00_valid, \
                        ethernet_00_valid}, r1

    // ipv4 header
    phvwr           p.capri_deparser_len_ipv4_00_hdr_len, 20
    cmov            r1, c6, (20 + 8 + 8), (20 + 8 + 4 - 14)
    add             r7, r7, r1
    add             r1, k.rewrite_metadata_tunnel_tos, 0x45, 8
    phvwr           p.{ipv4_00_version,ipv4_00_ihl,ipv4_00_diffserv}, r1
    or              r1, d.tunnel2_info_d.dipo, \
                        k.rewrite_metadata_device_ipv4_addr, 32
    phvwr           p.{ipv4_00_srcAddr,ipv4_00_dstAddr}, r1
    phvwr           p.ipv4_00_ttl, 64
    phvwr           p.ipv4_00_protocol, IP_PROTO_UDP
    phvwr           p.ipv4_00_totalLen, r7
    add             r6, r0, ETHERTYPE_IPV4

    // udp header
    or              r2, k.p4e_i2e_entropy_hash, 0xC000
    or.!c6          r2, UDP_PORT_MPLS, r2, 16
    or.c6           r2, UDP_PORT_VXLAN, r2, 16
    phvwr           p.{udp_00_srcPort,udp_00_dstPort}, r2
    sub             r1, r7, 20
    phvwr           p.udp_00_len, r1

tunnel2_vlan_encap:
    // vlan header
    add             r7, r7, 14
    bcf             [!c7], tunnel2_encap
    phvwr.!c7       p.ethernet_00_etherType, r6
    phvwr           p.ethernet_00_etherType, ETHERTYPE_VLAN
    or              r1, r6, d.tunnel2_info_d.vlan, 16
    phvwr           p.{ctag_00_pcp,ctag_00_dei,ctag_00_vid,ctag_00_etherType}, r1
    add             r7, r7, 4

tunnel2_encap:
    // encap header
    bcf             [c6], tunnel2_vxlan_encap
    phvwr           p.capri_p4_intrinsic_packet_len, r7
tunnel2_mpls_encap:
    or.e            r1, 0x140, k.rewrite_metadata_tunnel2_vni, 12
    phvwr.f         p.{mpls_00_label,mpls_00_exp,mpls_00_bos,mpls_00_ttl}, r1

tunnel2_vxlan_encap:
    or              r1, k.rewrite_metadata_tunnel2_vni, 0x8, 48
    or.e            r1, r0, r1, 8
    phvwr.f         p.{vxlan_00_flags,vxlan_00_reserved,vxlan_00_vni, \
                        vxlan_00_reserved2}, r1

tunnel2_ipv6_encap:
    phvwr.!c6       p.ethernet_0_valid, 0
    cmov            r1, c6, 0x2C1, 0x1C1
    or.c7           r1, r1, 0x20
    phvwr           p.{vxlan_00_valid, \
                        mpls_00_valid, \
                        udp_00_valid, \
                        ipv6_00_valid, \
                        ipv4_00_valid, \
                        ipv4_00_udp_csum, \
                        ipv4_00_tcp_csum, \
                        ipv4_00_csum, \
                        ctag_00_valid, \
                        ethernet_00_valid}, r1

    // ipv6 header
    cmov            r1, c6, (8 + 8), (8 + 4 - 14)
    add             r7, r7, r1
    add             r1, k.rewrite_metadata_tunnel_tos, 0x6, 8
    phvwr           p.{ipv6_00_version,ipv6_00_trafficClass}, r1
    phvwr           p.ipv6_00_srcAddr, k.rewrite_metadata_device_ipv6_addr
    phvwr           p.ipv6_00_dstAddr, d.tunnel2_info_d.dipo
    phvwr           p.{ipv6_00_nextHdr,ipv6_00_hopLimit}, (IP_PROTO_UDP << 8)|64
    phvwr           p.ipv6_00_payloadLen, r7
    add             r6, r0, ETHERTYPE_IPV6

    // udp header
    or              r2, k.p4e_i2e_entropy_hash, 0xC000
    or.!c6          r2, UDP_PORT_MPLS, r2, 16
    or.c6           r2, UDP_PORT_VXLAN, r2, 16
    phvwr           p.{udp_00_srcPort,udp_00_dstPort}, r2
    phvwr           p.udp_00_len, r1

    b               tunnel2_vlan_encap
    add             r7, r7, 40

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel2_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
