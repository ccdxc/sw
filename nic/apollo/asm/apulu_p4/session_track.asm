#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_session_track_k.h"

struct session_track_k_ k;
struct session_track_d  d;
struct phv_             p;

%%

// R2 : tcp_data_len/tcp_exceptions_seen
// R4 : rflow_tcp_ack_num + tcp_rcvr_win_size
// R5 : tcp_seq_num_hi
// R6 : tcp_rcvr_win_size

// C5 - tcp_data_len != 0
// C6 - tcp_rcvr_win_size != 0
// C7 - tcp_seq_num_hi <= rflow_tcp_ack_num + tcp_rcvr_win_size
session_track_info:
    seq             c1, k.ipv4_1_valid, TRUE
    add.c1          r2, k.ipv4_1_ihl, k.tcp_dataOffset
    sub.c1          r2, k.ipv4_1_totalLen, r2, 2
    sub.!c1         r2, k.ipv6_1_payloadLen, k.tcp_dataOffset, 2
    bbeq            k.p4e_i2e_flow_role, TCP_FLOW_RESPONDER, \
                        tcp_responder
    add             r5, k.tcp_seqNo, r2
tcp_initiator:
    sll             r6, d.session_track_info_d.rflow_tcp_win_size, \
                        d.session_track_info_d.rflow_tcp_win_scale
    add             r4, d.session_track_info_d.rflow_tcp_ack_num, r6
    sne             c5, r2, r0
    sne             c6, r6, r0
    scwle           c7, r5, r4
    seq             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    seq.c1          c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    seq             c2, k.tcp_flags, TCP_FLAG_ACK
    seq.!c2         c2, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
    setcf           c1, [c1 & c2 & c6]
    seq             c2, k.tcp_seqNo, d.session_track_info_d.iflow_tcp_seq_num
    setcf           c2, [c5 & c2 & c7]
    sub             r1, d.session_track_info_d.rflow_tcp_ack_num, 1
    scwle           c3, r1, k.tcp_seqNo
    scwlt           c4, k.tcp_seqNo, r4
    setcf.!c2       c2, [!c5 & c3 & c4]
    bcf             ![c1 & c2], tcp_initiator_non_optimal
    scwle           c1, d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    scwle.c1        c1, k.tcp_ackNo, d.session_track_info_d.rflow_tcp_seq_num
    tblwr.c1        d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    tblwr.c1        d.session_track_info_d.iflow_tcp_win_size, k.tcp_window
    scwlt.e         c1, d.session_track_info_d.iflow_tcp_seq_num, r5
    tblwr.c1        d.session_track_info_d.iflow_tcp_seq_num, r5

tcp_initiator_non_optimal:
    add             r2, r0, r0 // tcp_exceptions_seen
    sub             r5, r5, 1

    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, 0
    add.!c1         r5, r5, 1
    bcf             ![c5 & c6], tcp_initiator_1
    seq             c1, k.tcp_seqNo, d.session_track_info_d.iflow_tcp_seq_num
    bcf             [c1 & c7], tcp_initiator_transition
    scwle           c1, d.session_track_info_d.rflow_tcp_ack_num, k.tcp_seqNo
    setcf           c1, [c1 & c7]
    b.c1            tcp_initiator_transition
    ori.c1          r2, r2, TCP_PACKET_REORDER
    scwlt           c1, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_ack_num
    scwlt           c2, r5, d.session_track_info_d.rflow_tcp_ack_num
    setcf           c1, [c1 & c2]
    b.c1            tcp_initiator_update
    ori.c1          r2, r2, TCP_FULL_REXMIT
    scwlt           c1, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_ack_num
    setcf           c1, [c1 & c7]
    b.c1            tcp_initiator_transition
    ori.c1          r2, r2, TCP_PARTIAL_OVERLAP
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    phvwr           p.capri_intrinsic_drop, 1
    b               tcp_initiator_exit
    ori             r2, r2, TCP_OUT_OF_WINDOW

