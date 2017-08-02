#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct session_state_k k;
struct session_state_d d;
struct phv_               p;

%%

#if 0
nop:
  nop.e
  nop
#endif /* 0 */
// Code is written to match the P4 program as much as possible to
// facilitate changes to assemmbly easily. Once the P4 Program is
// final and don't anticipate any changes we can optimize the
// assembly code. If we optimize the assembly code right now
// then even a single change in P4 program can invalidate the
// complete assembly code.

// Pick the initator or responder values to use.
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tcp_session_state_info:
  seq          c1, k.flow_info_metadata_flow_role, TCP_FLOW_INITIATOR
  b.!c1        lb_tcp_session_state_responder
  seq          c1, k.l4_metadata_tcp_normalization_en, 1

lb_tcp_session_state_initiator:
  add          r1, d.u.tcp_session_state_info_d.rflow_tcp_win_scale, r0
  sllv         r6, d.u.tcp_session_state_info_d.rflow_tcp_win_sz, r7   // r6 = rcvr_win_sz
  add          r4, d.u.tcp_session_state_info_d.rflow_tcp_mss, r4
  bal.c1       r3, f_tcp_session_normalization
  add          r7, k.tcp_ackNo, d.u.tcp_session_state_info_d.syn_cookie_delta // r7 = adjusted_ack_num
  add          r5, k.tcp_seqNo, k.l4_metadata_tcp_data_len  // tcp_seq_num_hi
  add          r4, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, r6 // rflow_tcp_ack_num + rcvr_win_sz
  seq          c1, k.tcp_flags, TCP_FLAG_ACK
  seq          c2, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
  setcf        c1, [c1 | c2]
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  sne          c5, k.l4_metadata_tcp_data_len, r0 // tcp_data_len != 0
  sne          c6, r6, r0 // tcp_rcvr_win_sz != 0
  scwle        c7, r5, r4 // tcp_seq_num_hi <= rflow_tcp_ack_num + rcvr_win_sz
  setcf        c1, [c1 & c2 & c3 & c6]
  seq          c2, k.tcp_seqNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c2, [c5 & c2 & c7]
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1 // rflow_tcp_ack_num - 1
  scwle        c3, r1, k.tcp_seqNo   // rflow_tcp_ack_num -1 <= tcp.seqNo
  scwlt        c4, k.tcp_seqNo, r4
  setcf        c3, [!c5 & c3 & c4]
  setcf        c2, [c2 | c3]
  bcf          ![c1 & c2], lb_tcp_session_state_initiator_non_best
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_win_sz, k.tcp_window
#if 0 /* RTT_NOT_CONSIDERED */
  phvwr        p.tcp_ackNo, r7
  scwlt.e      c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
#else /* RTT_NOT_CONSIDERED */
  seq          c2, d.u.tcp_session_state_info_d.flow_rtt_seq_check_enabled, 1
  bal.c2       r3, f_tcp_session_initiator_rtt_calculate
  phvwr        p.tcp_ackNo, r7
  scwlt.e      c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
#endif /* RTT_NOT_CONSIDERED */

lb_tcp_session_state_initiator_non_best:
  add          r2, r0, r0  // zeroing the r2 register to save exceptions seen
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, 0
  add.!c1      r5, r5, 1
  bcf          ![c5 & c6], lb_tss_i_1
  seq          c1, k.tcp_seqNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  bcf          [c1 & c7], lb_tss_i_tcp_state_transition
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_seqNo
  setcf        c1, [c1 & c7]
  b.c1         lb_tss_i_tcp_state_transition
  ori.c1       r2, r2, TCP_PACKET_REORDER
  scwlt        c1, k.tcp_seqNo, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  scwle        c2, r5, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_i_tcp_session_update
  ori.c1       r2, r2, TCP_FULL_REXMIT
  scwlt        c1, k.tcp_seqNo, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  setcf        c1, [c1 & c7]
  b.c1         lb_tss_i_tcp_state_transition
  ori.c1       r2, r2, TCP_PARTIAL_OVERLAP
  phvwr        p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  phvwr        p.capri_intrinsic_drop, 1
  b            lb_tss_i_exit
  ori          r2, r2, TCP_OUT_OF_WINDOW

