#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_p4e_inter_pipe_k.h"
#include "CSUM_EGRESS.h"

struct p4e_inter_pipe_k_    k;
struct phv_                 p;

%%

p4e_inter_pipe:
    phvwr           p.capri_txdma_intrinsic_valid, 0
    sne             c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr.!c1       p.capri_intrinsic_tm_oq, k.capri_intrinsic_tm_iq
    seq             c1, k.egress_recirc_mapping_done, FALSE
    bcf             [c1], egress_recirc
    phvwrmi.!c1     p.{p4e_i2e_valid, \
                        p4e_to_arm_valid, \
                        txdma_to_p4e_valid, \
                        egress_recirc_valid}, 0x0, 0xB
    seq             c1, k.capri_intrinsic_tm_oport, TM_PORT_DMA
    nop.!c1.e

egress_to_rxdma:
    // r7 actual packet len
    // c7 ctag_1 valid
    add             r7, r0, k.capri_p4_intrinsic_packet_len
    seq             c7, k.ctag_1_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    seq             c1, k.p4e_to_arm_valid, TRUE
    bcf             [c1], egress_to_rxdma_arm
    add             r6, r0, r7
    seq             c1, k.control_metadata_rx_packet, TRUE
    phvwr.c1        p.capri_intrinsic_tm_span_session, k.p4e_i2e_mirror_session

    seq             c1, k.rewrite_metadata_vlan_strip_en, TRUE
    bcf             ![c1&c7], egress_to_rxdma_common
    nop
    phvwr           p.p4e_to_p4plus_classic_nic_vlan_valid, TRUE
    phvwr           p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.{p4e_to_p4plus_classic_nic_vlan_pcp, \
                        p4e_to_p4plus_classic_nic_vlan_dei, \
                        p4e_to_p4plus_classic_nic_vlan_vid}, \
                        k.{ctag_1_pcp,ctag_1_dei,ctag_1_vid}
    phvwr           p.ctag_1_valid, FALSE
    sub             r7, r7, 4
    sub             r6, r6, 6
    b               egress_to_rxdma_common
    sne             c7, r0, r0

egress_to_rxdma_arm:
    add             r6, r6, APULU_P4_TO_ARM_HDR_SZ
    phvwr           p.capri_p4_intrinsic_packet_len, r6
    phvwr           p.p4e_to_arm_rx_packet, k.control_metadata_rx_packet
    phvwr           p.p4e_to_arm_egress_bd_id, k.{vnic_metadata_egress_bd_id}.hx
    phvwr           p.{p4e_to_arm_sacl_action,p4e_to_arm_sacl_root}, \
                        k.{txdma_to_p4e_sacl_action,txdma_to_p4e_sacl_root_num}
    phvwr           p.p4e_to_arm_drop, k.txdma_to_p4e_drop
    phvwr           p.{p4e_to_arm_snat_type,p4e_to_arm_dnat_en}, \
                        k.{txdma_to_p4e_snat_type,txdma_to_p4e_dnat_en}
    phvwr           p.p4e_to_arm_dnat_id, k.{txdma_to_p4e_dnat_idx}.hx
    phvwr           p.p4e_to_arm_sacl_priority, k.{txdma_to_p4e_sacl_priority}.hx

egress_to_rxdma_common:
    phvwr           p.p4e_to_p4plus_classic_nic_packet_len, r6
    phvwr           p.p4e_to_p4plus_classic_nic_p4plus_app_id, \
                        P4PLUS_APPTYPE_CLASSIC_NIC
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                         P4PLUS_CLASSIC_NIC_HDR_SZ)

    // l2 checksum computation
    phvwr           p.p4e_to_p4plus_classic_nic_l2csum, TRUE
    phvwr.c7        p.ctag_1_l2csum, TRUE
    phvwrpair.!c7   p.ipv4_1_l2csum, k.ipv4_1_valid, \
                        p.ipv6_1_l2csum, k.ipv6_1_valid
    sub             r6, r7, 14
    phvwr           p.capri_deparser_len_l2_checksum_len, r6

    // rss type and checksum result
    bbeq            k.ethernet_2_valid, TRUE, egress_to_rxdma_rss2
    nop

    // native packet
egress_to_rxdma_rss:
    bbeq            k.ipv4_1_valid, TRUE, egress_to_rxdma_rss_ipv4
    phvwr           p.{p4e_to_p4plus_classic_nic_ip2_valid, \
                        p4e_to_p4plus_classic_nic_ip_valid, \
                        p4e_to_p4plus_classic_nic_valid}, 0x3
    seq             c1, k.ipv6_1_valid, TRUE
    nop.!c1.e