tcp_initiator_1:
    bcf             ![!c5 & c6], tcp_initiator_2
    seq             c1, d.session_track_info_d.iflow_tcp_state, FLOW_STATE_INIT
    smeqb           c2, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
    setcf           c1, [c1 & c2]
    b.c1            tcp_initiator_init
    tblwr.c1        d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    // SYN Retransmit
    seq             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    add             r1, k.tcp_seqNo, 1
    seq             c3, r1, d.session_track_info_d.iflow_tcp_seq_num
    setcf           c1, [c1 & c2 & c3]
    b.c1            tcp_initiator_exit
    ori.c1          r2, r2, TCP_SYN_REXMIT
    sle             c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    b.c1            tcp_initiator_transition
    sub             r1, d.session_track_info_d.rflow_tcp_ack_num, 1
    scwle           c1, r1, k.tcp_seqNo
    scwlt           c2, k.tcp_seqNo, r4
    setcf           c1, [c1 & c2]
    b.c1            tcp_initiator_transition
    ori.!c1         r2, r2, TCP_OUT_OF_WINDOW
    phvwr.!c1       p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    b               tcp_initiator_exit
    phvwr.!c1       p.capri_intrinsic_drop, 1

tcp_initiator_init:
    seq             c1, k.control_metadata_tcp_option_ws_valid, 1
    tblwr.c1        d.session_track_info_d.iflow_tcp_win_scale, \
                        k.tcp_option_ws_value
    tblwr.c1        d.session_track_info_d.iflow_tcp_win_scale_option_sent, 1
    seq             c1, k.control_metadata_tcp_option_mss_valid, 1
    b               tcp_initiator_exit
    tblwr.c1        d.session_track_info_d.rflow_tcp_mss, k.tcp_option_mss_value

tcp_initiator_2:
    setcf           c1, [c5 & !c6]
    b.!c1           tcp_initiator_3
    ori.c1          r2, r2, TCP_WIN_ZERO_DROP
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_WIN_ZERO], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_initiator_3:
    // we will be here only if tcp_data_len == 0 and tcp_rcvr_win_size == 0
    seq             c1, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_ack_num
    sub             r1, d.session_track_info_d.rflow_tcp_ack_num, 1
    seq             c2, k.tcp_seqNo, r1
    setcf           c1, [c1 | c2]
    b.c1            tcp_initiator_transition
    ori.!c1         r2, r2, TCP_OUT_OF_WINDOW
    phvwr.!c1       p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    b               tcp_initiator_exit
    phvwr.!c1       p.capri_intrinsic_drop, 1

tcp_initiator_transition:
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST, 0
    seq             c2, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    bcf             [c1 & c2], tcp_initiator_update
    smeqb           c1, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
    b.!c1           tcp_initiator_4
    sle             c1, FLOW_STATE_ESTABLISHED, \
                        d.session_track_info_d.iflow_tcp_state
    sne             c1, d.session_track_info_d.iflow_tcp_state, FLOW_STATE_RESET
    b.!c1           tcp_initiator_exit
    slt             c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_ACK_RCVD
    b.c1            tcp_initiator_reset
    seq             c1, k.tcp_seqNo,  d.session_track_info_d.rflow_tcp_ack_num
    b.c1            tcp_initiator_reset
    b.!c1           tcp_initiator_exit
    nop

tcp_initiator_reset:
    tblwr           d.session_track_info_d.iflow_tcp_state, FLOW_STATE_RESET
    b               tcp_initiator_exit
    tblwr           d.session_track_info_d.rflow_tcp_state, FLOW_STATE_RESET

tcp_initiator_4:
    smeqb           c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
    bcf             ![c1 & c2], tcp_initiator_5
    smeqb           c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
    ori             r2, r2, TCP_UNEXPECTED_PKT
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_UNEXPECTED_PKT], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_initiator_5:
    smeqb           c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
    bcf             ![!c1 & c2], tcp_initiator_6
    add             r1, r0, d.session_track_info_d.iflow_tcp_state
    // We got a FIN in pre-established state.
    ori             r2, r2, TCP_UNEXPECTED_PKT
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_UNEXPECTED_PKT], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_initiator_6:
    // switch case based on the iflow_tcp_state
    .brbegin
    br              r1[2:0]
    nop
    .brcase         FLOW_STATE_INIT
    b               tcp_initiator_exit
    nop
    .brcase         FLOW_STATE_TCP_SYN_RCVD
    seq             c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_ACK_RCVD
    smeqb           c2, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
    seq             c3, k.tcp_ackNo, d.session_track_info_d.rflow_tcp_seq_num
    setcf           c1, [c1 & c2 & c3]
    b.!c1           tcp_initiator_syn_rcvd
    tblwr.c1        d.session_track_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
    tblwr           d.session_track_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
    tblwr           d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    tblwr           d.session_track_info_d.iflow_tcp_win_size, k.tcp_window
    add             r1, r5, 1
    b               tcp_initiator_exit
    tblwr           d.session_track_info_d.iflow_tcp_seq_num, r1