lb_tss_i_1:
  bcf          ![!c5 & c6], lb_tss_i_2
  // SYN Retransmit
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
  add          r1, k.tcp_seqNo, 1
  seq          c3, r1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2 & c3]
  b.c1         lb_tss_i_exit
  ori.c1       r2, r2, TCP_SYN_REXMIT
  sle          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  b.c1         lb_tss_i_tcp_state_transition
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1
  scwle        c1, r1, k.tcp_seqNo
  scwlt        c2, k.tcp_seqNo, r4
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_i_tcp_state_transition
  ori.!c1      r2, r2, TCP_OUT_OF_WINDOW
  phvwr.!c1    p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  b            lb_tss_i_exit
  phvwr.!c1    p.capri_intrinsic_drop, 1

lb_tss_i_2:
  setcf        c1, [c5 & !c6]
  b.!c1        lb_tss_i_3
  ori.c1       r2, r2, TCP_WIN_ZERO_DROP
  phvwr        p.control_metadata_drop_reason[DROP_TCP_WIN_ZERO_DROP], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1

lb_tss_i_3:
  // Only case we will be here is if tcp_data_len == 0 and tcp_rcvr_win_sz == 0
  seq          c1, k.tcp_seqNo, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1
  seq          c2, k.tcp_seqNo, r1
  setcf        c1, [c1 | c2]
  b.c1         lb_tss_i_tcp_state_transition
  ori.!c1      r2, r2, TCP_OUT_OF_WINDOW
  phvwr.!c1    p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  b            lb_tss_i_exit
  phvwr.!c1    p.capri_intrinsic_drop, 1
 

// Our goal below should be to go to per iflow_tcp_state based switch case after
// executing the least number of instructions. So we will fill delay slots
// for branch cases with the instruction from next if check assuming we get a
// good tcp state transition most of the times, like the 3 way setup handshake
// and 4 way close handshake.
lb_tss_i_tcp_state_transition:
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST, 0
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  bcf          [c1 & c2], lb_tss_i_tcp_session_update

  smeqb        c1, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  b.!c1        lb_tss_i_4
  // Delay slot Instruction for lb_tss_i_4
  sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.iflow_tcp_state
  sne          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  b            lb_tss_i_exit
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_RESET

lb_tss_i_4:
  // Below instruction is coming from where we branching to this label (delay slot)
  // sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.iflow_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          ![c1 & c2], lb_tss_i_5
  // Delay slot Instruction for lb_tss_i_5
  slt          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  ori          r2, r2, TCP_UNEXPECTED_SYN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_SYN], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1


// Packets with FIN before we are in established state.
// TBD
lb_tss_i_5:
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  bcf          ![c1 & c2], lb_tss_i_6
  add          r1, r0, d.u.tcp_session_state_info_d.iflow_tcp_state
  nop.e
  nop


lb_tss_i_6:
  // switch case based on the iflow_tcp_state.
  .brbegin
  br           r1[2:0]
  nop
  .brcase      FLOW_STATE_INIT
  b            lb_tss_i_exit
  nop
  .brcase      FLOW_STATE_TCP_SYN_RCVD
  seq          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  smeqb        c2, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  seq          c3, k.tcp_ackNo, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  setcf        c1, [c1 & c2 & c3]
  b.!c1        lb_tss_i_syn_rcvd_1
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_win_sz, k.tcp_window
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  b            lb_tss_i_exit     
  phvwr        p.tcp_ackNo, r7

lb_tss_i_syn_rcvd_1:
  seq          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  setcf        c1, [c1 & c2 &c3]
  b.!c1        lb_tss_i_exit;
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_win_sz, k.tcp_window
  b            lb_tss_i_exit     
  phvwr        p.tcp_ackNo, r7

  .brcase      FLOW_STATE_TCP_ACK_RCVD
  b            lb_tss_i_exit
  nop
  .brcase      FLOW_STATE_TCP_SYN_ACK_RCVD
  b            lb_tss_i_exit
  nop
  .brcase      FLOW_STATE_ESTABLISHED
  smeqb        c1, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  sne          c2, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_FIN_RCVD
  setcf        c3, [c1 & c2]
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_FIN_RCVD
  b.c3         lb_tss_i_tcp_session_update
  phvwr.c3     p.capri_intrinsic_lif, CPU_VFID
 
  setcf        c3, [c1 & !c2]
  b.c3         lb_tss_i_tcp_session_update
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  
  b            lb_tss_i_exit
  phvwr.c3     p.capri_intrinsic_lif, CPU_VFID

  .brcase      FLOW_STATE_FIN_RCVD
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1

  .brcase      FLOW_STATE_BIDIR_FIN_RCVD
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  b.!c1        lb_tss_i_tcp_session_update
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1
  .brcase      FLOW_STATE_RESET
  ori          r2, r2, TCP_NON_RST_PKT_AFTER_RST
  phvwr        p.control_metadata_drop_reason[DROP_TCP_NON_RST_PKT_AFTER_RST], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1
  .brend

