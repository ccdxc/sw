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

%%

    .align
tcp_rx_read_shared_stage0_start_ext:
    CAPRI_CLEAR_TABLE_VALID(1)

    // note: for feedback packets, these fields are valid for LAST_OOO_PKT
    // but not WIN_UPD. For WIN_UPD the following phvwr are a NOP
    phvwrpair       p.s1_s2s_payload_len, k.tcp_app_header_payload_len, \
                        p.s1_s2s_seq, k.tcp_app_header_seqNo

    seq             c1, k.tcp_app_header_from_ooq_txdma, 1
    // HACK, For tx2rx feedback packets 1 byte following tcp_app_header contains
    // pkt type. This falls in app_data1 region of common rxdma phv.
    // Until we can unionize this header correctly in p4, hardcoding the PHV
    // location for now. This is prone to error, but hopefully if something
    // breaks, we have DOL test cases to catch it.  (refer to
    // iris/gen/p4gen/tcp_proxy_rxdma/asm_out/INGRESS_p.h)
    seq             c2, k._tcp_app_header_end_pad_88[15:8], TCP_TX2RX_FEEDBACK_WIN_UPD
    seq.!c2         c2, k._tcp_app_header_end_pad_88[15:8], TCP_TX2RX_FEEDBACK_LAST_OOO_PKT
    bcf             [c1 & c2], tcp_rx_read_shared_stage0_end_ext

    /* Setup the to-stage/stage-to-stage variables based
     * on the p42p4+ app header info
     */
    phvwr           p.to_s1_data_ofs_rsvd[7:4], k.tcp_app_header_dataOffset
    phvwrpair       p.to_s1_rcv_wup, d.rcv_wup, \
                        p.to_s1_rcv_wnd_adv, d.rcv_wnd_adv
    //phvwr           p.cpu_hdr3_tcp_window, k.{tcp_app_header_window}.hx
    phvwrpair       p.s1_s2s_rcv_tsval[31:8], k.tcp_app_header_ts_s0_e23, \
                        p.s1_s2s_rcv_tsval[7:0], k.tcp_app_header_ts_s24_e31
    phvwrpair       p.s1_s2s_ack_seq, k.tcp_app_header_ackNo, \
                        p.s1_s2s_snd_nxt, d.snd_nxt
    phvwr           p.to_s2_window, k.tcp_app_header_window
tcp_rx_read_shared_stage0_end_ext:
    nop.e
    nop