tcp_initiator_syn_rcvd:
    seq             c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    setcf           c1, [c1 & c2 &c3]
    b.!c1           tcp_initiator_exit
    tblwr.c1        d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_ACK_RCVD
    tblwr           d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    b               tcp_initiator_exit
    tblwr           d.session_track_info_d.iflow_tcp_win_size, k.tcp_window

    .brcase         FLOW_STATE_TCP_ACK_RCVD
    b               tcp_initiator_exit
    nop
    .brcase         FLOW_STATE_TCP_SYN_ACK_RCVD
    b               tcp_initiator_exit
    nop
    .brcase         FLOW_STATE_ESTABLISHED
    smeqb           c1, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
    sne             c2, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_FIN_RCVD
    setcf           c3, [c1 & c2]
    b.c3            tcp_initiator_update
    tblwr.c3        d.session_track_info_d.iflow_tcp_state, FLOW_STATE_FIN_RCVD
    setcf           c3, [c1 & !c2]
    tblwr.c3        d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_BIDIR_FIN_RCVD
    b.c3            tcp_initiator_update
    tblwr.c3        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_BIDIR_FIN_RCVD
    b               tcp_initiator_exit
    nop

    .brcase         FLOW_STATE_FIN_RCVD
    scwle           c1, d.session_track_info_d.iflow_tcp_seq_num, r5
    b.!c1           tcp_initiator_update
    ori.c1          r2, r2, TCP_DATA_AFTER_FIN
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_DATA_AFTER_FIN], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1

    .brcase         FLOW_STATE_BIDIR_FIN_RCVD
    scwle           c1, d.session_track_info_d.iflow_tcp_seq_num, r5
    b.!c1           tcp_initiator_update
    ori.c1          r2, r2, TCP_DATA_AFTER_FIN
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_DATA_AFTER_FIN], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1

    .brcase         FLOW_STATE_RESET
    ori             r2, r2, TCP_NON_RST_PKT_AFTER_RST
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_NON_RST_PKT_AFTER_RST], 1
    b               tcp_initiator_exit
    phvwr           p.capri_intrinsic_drop, 1
    .brend

tcp_initiator_update:
    scwle           c1, d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    scwle.c1        c2, k.tcp_ackNo, d.session_track_info_d.rflow_tcp_seq_num
    tblwr.c1        d.session_track_info_d.iflow_tcp_ack_num, k.tcp_ackNo
    tblwr.c1        d.session_track_info_d.iflow_tcp_win_size, k.tcp_window
    scwle           c1, d.session_track_info_d.iflow_tcp_seq_num, r5
    add             r1, r5, 1 // tcp_seq_num_hi + 1
    tblwr.c1        d.session_track_info_d.iflow_tcp_seq_num, r1

tcp_initiator_exit:
    tblor.e         d.session_track_info_d.iflow_tcp_exceptions, r2
    nop

