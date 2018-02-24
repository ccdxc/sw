/*
 *    Implements the tx2rx shared state read stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_k_ k;
struct common_p4plus_stage0_app_header_table_read_tx2rx_d d;

#define COMMA     ,

%%

    .param          tcp_rx_read_tls_stage0_start
    .align
tcp_rx_read_shared_stage0_start:
#ifdef CAPRI_IGNORE_TIMESTAMP
    add             r4, r0, r0
    add             r6, r0, r0
#endif
    /* Debug instruction */
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_0, CAPRI_MPU_TABLE_0)

    tblwr           d.rx_ts, r4
    phvwr           p.common_phv_debug_dol, d.debug_dol

    add             r1, r0, k.p4_rxdma_intr_qstate_addr

    /* Write all the tx to rx shared state from table data into phv */

    /* Set the flow id in the phv global area for the rest of
     * the stages to use it
     */
    phvwr           p.common_phv_fid, k.p4_rxdma_intr_qid
    smeqb           c1, k.tcp_app_header_flags, TCPHDR_SYN, TCPHDR_SYN
    phvwri.c1       p.common_phv_syn, 1
    smeqb           c1, k.tcp_app_header_flags, TCPHDR_FIN, TCPHDR_FIN
    phvwri.c1       p.common_phv_fin, 1
    and             r2, k.tcp_app_header_flags, TCPHDR_ACK
    /* If we see a pure SYN drop it */
    sne             c1, r1, r0
    seq             c2, r2, r0
    setcf           c3, [c1 & c2]
    phvwri.c3       p.p4_intr_global_drop, 1
    bcf             [c3], flow_terminate
    nop

    and             r1, k.tcp_app_header_flags, TCPHDR_ECE
    phvwr           p.common_phv_ece, r1

    //TODO: move to s2s phvwr        p.common_phv_rcv_tsecr, k.tcp_app_header_prev_echo_ts

    /* Setup the to-stage/stage-to-stage variables based
     * on the p42p4+ app header info
     */
    phvwr           p.to_s2_flags, k.tcp_app_header_flags
    phvwr           p.to_s2_seq, k.tcp_app_header_seqNo
    phvwr           p.to_s2_ack_seq, k.tcp_app_header_ackNo
    phvwr           p.{cpu_hdr2_tcp_window_1,cpu_hdr3_tcp_window_2}, k.{tcp_app_header_window}.hx

    //phvwr        p.prr_out, d.prr_out
    phvwr           p.to_s2_snd_nxt, d.snd_nxt
    phvwr           p.to_s2_snd_nxt, d.snd_nxt
    //phvwr        p.ecn_flags_tx, d.ecn_flags_tx
    phvwr           p.common_phv_ecn_flags, d.ecn_flags_tx
    phvwr           p.s1_s2s_packets_out, d.packets_out
    phvwr           p.s1_s2s_rcv_wup, d.rcv_wup
    phvwr           p.common_phv_qstate_addr, k.p4_rxdma_intr_qstate_addr

    /*
     * Adjust quick based on acks sent in tx pipeline
     *
     * p.s1_s2s_quick_acks_decr = tx_quick_acks_decr - rx_quick_acks_decr
     * rx_quick_acks_decr = tx_quick_acks_decr
     */
    sub             r1, d.quick_acks_decr, d.quick_acks_decr_old
    tblwr           d.quick_acks_decr_old, d.quick_acks_decr
    phvwr           p.s1_s2s_quick_acks_decr, r1

    phvwr           p.s1_s2s_fin_sent, d.fin_sent
    tblwr           d.fin_sent, 0

    phvwr           p.to_s6_payload_len, k.tcp_app_header_payload_len
    phvwr           p.s1_s2s_payload_len, k.tcp_app_header_payload_len

    phvwrpair       p.to_s3_rcv_tsval[31:8], k.tcp_app_header_ts_s0_e23, \
                        p.to_s3_rcv_tsval[7:0], k.tcp_app_header_ts_s24_e31
    phvwrpair       p.s1_s2s_rcv_tsval[31:8], k.tcp_app_header_ts_s0_e23, \
                        p.s1_s2s_rcv_tsval[7:0], k.tcp_app_header_ts_s24_e31
    phvwr           p.to_s3_rcv_tsecr, k.tcp_app_header_prev_echo_ts
    CAPRI_OPERAND_DEBUG(k.tcp_app_header_payload_len)

read_l7_proxy_cfg:
    sne         c1, d.l7_proxy_type, L7_PROXY_TYPE_NONE
    phvwri.c1   p.common_phv_l7_proxy_en, 1
    seq         c2, d.l7_proxy_type, L7_PROXY_TYPE_REDIR
    phvwri.c2   p.common_phv_l7_proxy_type_redirect, 1

table_read_RX:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                tcp_rx_read_tls_stage0_start, d.tls_stage0_ring0_addr,
                TABLE_SIZE_32_BITS)
flow_terminate:
    nop.e
    nop
