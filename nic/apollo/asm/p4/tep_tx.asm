#include "egress.h"
#include "EGRESS_p.h"
#include "apollo.h"

struct tep_tx_k     k;
struct tep_tx_d     d;
struct phv_         p;

%%

udp_tep_tx:
    phvwr       p.{ctag_1_valid,ethernet_1_valid}, 0
    phvwr       p.{ipv4_0_valid,ctag_0_valid,ethernet_0_valid}, 0x5
    phvwrpair   p.mpls_dst_0_valid, 1, p.udp_0_valid, 1
    phvwrpair   p.ethernet_0_dstAddr, d.u.udp_tep_tx_d.dmac, \
                    p.ethernet_0_srcAddr, r5
    phvwr       p.ethernet_0_etherType, ETHERTYPE_IPV4
    add         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}, 18
    seq         c1, k.ctag_1_valid, TRUE
    sub.c1      r1, r1, 4
    phvwr       p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwrpair   p.ipv4_0_srcAddr, k.rewrite_metadata_mytep_ip, \
                    p.ipv4_0_dstAddr, d.u.udp_tep_tx_d.dipo
    phvwr       p.{ipv4_0_ttl,ipv4_0_protocol}, (64 << 8) | IP_PROTO_UDP
    phvwr       p.mpls_dst_0_label, k.rewrite_metadata_dst_slot_id
    phvwr       p.mpls_dst_0_bos, 1
    seq         c1, k.rewrite_metadata_encap_type, VNIC_ENCAP
    add.c1      r1, r1, 4
    phvwr.c1    p.mpls_src_0_label, \
                k.{p4e_apollo_i2e_src_slot_id_sbit0_ebit3, \
                   p4e_apollo_i2e_src_slot_id_sbit4_ebit19}
    phvwr.c1    p.mpls_src_0_valid, 1
    phvwr       p.ipv4_0_totalLen, r1
    sub         r1, r1, 20
    add         r7, UDP_PORT_MPLS, k.p4e_apollo_i2e_entropy_hash, 16
    phvwr.e     p.{udp_0_srcPort,udp_0_dstPort}, r7
    phvwr.f     p.udp_0_len, r1

.align
gre_tep_tx:
    phvwr       p.{ctag_1_valid,ethernet_1_valid}, 0
    phvwr       p.{ipv4_0_valid,ctag_0_valid,ethernet_0_valid}, 0x5
    phvwrpair   p.mpls_dst_0_valid, 1, p.gre_0_valid, 1
    phvwrpair   p.ethernet_0_dstAddr, d.u.gre_tep_tx_d.dmac, \
                    p.ethernet_0_srcAddr, r5
    phvwr       p.ethernet_0_etherType, ETHERTYPE_IPV4
    add         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}, 14
    seq         c1, k.ctag_1_valid, TRUE
    sub.c1      r1, r1, 4
    phvwr       p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwrpair   p.ipv4_0_srcAddr, k.rewrite_metadata_mytep_ip, \
                    p.ipv4_0_dstAddr, d.u.gre_tep_tx_d.dipo
    phvwr       p.{ipv4_0_ttl,ipv4_0_protocol}, (64 << 8) | IP_PROTO_GRE
    phvwr       p.mpls_dst_0_label, k.rewrite_metadata_dst_slot_id
    phvwr       p.mpls_dst_0_bos, 1
    seq         c1, k.rewrite_metadata_encap_type, VNIC_ENCAP
    add.c1      r1, r1, 4
    phvwr.c1    p.mpls_src_0_label, \
                k.{p4e_apollo_i2e_src_slot_id_sbit0_ebit3, \
                   p4e_apollo_i2e_src_slot_id_sbit4_ebit19}
    phvwr.c1    p.mpls_src_0_valid, 1
    phvwr.e     p.ipv4_0_totalLen, r1
    phvwr.f     p.gre_0_proto, ETHERTYPE_MPLS_UNICAST

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