egress_to_rxdma_rss_ipv6:
    bbeq            k.tcp_valid, TRUE, egress_to_rxdma_rss_ipv6_tcp
    sll             r7, 0x1, k.capri_intrinsic_csum_err[csum_hdr_tcp]
    bbeq            k.udp_1_valid, TRUE, egress_to_rxdma_rss_ipv6_udp
    sll             r7, 0x1, k.capri_intrinsic_csum_err[csum_hdr_udp_1]
egress_to_rxdma_rss_ipv6_generic:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6
    nop
egress_to_rxdma_rss_ipv6_tcp:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_TCP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_tcp_bad, \
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss_ipv6_udp:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_UDP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_udp_bad, \
                        p4e_to_p4plus_classic_nic_csum_udp_ok}, r7
egress_to_rxdma_rss_ipv4:
    sll             r7, 0x10, k.capri_intrinsic_csum_err[csum_hdr_ipv4_1]
    bbeq            k.tcp_valid, TRUE, egress_to_rxdma_rss_ipv4_tcp
    sll             r6, 0x1, k.capri_intrinsic_csum_err[csum_hdr_tcp]
    bbeq            k.udp_1_valid, TRUE, egress_to_rxdma_rss_ipv4_udp
    sll             r6, 0x4, k.capri_intrinsic_csum_err[csum_hdr_udp_1]
egress_to_rxdma_rss_ipv4_generic:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss_ipv4_tcp:
    or              r7, r7, r6
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_TCP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss_ipv4_udp:
    or              r7, r7, r6
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_UDP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7

    // tunneled packet
egress_to_rxdma_rss2:
    bbeq            k.ipv4_2_valid, TRUE, egress_to_rxdma_rss2_ipv4
    phvwr           p.{p4e_to_p4plus_classic_nic_ip2_valid, \
                        p4e_to_p4plus_classic_nic_ip_valid, \
                        p4e_to_p4plus_classic_nic_valid}, 0x5
    seq             c1, k.ipv6_2_valid, TRUE
    nop.!c1.e
egress_to_rxdma_rss2_ipv6:
    sll             r7, 0x4, k.capri_intrinsic_csum_err[csum_hdr_udp_1]
    seq             c1, k.ipv4_1_valid, TRUE
    sll             r6, 0x10, k.capri_intrinsic_csum_err[csum_hdr_ipv4_1]
    or.c1           r7, r7, r6
    bbeq            k.tcp_valid, TRUE, egress_to_rxdma_rss2_ipv6_tcp
    sll             r6, 0x1, k.capri_intrinsic_csum_err[csum_hdr_tcp]
    bbeq            k.udp_2_valid, TRUE, egress_to_rxdma_rss2_ipv6_udp
    sll             r6, 0x4, k.capri_intrinsic_csum_err[csum_hdr_udp_2]
egress_to_rxdma_rss2_ipv6_generic:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss2_ipv6_tcp:
    or              r7, r7, r6
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_TCP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss2_ipv6_udp:
    or              r7, r7, r6
    phvwrpair       p.key_metadata_dport, k.udp_2_dstPort, \
                        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_UDP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss2_ipv4:
    or              r6, k.capri_intrinsic_csum_err[csum_hdr_ipv4_1], \
                        k.capri_intrinsic_csum_err[csum_hdr_ipv4_2]
    sll             r7, 0x10, r6
    sll             r6, 0x4, k.capri_intrinsic_csum_err[csum_hdr_udp_1]
    or.c2           r7, r7, r6
    bbeq            k.tcp_valid, TRUE, egress_to_rxdma_rss2_ipv4_tcp
    sll             r6, 0x1, k.capri_intrinsic_csum_err[csum_hdr_tcp]
    bbeq            k.udp_1_valid, TRUE, egress_to_rxdma_rss2_ipv4_udp
    sll             r5, 0x4, k.capri_intrinsic_csum_err[csum_hdr_udp_2]
egress_to_rxdma_rss2_ipv4_generic:
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss2_ipv4_tcp:
    or              r7, r7, r6
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_TCP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7
egress_to_rxdma_rss2_ipv4_udp:
    or              r7, r7, r6
    phvwrpair       p.key_metadata_dport, k.udp_2_dstPort, \
                        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.e         p.p4e_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_UDP
    phvwr.f         p.{p4e_to_p4plus_classic_nic_csum_ip_bad...\
                        p4e_to_p4plus_classic_nic_csum_tcp_ok}, r7

egress_recirc:
    phvwr           p.capri_intrinsic_tm_span_session, r0
    phvwr           p.egress_recirc_p4_to_arm_valid, k.p4e_to_arm_valid
    phvwr.e         p.egress_recirc_valid, TRUE
    phvwr.f         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4e_inter_pipe_error:
    phvwr           p.capri_intrinsic_drop, 1
    sne.e           c1, k.capri_intrinsic_tm_oq, TM_P4_RECIRC_QUEUE
    phvwr.c1        p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