lb_tss_i_tcp_session_update:
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_win_sz, k.tcp_window
  phvwr        p.tcp_ackNo, r7
  scwlt       c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5

lb_tss_i_exit:
  phvwrm.e     p.l4_metadata_exceptions_seen, r2, 0xffff
  tblor        d.u.tcp_session_state_info_d.iflow_exceptions_seen, r2


f_tcp_session_initiator_rtt_calculate:
  jr           r3
  nop

lb_tcp_session_state_responder:
  nop.e
  nop

f_tcp_session_normalization:
  jr           r3
  nop



#if 0
lb_seq_flow_role_initator:
  add          r7, d.u.tcp_session_state_info_d.rflow_tcp_win_scale, r0
  sllv         r1, d.u.tcp_session_state_info_d.rflow_tcp_win_sz, r7   // r1 = rcvr_win_sz
  add          r2, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, r0   // r2 = expected_seq_num
  add          r3, k.tcp_seqNo, r0           // r3 = adjusted_seq_num
  add          r4, r3, k.l4_metadata_tcp_data_len                // r4 = seq_num_hi
  add          r5, k.tcp_ackNo, d.u.tcp_session_state_info_d.syn_cookie_delta // r5 = adjusted_ack_num
  add          r6, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r0   // r6 = b2b_iflow_tcp_seq_num
  b            lb_good_seq_check_start
  nop

lb_seq_flow_role_responder:
  add          r7, d.u.tcp_session_state_info_d.iflow_tcp_win_scale, r0
  sllv         r1, d.u.tcp_session_state_info_d.iflow_tcp_win_sz, r7
  add          r2, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, r0
  add          r3, k.tcp_seqNo, d.u.tcp_session_state_info_d.syn_cookie_delta
  add          r4, r3, k.l4_metadata_tcp_data_len
  add          r5, k.tcp_ackNo, r0
  add          r6, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r0   // r6 = b2b_rflow_tcp_seq_num
  b            lb_good_seq_check_start
  nop


// best packet case which will go through the session state stage with least number of instructions
// 1. Both sides in established state.
// 2. Packet has only ACK or ACK+PUSH flags only.
// 3. Receiver Windlow size is not zero
// 4a. Data len is non zero and
//       1. New seq number is aligned exactly to left of window and full packet is within window.
//       2. Data is to right of window but in window. When Back to back packets
// 4b. Data is zero and new seq number is within window (to cover ACK only packets)
//     are sent, this is common case

//
//    if (iflow_tcp_state == FLOW_STATE_ESTABLISHED and
//        rflow_tcp_state == FLOW_STATE_ESTABLISHED and
//        (tcp.flags == TCP_FLAG_ACK or
//         tcp.flags == (TCP_FLAG_ACK|TCP_FLAG_PSH)) and
//        (l4_metadata.tcp_rcvr_win_sz != 0) and
//        (((l4_metadata.tcp_data_len != 0) and
//          (scratch_metadata.adjusted_seq_num == scratch_metadata.b2b_expected_seq_num) and
//          (scratch_metadata.tcp_seq_num_hi <= (scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))) or
//         ((l4_metadata.tcp_data_len == 0) and
//          ((scratch_metadata.expected_seq_num -1 <= scratch_metadata.adjusted_seq_num) and
//         (scratch_metadata.adjusted_seq_num < scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))))) {
//
lb_good_seq_check_start:
  seq          c2, k.tcp_flags, TCP_FLAG_ACK
  seq          c3, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
  setcf        c2, [c2 | c3]
  seq          c4, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  seq          c5, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  sne          c6, r1, r0                    // rcvr_win_sz != 0
  bcf          ![c2 & c4 & c5 & c6], lb_flow_state_slow_path_1
  sne          c2, k.l4_metadata_tcp_data_len, r0 // tcp_data_len != 0
  seq          c3, r3, r6                   // adjusted_seq_num == b2b_iflow_tcp_seq_num
  sscwle       c4, r4, r2, r1            // seq_num_hi <= expected_seq + rcvr_win_sz
  setcf        c5, [c2 & c3 & c4]
  subi         r7, r2, 1                   // expected_seq - 1
  scwle        c6, r7, r3                   // expected_seq -1 <= adjusted_seq_num
  sscwlt       c7, r3, r2, r1            // (adjusted_seq_num < expected_seq + rcvr_win_sz)
  setcf        c7, [!c2 & c6 & c7]        //
  bcf          ![c5 | c7], lb_flow_state_slow_path_1
  //nop
  scwlt        c3, r6, r4 // Candidate for NOP fill

