#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_fixup_k k;
struct phv_ p;

%%

tx_fixup:
    seq             c3, k.ipv4_1_valid, TRUE
    seq             c4, k.ipv6_1_valid, TRUE

    seq             c1, k.ctag_1_valid, TRUE
    cmov            r1, c1, 18, 14
    sub             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                        capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1
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
    // TDB: seq             c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
    seq             c1, k.udp_1_valid, TRUE
    phvwrpair.c1    p.roce_bth_1_icrc, TRUE, p.udp_1_icrc, TRUE
    phvwrpair.c1    p.ipv4_1_icrc, k.ipv4_1_valid, p.ipv6_1_icrc, k.ipv6_1_valid
    sub             r1, r1, k.p4plus_to_p4_udp_opt_bytes
    bcf             [c1], tx_fixup_done
    phvwr.c1        p.capri_deparser_len_icrc_payload_len, r1

    // set l4 checksum flags
    phvwr           p.capri_deparser_len_tx_l4_payload_len, r3
    phvwrpair       p.udp_1_csum, k.udp_1_valid, p.ipv4_1_udp_csum, k.udp_1_valid
    phvwrpair       p.tcp_1_csum, k.tcp_1_valid, p.ipv4_1_tcp_csum, k.tcp_1_valid

tx_fixup_done:
    // remove the headers
    phvwr.e         p.capri_txdma_intrinsic_valid, FALSE
    phvwr           p.p4plus_to_p4_valid, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_fixup_error:
    nop.e
    nop
