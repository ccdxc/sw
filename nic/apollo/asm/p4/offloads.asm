#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_offloads_k.h"

struct offloads_k_ k;
struct phv_ p;

%%

// c4 : ipv4_1_valid
// c5 : ipv6_1_valid
// c6 : update_ip_chksum
// c7 : update_l4_chksum
// r7 : l4 length
offloads:
    seq             c4, k.ipv4_1_valid, TRUE
    seq             c5, k.ipv6_1_valid, TRUE
    bbne            k.p4plus_to_p4_update_ip_len, TRUE, offloads_get_l4_len
    add             r7, r0, r0
    seq             c1, k.ctag_1_valid, TRUE
    cmov            r7, c1, 18, 14
    sub             r7, k.capri_p4_intrinsic_packet_len, r7
    phvwr.c4        p.ipv4_1_totalLen, r7
    sub.c4          r7, r7, k.ipv4_1_ihl, 2
    seq.c4          c6, r0, r0
    sub.c5          r7, r7, 40
    b               offloads_update_l4_len
    phvwr.c5        p.ipv6_1_payloadLen, r7

offloads_get_l4_len:
    sub.c4          r7, k.ipv4_1_totalLen, k.ipv4_1_ihl, 2
    add.c5          r7, r0, k.ipv6_1_payloadLen

offloads_update_l4_len:
    seq             c1, k.udp_1_valid, TRUE
    sub.c1          r7, r7, k.p4plus_to_p4_udp_opt_bytes

offloads_update_ip_id:
    bbne            k.p4plus_to_p4_update_ip_id, TRUE, offloads_update_udp_len
    add             r1, k.ipv4_1_identification, k.p4plus_to_p4_ip_id_delta
    phvwr           p.ipv4_1_identification, r1
    seq             c6, r0, r0

offloads_update_udp_len:
    bbne            k.p4plus_to_p4_update_udp_len, TRUE, offloads_update_tcp_seq_no
    seq             c3, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
    phvwr           p.udp_1_len, r7
    seq             c7, r0, r0

offloads_update_tcp_seq_no:
    bbne            k.p4plus_to_p4_update_tcp_seq_no, TRUE, offloads_tso
    add             r1, k.tcp_seqNo, k.p4plus_to_p4_tcp_seq_delta
    phvwr           p.tcp_seqNo, r1
    seq             c7, r0, r0

offloads_tso:
    bbne            k.p4plus_to_p4_tso_valid, TRUE, offloads_insert_vlan_tag
    crestore        [c2-c1], k.{p4plus_to_p4_tso_last_segment, \
                        p4plus_to_p4_tso_first_segment}, 0x3
    phvwrmi.!c1     p.tcp_flags, 0, TCP_FLAG_CWR
    phvwrmi.!c2     p.tcp_flags, 0, (TCP_FLAG_FIN|TCP_FLAG_PSH)
    setcf           c7, [c1|c2]

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
    bcf             [!c3], offloads_update_checksum_flags
    crestore.!c3    [c7-c6], 0x3, 0x3
    seq             c1, k.p4plus_to_p4_compute_ip_csum, TRUE
    andcf           c1, [c4]
    seq.c1          c6, r0, r0
    seq             c1, k.p4plus_to_p4_compute_l4_csum, TRUE
    b               offloads_update_checksum_flags
    seq.c1          c7, r0, r0

offloads_update_checksum_flags:
    add             r1, r0, k.ipv4_1_ihl, 2
    phvwr.c6        p.capri_deparser_len_ipv4_1_hdr_len, r1
    phvwr.c6        p.ipv4_1_csum, 1

    nop.!c7.e
    phvwr.c7        p.capri_deparser_len_l4_payload_len, r7
    bbne            k.tcp_valid, TRUE, offloads_update_checksum_flags_non_tcp
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c4        p.ipv4_1_tcp_csum, 1
    phvwr.e         p.tcp_csum, 1
    phvwr.c5        p.ipv6_1_tcp_csum, 1

offloads_update_checksum_flags_non_tcp:
    nop.!c1.e
    nop
    phvwr.c4        p.ipv4_1_udp_csum, 1
    phvwr.e         p.udp_1_csum, 1
    phvwr.c5        p.ipv6_1_tcp_csum, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
offloads_error:
    nop.e
    nop