tcp_responder:
    sll             r6, d.session_track_info_d.iflow_tcp_win_size, \
                        d.session_track_info_d.iflow_tcp_win_scale
    add             r4, d.session_track_info_d.iflow_tcp_ack_num, r6
    sne             c5, r2, r0
    sne             c6, r6, r0
    scwle           c7, r5, r4
    seq             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    seq.c1          c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    seq             c2, k.tcp_flags, TCP_FLAG_ACK
    seq.!c2         c2, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
    setcf           c1, [c1 & c2 & c6]
    seq             c2, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_seq_num
    setcf           c2, [c5 & c2 & c7]
    sub             r1, d.session_track_info_d.iflow_tcp_ack_num, 1
    scwle           c3, r1, k.tcp_seqNo
    scwlt           c4, k.tcp_seqNo, r4
    setcf.!c2       c2, [!c5 & c3 & c4]
    bcf             ![c1 & c2], tcp_responder_non_optimal
    scwle           c1, d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    scwle.c1        c1, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    tblwr.c1        d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_size, k.tcp_window
    scwlt.e         c1, d.session_track_info_d.rflow_tcp_seq_num, r5
    tblwr.c1        d.session_track_info_d.rflow_tcp_seq_num, r5

tcp_responder_non_optimal:
    add             r2, r0, r0 // tcp_exceptions_seen
    sub             r5, r5, 1

    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, 0
    add.!c1         r5, r5, 1
    bcf             ![c5 & c6], tcp_responder_1
    seq             c1, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_seq_num
    bcf             [c1 & c7], tcp_responder_transition
    scwle           c1, d.session_track_info_d.iflow_tcp_ack_num, k.tcp_seqNo
    setcf           c1, [c1 & c7]
    b.c1            tcp_responder_transition
    ori.c1          r2, r2, TCP_PACKET_REORDER
    scwlt           c1, k.tcp_seqNo, d.session_track_info_d.iflow_tcp_ack_num
    scwlt           c2, r5, d.session_track_info_d.iflow_tcp_ack_num
    setcf           c1, [c1 & c2]
    b.c1            tcp_responder_update
    ori.c1          r2, r2, TCP_FULL_REXMIT
    scwlt           c1, k.tcp_seqNo, d.session_track_info_d.rflow_tcp_ack_num
    setcf           c1, [c1 & c7]
    b.c1            tcp_responder_transition
    ori.c1          r2, r2, TCP_PARTIAL_OVERLAP
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    phvwr           p.capri_intrinsic_drop, 1
    b               tcp_responder_exit
    ori             r2, r2, TCP_OUT_OF_WINDOW

tcp_responder_1:
    bcf             ![!c5 & c6], tcp_responder_2
    slt             c1, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    slt             c2, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    smeqb           c3, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, \
                        TCP_FLAG_SYN|TCP_FLAG_ACK
    seq             c4, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    bcf             [c1 & c2 & c3 & c4], tcp_responder_transition
    smeqb           c3, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
    setcf           c3, [c1 & c2 & c3 & !c4]
    ori.c3          r2, r2, TCP_RST_WITH_INVALID_ACK_NUM
    phvwr.c3        p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_RST_WITH_INVALID_ACK_NUM], 1
    b.c3            tcp_responder_exit
    phvwr.c3        p.capri_intrinsic_drop, 1
    smeqb           c3, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
    bcf             [c1 & c2 & c3 & c4],tcp_responder_transition
    smeqb           c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
    bcf             [c1 & c2 & c3], tcp_responder_transition
    sub             r1, d.session_track_info_d.iflow_tcp_ack_num, 1
    scwle           c1, r1, k.tcp_seqNo
    scwlt           c2, k.tcp_seqNo, r4
    setcf           c1, [c1 & c2]
    b.c1            tcp_responder_transition
    ori.!c1         r2, r2, TCP_OUT_OF_WINDOW
    phvwr.!c1       p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    b               tcp_responder_exit
    phvwr.!c1       p.capri_intrinsic_drop, 1

tcp_responder_2:
    setcf           c1, [c5 & !c6]
    b.!c1           tcp_responder_3
    ori.c1          r2, r2, TCP_WIN_ZERO_DROP
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_WIN_ZERO], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_responder_3:
    // we will be here only if tcp_data_len == 0 and tcp_rcvr_win_size == 0
    seq             c1, k.tcp_seqNo, d.session_track_info_d.iflow_tcp_ack_num
    sub             r1, d.session_track_info_d.iflow_tcp_ack_num, 1
    seq             c2, k.tcp_seqNo, r1
    setcf           c1, [c1 | c2]
    b.c1            tcp_responder_transition
    ori.!c1         r2, r2, TCP_OUT_OF_WINDOW
    phvwr.!c1       p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_OUT_OF_WINDOW], 1
    b               tcp_responder_exit
    phvwr.!c1       p.capri_intrinsic_drop, 1