// We can skip the rest of the processing for Seq Number and
// tcp state transition checks.
// Update the following in Flow State Data for any direction:
//  1. Ack Num
//  2. Adjusted tcp_seq_num_hi
//  3. New Window Size.
// Update the following in PHV:
//  Initator:
//    1. Adjusted Ack Num
//  Responder:
//    1. Adjusted Seq Num

// c1 still has the flow_role check.
// This assumes that
//  // slt c3, r6, r4  ==> saved seq_num < new seq_num_hi
f_update_seq_ack_window_no_tcp_state:
  bcf          ![c1], lb_update_seq_ack_window_no_tcp_state_responder
  scwlt        c3, r6, r4 // saved seq_num < new seq_num_hi
  scwlt        c2, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c2     d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c2     d.u.tcp_session_state_info_d.iflow_tcp_win_sz, k.tcp_window
  // slt c3, r6, r4 // Candidate for NOP fill
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r4
  seq.e        c4, d.u.tcp_session_state_info_d.syn_cookie_delta, r0
  phvwr.!c4    p.tcp_ackNo, r5

lb_update_seq_ack_window_no_tcp_state_responder:
  scwlt        c2, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c2     d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c2     d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window
  // slt c3, r6, r4  // Candidate for NOP fill
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r4
  seq.e        c4, d.u.tcp_session_state_info_d.syn_cookie_delta, r0
  phvwr.!c4    p.tcp_seqNo, r3

// Function end f_update_seq_ack_window_no_tcp_state


// lb_flow_state_slow_path_1:
// In this second best case of TCP packet handling we will check for
// TCP Seq Number checks for estalished connections only and identify
// - Packet with any flags.
// 1. TCP Full Retransmits
// 2. TCP Partial Overlap
// 3. TCP Packet reorder
// 4. TCP out of window - Dropped Packets
//
// r1 = rcvr_win_sz
// r2 = expected_seq_num
// r3 = adjusted_seq_num
// r4 = seq_num_hi
// r5 = adjusted_ack_num
// r6 = b2b_iflow_tcp_seq_num,

lb_flow_state_slow_path_1:
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  setcf        c2, [c2 & c1]
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  setcf        c3, [c3 & !c1]
  setcf        c2, [c2 | c3]
  sne          c4, r1, r0                    // rcvr_win_sz != 0
  sne          c5, k.l4_metadata_tcp_data_len, r0 // tcp_data_len != 0
  bcf          ![c2 & c4 & c5], lb_flow_state_slow_path_2

  // if ((scratch_metadata.adjusted_seq_num == scratch_metadata.b2b_expected_seq_num) and
  //     (scratch_metadata.tcp_seq_num_hi <= (scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))) {
  //      // Good Packet but with flags other than ACK & PSH.
  //      // JUMP TO TCP STATE TRANSITIONS
  // }
  seq          c2, r3, r6                   // adjusted_seq_num == b2b_iflow_tcp_seq_num
  sscwle       c3, r4, r2, r1               // tcp_seq_num_hi <= expected_seq_num + tcp_rcvr_win_sz
  bcf          [c2 & c3], f_tcp_session_state_transition;
  //
  //  if ((scratch_metadata.adjusted_seq_num < scratch_metadata.expected_seq_num) and
  //      (scratch_metadata.tcp_seq_num_hi < scratch_metadata.expected_seq_num)) {
  //      // full retransmit of packet we have seen before, still acceptable
  //      bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_FULL_REXMIT);
  //      // GOOD PACKET JUMP TO TCP STATE TRANSITIONS
  //  }
  scwlt        c2, r3, r2 // This is in NOP fill for above bcf.
  scwlt        c3, r4, r2
  bcf          [c2 & c3], lb_tcp_seq_full_rexmit;
  //
  //  if ((scratch_metadata.adjusted_seq_num < scratch_metadata.expected_seq_num) and
  //      (scratch_metadata.tcp_seq_num_hi <= (scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))) {
  //      // left side overlap, still acceptable
  //      bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_PARTIAL_OVERLAP);
  //      // GOOD PACKET JUMP TO TCP STATE TRANSITIONS
  //  }
  //  c2 = scratch_metadata.adjusted_seq_num < scratch_metadata.expected_seq_num
  sscwle       c3, r4, r2, r1  // This is a NOP fill for above bcf.
  bcf          [c2 & c3], lb_tcp_seq_partial_overlap;
  //
  //  if ((scratch_metadata.adjusted_seq_num >= scratch_metadata.expected_seq_num) and
  //      (scratch_metadata.tcp_seq_num_hi <= (scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))) {
  //      // non-overlapping within the window but not the next one we are
  //      // expecting, possible re-ordering of segments happens in between
  //      bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_PACKET_REORDER);
  //      // GOOD PACKET JUMP TO TCP STATE TRANSITIONS
  //  }
  // c3 = (scratch_metadata.tcp_seq_num_hi <= (scratch_metadata.expected_seq_num + l4_metadata.tcp_rcvr_win_sz))
  scwle        c2, r2, r3
  bcf          [c2 & c3], lb_tcp_seq_packet_reorder
  nop
