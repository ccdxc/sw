#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_offloads_k.h"

struct offloads_k_ k;
struct phv_ p;

%%

offloads:
    crestore        [c3-c1], k.{udp_1_valid,ipv4_1_valid,ipv6_1_valid}, 0x7
    bcf             [!c2 & !c1], offloads_end
    nop
    bbne            k.p4plus_to_p4_update_ip_len, TRUE, offloads_update_ip_id
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l3_1
    phvwr.c2        p.ipv4_1_totalLen, r1
    sub             r1, r1, 40
    phvwr.c1        p.ipv6_1_payloadLen, r1
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_1
    phvwr.c3        p.udp_1_len, r1
    crestore        [c7-c1], k.{udp_2_valid,udp_2_csum,ipv4_2_valid, \
                        ipv4_2_udp_csum,ipv4_2_tcp_csum,ipv4_2_csum, \
                        ipv6_2_valid}, 0x7F
    bcf             [!c5 & !c1], offloads_update_ip_id
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l3_2
    phvwr.c5        p.ipv4_2_totalLen, r1
    sub             r1, r1, 40
    phvwr.c1        p.ipv6_2_payloadLen, r1
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_2
    phvwr.c7        p.udp_2_len, r1

offloads_update_ip_id:
    bbne            k.p4plus_to_p4_update_ip_id, TRUE, offloads_update_tcp_seq_no
    add             r1, k.ipv4_1_identification, k.p4plus_to_p4_ip_id_delta
    phvwr           p.ipv4_1_identification, r1

offloads_update_tcp_seq_no:
    bbne            k.p4plus_to_p4_update_tcp_seq_no, TRUE, offloads_tso
    add             r1, k.tcp_seqNo, k.p4plus_to_p4_tcp_seq_delta
    phvwr           p.tcp_seqNo, r1

offloads_tso:
    bbne            k.p4plus_to_p4_tso_valid, TRUE, offloads_insert_vlan_tag
    crestore        [c2-c1], k.{p4plus_to_p4_tso_last_segment, \
                        p4plus_to_p4_tso_first_segment}, 0x3
    phvwrmi.!c1     p.tcp_flags, 0, TCP_FLAG_CWR
    phvwrmi.!c2     p.tcp_flags, 0, (TCP_FLAG_FIN|TCP_FLAG_PSH)

offloads_insert_vlan_tag:
    bbne            k.p4plus_to_p4_insert_vlan_tag, TRUE, offloads_checksum
    add             r1, k.capri_p4_intrinsic_packet_len, 4
    phvwr           p.ctag_1_valid, TRUE
    phvwrpair       p.{ctag_1_pcp...ctag_1_vid}, \
                        k.{p4plus_to_p4_vlan_pcp...p4plus_to_p4_vlan_vid}, \
                        p.ctag_1_etherType, k.ethernet_1_etherType
    phvwr           p.ethernet_1_etherType, ETHERTYPE_VLAN
    phvwr           p.capri_p4_intrinsic_packet_len, r1

offloads_checksum:
    seq             c1, k.p4plus_to_p4_compute_ip_csum, TRUE
    seq.!c1         c1, k.p4plus_to_p4_compute_l4_csum, TRUE
    phvwr.c1        p.p4i_i2e_update_checksum, TRUE
    seq             c1, k.p4plus_to_p4_compute_inner_ip_csum, TRUE
    seq             c2, k.p4plus_to_p4_compute_inner_l4_csum, TRUE
    bcf             [!c1 & !c2], offloads_end
    phvwr           p.capri_gso_csum_gso, k.p4plus_to_p4_gso_valid
offloads_checksum_inner:
    sll             r1, k.ipv4_2_ihl, 2
    phvwr           p.capri_deparser_len_ipv4_2_hdr_len, r1
    bcf             [!c2], offloads_end
    phvwr.c1        p.ipv4_2_csum, 1
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_2
    bbeq            k.tcp_valid, TRUE, offloads_checksum_inner_tcp
    seq             c1, k.ipv4_2_valid, TRUE
    bbne            k.udp_2_valid, TRUE, offloads_end
    nop
    phvwrpair.c1    p.udp_2_csum, 1, p.ipv4_2_udp_csum, 1
    phvwr.e         p.capri_deparser_len_l4_payload_len, r1
    phvwrpair.!c1   p.udp_2_csum, 1, p.ipv6_2_udp_csum, 1

offloads_checksum_inner_tcp:
    phvwrpair.c1    p.tcp_csum, 1, p.ipv4_2_tcp_csum, 1
    phvwr.e         p.capri_deparser_len_l4_payload_len, r1
    phvwrpair.!c1   p.tcp_csum, 1, p.ipv6_2_tcp_csum, 1

offloads_end:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
offloads_error:
    nop.e
    nop