tcp_responder_transition:
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST, 0
    seq             c2, d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    bcf             [c1 & c2], tcp_responder_update
    smeqb           c1, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
    sle             c1, FLOW_STATE_ESTABLISHED, \
                        d.session_track_info_d.rflow_tcp_state
    sne             c1, d.session_track_info_d.rflow_tcp_state, FLOW_STATE_RESET
    b.!c1           tcp_responder_exit
    slt             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_ACK_RCVD
    b.c1            tcp_responder_reset
    seq             c1, k.tcp_seqNo, d.session_track_info_d.iflow_tcp_ack_num
    b.c1            tcp_responder_reset
    b.!c1           tcp_responder_exit
    nop

tcp_responder_reset:
    tblwr           d.session_track_info_d.rflow_tcp_state, FLOW_STATE_RESET
    b               tcp_responder_exit
    tblwr           d.session_track_info_d.iflow_tcp_state, FLOW_STATE_RESET

tcp_responder_4:
    smeqb           c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
    bcf             ![c1 & c2], tcp_responder_5
    smeqb           c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
    ori             r2, r2, TCP_UNEXPECTED_PKT
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_UNEXPECTED_PKT], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_responder_5:
    bcf             ![!c1 & c2], tcp_responder_6
    add             r1, r0, d.session_track_info_d.rflow_tcp_state
    // We got a FIN in pre-established state.
    ori             r2, r2, TCP_UNEXPECTED_PKT
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_UNEXPECTED_PKT], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1

tcp_responder_6:
    // switch case based on the rflow_tcp_state.
    .brbegin
    br              r1[2:0]
    nop

    .brcase         FLOW_STATE_INIT
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, \
                        TCP_FLAG_SYN|TCP_FLAG_ACK
    seq             c2, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    setcf           c1, [c1 & c2]
    b.!c1           tcp_responder_init_1
    tblwr.c1        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_ACK_RCVD
    seq             c1, d.session_track_info_d.iflow_tcp_win_scale_option_sent, 1
    seq.c1          c1, k.control_metadata_tcp_option_ws_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_scale, \
                        k.tcp_option_ws_value
    tblwr.!c1       d.session_track_info_d.iflow_tcp_win_scale, 0
    seq             c1, k.control_metadata_tcp_option_mss_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_mss, k.tcp_option_mss_value
    add             r1, r5, 1 // tcp_seq_num_hi + 1
    tblwr           d.session_track_info_d.rflow_tcp_seq_num, r1
    tblwr           d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    b               tcp_responder_exit
    tblwr           d.session_track_info_d.rflow_tcp_win_size, k.tcp_window

tcp_responder_init_1:
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_ACK
    seq             c2, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    setcf           c1, [c1 & c2]
    b.!c1           tcp_responder_init_2
    ori.c1          r2, r2, TCP_SPLIT_HANDSHAKE_DETECTED
    tblwr           d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_ACK_RCVD
    tblwr           d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    b               tcp_responder_exit
    tblwr           d.session_track_info_d.rflow_tcp_win_size, k.tcp_window

tcp_responder_init_2:
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
    seq             c2, k.tcp_ackNo, 0
    setcf           c1, [c1 & c2]
    b.!c1           tcp_responder_init_3
    ori.c1          r2, r2, TCP_SPLIT_HANDSHAKE_DETECTED
    tblwr           d.session_track_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
    seq             c1, d.session_track_info_d.iflow_tcp_win_scale_option_sent, 1
    seq.c1          c1, k.control_metadata_tcp_option_ws_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_scale, \
                        k.tcp_option_ws_value
    tblwr.!c1       d.session_track_info_d.iflow_tcp_win_scale, 0
    seq             c1, k.control_metadata_tcp_option_mss_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_mss, k.tcp_option_mss_value
    add             r1, r5, 1 // tcp_seq_num_hi + 1
    b               tcp_responder_exit
    tblwr           d.session_track_info_d.rflow_tcp_seq_num, r1