lb_tcp_out_of_window:
  // Drop the out of window packet and exit the action routine.
  tblor.c1     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_OUT_OF_WINDOW
  tblor.!c1    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_OUT_OF_WINDOW
  phvwr.e      p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  phvwr        p.capri_intrinsic_drop, 1


// Set the exception seen and drop the packet.
lb_tcp_seq_full_rexmit:
  tblor.c1     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_FULL_REXMIT
  tblor.!c1    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_FULL_REXMIT
  b f_update_seq_ack_window_no_tcp_state;
  nop

// Set the exception seen and drop the packet.
lb_tcp_seq_win_zero_drop:
  tblor.c1     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_WIN_ZERO_DROP
  tblor.!c1    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_WIN_ZERO_DROP
  phvwr.e      p.control_metadata_drop_reason[DROP_TCP_WIN_ZERO_DROP], 1
  phvwr        p.capri_intrinsic_drop, 1

// Set the exception seen and continue packet processing
lb_tcp_seq_partial_overlap:
  tblor.c1     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_PARTIAL_OVERLAP
  b            f_tcp_session_state_transition;
  tblor.!c1    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_PARTIAL_OVERLAP

// Set the exception seen and continue packet processing
lb_tcp_seq_packet_reorder:
  tblor.c1     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_PACKET_REORDER
  b            f_tcp_session_state_transition;
  tblor.!c1    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_PACKET_REORDER

// Set the exception seen and exit packet processing
lb_tcp_seq_syn_retransmit:
  tblor.c1.e     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_SYN_REXMIT
  tblor.!c1.e    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_SYN_REXMIT
  nop

lb_tcp_ack_err:
  tblor.c1.e     d.u.tcp_session_state_info_d.iflow_exceptions_seen, TCP_ACK_ERR
  tblor.!c1.e    d.u.tcp_session_state_info_d.rflow_exceptions_seen, TCP_ACK_ERR
  phvwr.e        p.control_metadata_drop_reason[DROP_TCP_ACK_ERR], 1
  phvwr          p.capri_intrinsic_drop, 1
  nop

lb_tcp_state_established:
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  b            f_update_seq_ack_window_no_tcp_state;

lb_flow_state_slow_path_2:
// Here we will do the sequence number checks for most non-common cases and
// TCP State not being in Established state.

// if both segment length and receive window are zero, only packets that
// are allowed are empty ACK or RST or URG pkts
// if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
//     if ((scratch_metadata.expected_seq_num - 1 <= scratch_metadata.adjusted_seq_num) and
//         (scratch_metadata.adjusted_seq_num <= scratch_metadata.expected_seq_num)) {
//            // JUMP TO TCP STATE TRANSITIONS
//     } else {
//         bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
//         modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
//         drop_packet();
//     }
// }
  seq          c2, k.l4_metadata_tcp_data_len, r0 // tcp_data_len == 0
  seq          c3, r1, r0                         // tcp_rcvr_win_sz == 0
  subi         r7, r2, 1                          // expected_seq_num - 1
  // Only for seq we need to make sure that we check using 32 bits. Other
  // opcodes like scwlt already act on 32 bits only.
  // Updated after Mike & David Comment
  seq          c5, r7[31:0], r3                   // (expected_seq_num - 1 == adjusted_seq_num)
  seq          c6, r2, r3                         // expected_seq_num == adjusted_seq_num
  setcf        c7, [c5 | c6]
  bcf          [c2 & c3 & c7], f_tcp_session_state_transition;
  nop
  bcf          [c2 & c3 & !c7], lb_tcp_out_of_window;
  nop

