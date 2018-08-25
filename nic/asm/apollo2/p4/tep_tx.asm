#include "egress.h"
#include "EGRESS_p.h"
#include "apollo.h"

struct tep_tx_k     k;
struct tep_tx_d     d;
struct phv_         p;

%%

tep_tx:
    phvwr       p.{ctag_1_valid ... ethernet_1_valid}, 0
    phvwrpair   p.ipv4_0_valid, 1, \
                p.ethernet_0_valid, 1
    phvwrpair   p.mpls_0_0_valid, 1, \
                p.gre_0_valid, 1
    phvwrpair   p.ethernet_0_dstAddr, d.tep_tx_d.dmac, \
                p.ethernet_0_srcAddr, r5
    phvwr       p.ethernet_0_etherType, ETHERTYPE_IPV4
    .assert((offsetof(k, capri_p4_intrinsic_packet_len_sbit0_ebit5) - \
            offsetof(k, capri_p4_intrinsic_packet_len_sbit6_ebit13)) == 8)
    add         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}, 14
    seq         c1, k.ctag_1_valid, TRUE
    sub.c1      r1, r1, 4
    phvwr       p.{ipv4_0_version...ipv4_0_ihl}, 0x45
    phvwrpair   p.ipv4_0_srcAddr, k.rewrite_metadata_mytep_ip, \
                p.ipv4_0_dstAddr, d.tep_tx_d.dipo
    phvwr       p.{ipv4_0_ttl, ipv4_0_protocol}, (64 << 8) | IP_PROTO_GRE
    // Protocol and Label cannot be combined into a phvwrpair,
    // because it only takes a 9-bit constant.
    phvwr       p.gre_0_proto, ETHERTYPE_MPLS_UNICAST
    .assert(offsetof(k, p4e_apollo_i2e_src_slot_id_sbit0_ebit3) - \
            offsetof(k, p4e_apollo_i2e_src_slot_id_sbit4_ebit19) == 16)
    phvwr       p.mpls_0_0_label, k.rewrite_metadata_dst_slot_id
    seq         c1, k.rewrite_metadata_encap_type, VNIC_ENCAP
    add.c1      r1, r1, 4
    phvwr.!c1.e p.mpls_0_0_bos, 1
    phvwr       p.ipv4_0_totalLen, r1
    phvwr.c1.e  p.mpls_0_1_label, \
                k.{p4e_apollo_i2e_src_slot_id_sbit0_ebit3, \
                   p4e_apollo_i2e_src_slot_id_sbit4_ebit19}
    phvwr.c1    p.mpls_0_1_valid, 1

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tep_tx_error:
    nop.e
    nop
