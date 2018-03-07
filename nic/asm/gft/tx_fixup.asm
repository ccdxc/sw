#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_fixup_k k;
struct phv_ p;

#define TXDMA_SZ (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_TXDMA_INTRINSIC_HDR_SZ + P4PLUS_TO_P4_HDR_SZ)

%%

tx_fixup:
    phvwr           p.flow_action_metadata_tx_ethernet_dst, k.ethernet_1_dstAddr
    seq             c7, k.control_metadata_skip_flow_lkp, TRUE
    phvwr.c7        p.flow_action_metadata_flow_index, k.p4plus_to_p4_flow_index

    seq             c3, k.ipv4_1_valid, TRUE
    seq             c4, k.ipv6_1_valid, TRUE

    // update packet length
    sub             r7, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5, \
                           capri_p4_intrinsic_frame_size_sbit6_ebit13}, TXDMA_SZ
    phvwr           p.capri_p4_intrinsic_packet_len, r7

    seq             c1, k.ctag_1_valid, TRUE
    cmov            r1, c1, 18, 14
    sub             r1, r7, r1
    sub.c3          r2, r1, k.ipv4_1_ihl, 2
    sub.c4          r2, r1, 40

    // update IP id
    smeqb           c1, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID
    add             r3, k.ipv4_1_identification, k.p4plus_to_p4_ip_id_delta
    phvwr.c1        p.ipv4_1_identification, r3

    // update IP length
    bbne            k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN_BIT_POS], 1, \
                        tx_fixup_upd_tcp_seq
    phvwr.c3        p.ipv4_1_totalLen, r1
    phvwr.c4        p.ipv6_1_payloadLen, r2

tx_fixup_upd_tcp_seq:
    // update TCP sequence number
    smeqb           c1, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO
    add             r3, k.{tcp_1_seqNo_sbit0_ebit15,tcp_1_seqNo_sbit16_ebit31}, \
                        k.p4plus_to_p4_tcp_seq_delta
    phvwr.c1        p.tcp_1_seqNo, r3

    // update UDP length
    sub             r3, r2, k.p4plus_to_p4_udp_opt_bytes
    smeqb           c1, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
    phvwr.c1        p.udp_1_len, r3

    // compute IP header checksum if ipv4.valid is true
    phvwr.c3        p.ipv4_1_csum, TRUE

    // set compute_icrc flag (rdma)
    seq             c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
    bcf             [!c1], tx_fixup_rdma_done
    .assert(offsetof(p, roce_bth_1_icrc) - offsetof(p, udp_1_icrc) == 10)
    phvwrmi.c1      p.{roce_bth_1_icrc...udp_1_icrc}, 0xFFFF, 0x401
    .assert(offsetof(p, ipv4_1_icrc) - offsetof(p, ipv6_1_icrc) == 9)
    or              r7, k.ipv6_1_valid, k.ipv4_1_valid, 9
    phvwrm.c1       p.{ipv4_1_icrc...ipv6_1_icrc}, r7, 0x201
    sub             r1, r1, k.p4plus_to_p4_udp_opt_bytes
    bcf             [c1], tx_fixup_done
    phvwr.c1.e      p.capri_deparser_len_icrc_payload_len, r1

tx_fixup_rdma_done:
    phvwr           p.capri_deparser_len_tx_l4_payload_len, r3
    // set l4 checksum flags
    .assert(offsetof(p, udp_1_csum) - offsetof(p, ipv4_1_udp_csum) == 3)
    or              r7, k.udp_1_valid, k.udp_1_valid, 3
    phvwrm          p.{udp_1_csum...ipv4_1_udp_csum}, r7, 0x9
    phvwrpair.e     p.tcp_1_csum, k.tcp_1_valid, p.ipv4_1_tcp_csum, k.tcp_1_valid

tx_fixup_done:
    // remove the headers
    phvwr.f         p.{p4plus_to_p4_valid,capri_txdma_intrinsic_valid}, 0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_fixup_error:
    nop.e
    nop