//    // if segment lenght is > 0 but receiver window size is 0
//    // this is not actually a security breach, may be the sender
//    // didn't receive the window advertisement with sz 0 yet, simply
//    // retransmit, but increment a counter
//    if ((l4_metadata.tcp_data_len != 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
//        bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_WIN_ZERO_DROP);
//        modify_field(control_metadata.drop_reason, DROP_TCP_WIN_ZERO_DROP);
//        drop_packet();
//    }
//
  bcf          [!c2 & c3], lb_tcp_seq_win_zero_drop
  nop
//
//  TCP Syn Retransmit:
//    if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz > 0) and
//        (scratch_metadata.expected_seq_num == 0) and
//        (flow_info_metadata.flow_role == TCP_FLOW_INITIATOR) and
//        (scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_SENT) and
//        (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) and
//        (scratch_metadata.adjusted_seq_num + 1 == scratch_metadata.iflow_tcp_seq_num)) {
//        // TRACE_DBG("SYN rexmit detected\n");
//        bit_or(flow_state_metadata.exceptions_seen, flow_state_metadata.exceptions_seen, TCP_SYN_REXMIT);
//    }
  setcf        c7, [c1 & c2 & !c3 & c4]
  seq          c6, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  smeqb        c5, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
  addi         r7, r3, 1                          // adjusted_seq_num + 1
  seq          c4, r7, r6                         // adjsuted_seq_num + 1 == b2b_tcp_seq_num
  bcf          [c4 & c5 & c6 & c7], lb_tcp_seq_syn_retransmit;
  nop

// ACK # verification for SYN + ACK
  smeqb        c2, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_SYN, TCP_FLAG_ACK|TCP_FLAG_SYN
  seq          c3, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  sne          c4, k.tcp_ackNo, r6
  bcf          [!c1 & c2 & c3 & c4], lb_tcp_ack_err;

// Continue with other checks related to sequence number here.

f_tcp_session_state_transition:
  // Here we will check for the TCP Flags and transition the TCP States.
  // Packet from Initiator
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  smeqb        c4, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  bcf          [c1 & c2 & c3 & c4], lb_tcp_state_established;
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  smeqb        c4, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_SYN, TCP_FLAG_ACK|TCP_FLAG_SYN
  smeqb        c5, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  setcf        c4, [c4 | c5]
  setcf        c6, [c1 & c2 & c3 & c4]
  tblwr.c6     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  b.c6         f_update_seq_ack_window_no_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  setcf        c3, [c1 | c2]
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_FIN_RCVD
  b.c3         f_update_seq_ack_window_no_tcp_state
  // Packet from Responder
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  smeqb        c4, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  bcf          [!c1 & c2 & c3 & c4], lb_tcp_state_established;
  seq          c2, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_INIT
  smeqb        c3, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_SYN, TCP_FLAG_ACK|TCP_FLAG_SYN
  setcf        c4, [!c1 & c2 & c3]
  tblwr.c4     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  b.c4         f_update_seq_ack_window_no_tcp_state
  smeqb        c3, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_SYN, TCP_FLAG_ACK
  setcf        c4, [!c1 & c2 & c3]
  tblwr.c4     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_ACK_RCVD
  b.c4         f_update_seq_ack_window_no_tcp_state
  smeqb        c3, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_SYN, TCP_FLAG_SYN
  setcf        c4, [!c1 & c2 & c3]
  tblwr.c4     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  b.c4         f_update_seq_ack_window_no_tcp_state
  seq          c2, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_ACK_RCVD
  setcf        c4, [!c1 & c2 & c3]
  tblwr.c4     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  b.c4         f_update_seq_ack_window_no_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_FIN, TCP_FLAG_FIN
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_FIN_RCVD
  b.c3         f_update_seq_ack_window_no_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_ACK|TCP_FLAG_RST, TCP_FLAG_RST
  tblwr.c2     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  tblwr.c2     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_RESET
 
  // We should update the state before branching to update the rest of the state.
  b f_update_seq_ack_window_no_tcp_state;
  scwlt        c3, r6, r4 // Candidate for NOP fill, Prerequisite to jump to above label
  nop



#endif /* 0 */
