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
tcp_rx_read_shared_stage0_start_ext:
    /* Setup the to-stage/stage-to-stage variables based
     * on the p42p4+ app header info
     */
    phvwr           p.to_s2_flags, k.tcp_app_header_flags
    phvwrpair       p.to_s2_seq, k.tcp_app_header_seqNo, \
                        p.to_s2_ack_seq, k.tcp_app_header_ackNo
    phvwr           p.{cpu_hdr2_tcp_window_1,cpu_hdr3_tcp_window_2}, k.{tcp_app_header_window}.hx

    phvwr           p.to_s2_snd_nxt, d.snd_nxt

    phvwrpair       p.to_s3_rcv_tsval[31:8], k.tcp_app_header_ts_s0_e23, \
                        p.to_s3_rcv_tsval[7:0], k.tcp_app_header_ts_s24_e31
    phvwr.f         p.s1_s2s_payload_len, k.tcp_app_header_payload_len

    nop.e
    nop
