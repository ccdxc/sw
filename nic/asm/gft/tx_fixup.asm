#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct tx_fixup_k k;
struct phv_ p;

%%

tx_fixup:
    // update IP id
    smeqb           c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID
    add             r1, k.ipv4_1_identification, k.p4plus_to_p4_ip_id_delta
    phvwr.c2        p.ipv4_1_identification, r1

    // update IP length
    bbne            k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN_BIT_POS], 1, \
                        tx_fixup_upd_tcp_seq
    seq             c2, k.ctag_1_valid, TRUE
    cmov            r1, c2, 18, 14
    sub             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                        capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1
    seq             c3, k.ipv4_1_valid, TRUE
    sub.c3          r2, r1, k.ipv4_1_ihl, 2
    phvwr.c3        p.ipv4_1_totalLen, r1
    seq             c4, k.ipv6_1_valid, TRUE
    sub.c4          r2, r1, 40
    phvwr.c4        p.ipv6_1_payloadLen, r2

tx_fixup_upd_tcp_seq:
    // update TCP sequence number
    smeqb           c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO
    add             r1, k.{tcp_1_seqNo_sbit0_ebit15,tcp_1_seqNo_sbit16_ebit31}, \
                        k.p4plus_to_p4_tcp_seq_delta
    phvwr.c2        p.tcp_1_seqNo, r1

    // update UDP length
    sub             r2, r2, k.p4plus_to_p4_udp_opt_bytes
    smeqb           c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                        P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
    phvwr.c2        p.udp_1_len, r2

    // compute IP header checksum if ipv4.valid is true
    phvwr.c3        p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

    // set compute_icrc flag (rdma)
    seq             c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwr.c1        p.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
    seq             c2, k.udp_1_valid, TRUE
    seq             c3, k.tcp_1_valid, TRUE
    setcf.!c1       c1, [!c2 & !c3]
    phvwr.!c1       p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE

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