tcp_responder_init_3:
    ori             r2, r2, TCP_INVALID_RESPONDER_FIRST_PKT
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_INVALID_RESPONDER_FIRST_PKT], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1


    .brcase         FLOW_STATE_TCP_SYN_RCVD
    seq             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    smeqb           c2, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
    seq             c3, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    setcf           c1, [c1 & c2 & c3]
    tblwr.c1        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_ACK_RCVD
    tblwr.c1        d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    b.c1            tcp_responder_exit
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_size, k.tcp_window
    seq             c1, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_ACK_RCVD
    setcf           c1, [c1 & c2 & c3]
    tblwr.c1        d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    b.c1            tcp_responder_update
    tblwr.c1        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_ESTABLISHED
    b               tcp_responder_exit
    nop

    .brcase         FLOW_STATE_TCP_ACK_RCVD
    smeqb           c1, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
    b.!c1           tcp_responder_exit
    tblwr.c1        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_TCP_SYN_RCVD
    seq             c1, d.session_track_info_d.iflow_tcp_win_scale_option_sent, 1
    seq.c1          c1, k.control_metadata_tcp_option_ws_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_scale, \
                        k.tcp_option_ws_value
    tblwr.!c1       d.session_track_info_d.iflow_tcp_win_scale, 0
    seq             c1, k.control_metadata_tcp_option_mss_valid, 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_mss, k.tcp_option_mss_value
    add             r1, r5, 1 // tcp_seq_num_hi + 1
    b               tcp_responder_exit
    tblwr           d.session_track_info_d.rflow_tcp_seq_num, r1

    .brcase         FLOW_STATE_TCP_SYN_ACK_RCVD
    b               tcp_responder_exit
    nop

    .brcase         FLOW_STATE_ESTABLISHED
    smeqb           c1, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
    sne             c2, d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_FIN_RCVD
    setcf           c3, [c1 & c2]
    tblwr.c3        d.session_track_info_d.rflow_tcp_state, FLOW_STATE_FIN_RCVD
    b.c3            tcp_responder_update
    setcf           c3, [c1 & !c2]
    tblwr.c3        d.session_track_info_d.rflow_tcp_state, \
                        FLOW_STATE_BIDIR_FIN_RCVD
    tblwr.c3        d.session_track_info_d.iflow_tcp_state, \
                        FLOW_STATE_BIDIR_FIN_RCVD
    b.c3            tcp_responder_update
    b.!c3           tcp_responder_exit
    nop

    .brcase         FLOW_STATE_FIN_RCVD
    scwle           c1, d.session_track_info_d.rflow_tcp_seq_num, r5
    b.!c1           tcp_responder_update
    ori.c1          r2, r2, TCP_DATA_AFTER_FIN
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_DATA_AFTER_FIN], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1

    .brcase         FLOW_STATE_BIDIR_FIN_RCVD
    scwle           c1, d.session_track_info_d.rflow_tcp_seq_num, r5
    b.!c1           tcp_responder_update
    ori.c1          r2, r2, TCP_DATA_AFTER_FIN
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_DATA_AFTER_FIN], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1

    .brcase         FLOW_STATE_RESET
    ori             r2, r2, TCP_NON_RST_PKT_AFTER_RST
    phvwr           p.control_metadata_p4e_drop_reason[P4E_DROP_TCP_NON_RST_PKT_AFTER_RST], 1
    b               tcp_responder_exit
    phvwr           p.capri_intrinsic_drop, 1
    .brend

tcp_responder_update:
    scwle           c1, d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    scwle.c1        c2, k.tcp_ackNo, d.session_track_info_d.iflow_tcp_seq_num
    tblwr.c1        d.session_track_info_d.rflow_tcp_ack_num, k.tcp_ackNo
    tblwr.c1        d.session_track_info_d.rflow_tcp_win_size, k.tcp_window
    scwle           c1, d.session_track_info_d.rflow_tcp_seq_num, r5
    add             r1, r5, 1 // tcp_seq_num_hi + 1
    tblwr.c1        d.session_track_info_d.rflow_tcp_seq_num, r1

tcp_responder_exit:
    tblor.e         d.session_track_info_d.rflow_tcp_exceptions, r2
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_track_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
