#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4plus_to_p4_1_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct p4plus_to_p4_1_k_ k;
struct phv_ p;

%%

f_p4plus_to_p4_1:
    bbne            k.p4plus_to_p4_update_ip_len, TRUE, p4plus_to_p4_1_update_udp_len
    crestore        [c3-c1], k.{udp_valid,ipv4_valid,ipv6_valid}, 0x7
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l3_1
    phvwr.c2        p.ipv4_totalLen, r1
    sub             r1, r1, 40
    phvwr.c1        p.ipv6_payloadLen, r1
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_1
    phvwr.c3        p.udp_len, r1
    crestore        [c3-c1], k.{inner_udp_valid,inner_ipv4_valid,inner_ipv6_valid}, 0x7
    bcf             [!c2 & !c1], p4plus_to_p4_1_update_udp_len
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l3_2
    phvwr.c2        p.inner_ipv4_totalLen, r1
    sub             r1, r1, 40
    phvwr.c1        p.inner_ipv6_payloadLen, r1
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_2
    phvwr.c3        p.inner_udp_len, r1

p4plus_to_p4_1_update_udp_len:
    // update UDP len
    bbne            k.p4plus_to_p4_update_udp_len, TRUE, p4plus_to_p4_1_update_ip_id
    phvwr           p.control_metadata_udp_opt_bytes, k.p4plus_to_p4_udp_opt_bytes
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_1
    sub             r1, r1, k.p4plus_to_p4_udp_opt_bytes
    phvwr           p.udp_len, r1

p4plus_to_p4_1_update_ip_id:
    // update IP id
    bbne            k.p4plus_to_p4_update_ip_id, TRUE, p4plus_to_p4_1_update_tcp_seq_no
    seq             c1, k.inner_ipv4_valid, TRUE
    cmov            r1, c1, k.inner_ipv4_identification, k.ipv4_identification
    add             r1, r1, k.p4plus_to_p4_ip_id_delta
    phvwr.c1        p.inner_ipv4_identification, r1
    phvwr.!c1       p.ipv4_identification, r1


p4plus_to_p4_1_update_tcp_seq_no:
    // update TCP sequence number
    bbne            k.p4plus_to_p4_update_tcp_seq_no, TRUE, p4plus_to_p4_1_update_tcp_flags
    add             r1, k.tcp_seqNo, k.p4plus_to_p4_tcp_seq_delta
    phvwr           p.tcp_seqNo, r1

p4plus_to_p4_1_update_tcp_flags:
    // update tcp flags
    bbne            k.p4plus_to_p4_tso_valid, TRUE, p4plus_to_p4_1_insert_vlan_tag
    crestore        [c2-c1], k.{p4plus_to_p4_tso_last_segment, \
                        p4plus_to_p4_tso_first_segment}, 0x3
    phvwrmi.!c1     p.tcp_flags, 0, TCP_FLAG_CWR
    phvwrmi.!c2     p.tcp_flags, 0, (TCP_FLAG_FIN|TCP_FLAG_PSH)

p4plus_to_p4_1_insert_vlan_tag:
    // insert vlan tag
    bbne            k.p4plus_to_p4_insert_vlan_tag, TRUE, p4plus_to_p4_1_update_cpu_flags
    phvwr           p.control_metadata_src_app_id, k.p4plus_to_p4_p4plus_app_id
    phvwr           p.vlan_tag_valid, TRUE
    phvwrpair       p.{vlan_tag_pcp...vlan_tag_vid}, \
                        k.{p4plus_to_p4_vlan_pcp...p4plus_to_p4_vlan_vid}, \
                        p.vlan_tag_etherType, k.ethernet_etherType
    phvwr           p.ethernet_etherType, ETHERTYPE_VLAN

p4plus_to_p4_1_update_cpu_flags:
    // update "from CPU" flag
    seq             c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
    phvwr.c1        p.control_metadata_from_cpu, TRUE

    // update flow_index
    seq.e           c1, k.p4plus_to_p4_flow_index_valid, TRUE
    phvwr.c1        p.flow_info_metadata_flow_index, k.p4plus_to_p4_flow_index


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_to_p4_1_error:
    nop.e
    nop
