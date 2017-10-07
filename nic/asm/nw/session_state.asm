#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct session_state_k k;
struct session_state_d d;
struct phv_               p;

%%

nop:
  nop.e
  nop

// Code is written to match the P4 program as much as possible to
// facilitate changes to assemmbly easily. Once the P4 Program is
// final and don't anticipate any changes we can optimize the
// assembly code. If we optimize the assembly code right now
// then even a single change in P4 program can invalidate the
// complete assembly code.

// Some computed values are saved in registers and maintained throughtout the
// the program so that we can reduce the overall # of instructions
// R1 - Temporary use
// R2 - Exceptions seen for the packet
// R3 - Used for bal instruction to save the return address. No bal in bal usage.
// R4 - tcp_mss before calling normalization. After that rflow_tcp_ack_num + tcp_rcvr_win_sz
// R5 - tcp_seq_num_hi
// R6 - tcp_rcvr_win_sz
// R7 - adjusted ack or seq num (taking into account the syn_cookie_delta)
//
// C5 - tcp_data_len != 0
// C6 - tcp_rcvr_win_sz != 0
// C7 - tcp_seq_num_hi <= rflow_tcp_ack_num + tco_rcvr_win_sz

// Pick the initator or responder values to use.
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tcp_session_state_info:
  seq          c1, k.flow_info_metadata_flow_role, TCP_FLOW_INITIATOR
  b.!c1        lb_tcp_session_state_responder
  seq          c1, k.l4_metadata_tcp_normalization_en, 1

lb_tcp_session_state_initiator:
  add          r1, d.u.tcp_session_state_info_d.rflow_tcp_win_scale, r0
  sllv         r6, d.u.tcp_session_state_info_d.rflow_tcp_win_sz, r1   // r6 = rcvr_win_sz
  add          r4, d.u.tcp_session_state_info_d.rflow_tcp_mss, r0
  bal.c1       r3, f_tcp_session_normalization
  add          r7, k.tcp_ackNo, d.u.tcp_session_state_info_d.syn_cookie_delta // r7 = adjusted_ack_num
  add          r5, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, k.l4_metadata_tcp_data_len  // tcp_seq_num_hi
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
  seq          c2, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c2, [c5 & c2 & c7]
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1 // rflow_tcp_ack_num - 1
  scwle        c3, r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}   // rflow_tcp_ack_num -1 <= tcp.seqNo
  scwlt        c4, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, r4
  setcf        c3, [!c5 & c3 & c4]
  setcf        c2, [c2 | c3]
  bcf          ![c1 & c2], lb_tcp_session_state_initiator_non_best
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  scwle        c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  setcf        c1, [c1 & c2]
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
  seq          c1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  bcf          [c1 & c7], lb_tss_i_tcp_state_transition
  scwle        c1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}
  setcf        c1, [c1 & c7]
  b.c1         lb_tss_i_tcp_state_transition
  ori.c1       r2, r2, TCP_PACKET_REORDER
  scwlt        c1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  scwle        c2, r5, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_i_tcp_session_update
  ori.c1       r2, r2, TCP_FULL_REXMIT
  scwlt        c1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
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
  add          r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, 1
  seq          c3, r1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2 & c3]
  b.c1         lb_tss_i_exit
  ori.c1       r2, r2, TCP_SYN_REXMIT
  sle          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  b.c1         lb_tss_i_tcp_state_transition
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1
  scwle        c1, r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}
  scwlt        c2, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, r4
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
  seq          c1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1
  seq          c2, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, r1
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
  //phvwr.c3     p.control_metadata_dst_lport, CPU_LPORT
  nop // replace when above line is uncommented. FTE needs to FIN handling

  setcf        c3, [c1 & !c2]
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  b.c3         lb_tss_i_tcp_session_update
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD

  b            lb_tss_i_exit
  //phvwr.c3     p.control_metadata_dst_lport, CPU_LPORT
  nop // replace when above line is uncommented. FTE needs to FIN handling

  .brcase      FLOW_STATE_FIN_RCVD
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  b.!c1        lb_tss_i_tcp_session_update
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
  scwle        c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  setcf        c1, [c1 & c2]
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
  add          r1, d.u.tcp_session_state_info_d.iflow_tcp_win_scale, r0
  sllv         r6, d.u.tcp_session_state_info_d.iflow_tcp_win_sz, r1   // r6 = rcvr_win_sz
  add          r4, d.u.tcp_session_state_info_d.iflow_tcp_mss, r0
  bal.c1       r3, f_tcp_session_normalization
  sub          r7, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.syn_cookie_delta // r7 = adjusted_seq_num
  add          r5, r7, k.l4_metadata_tcp_data_len  // tcp_seq_num_hi
  add          r4, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, r6 // iflow_tcp_ack_num + rcvr_win_sz
  seq          c1, k.tcp_flags, TCP_FLAG_ACK
  seq          c2, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
  setcf        c1, [c1 | c2]
  seq          c2, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  seq          c3, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  sne          c5, k.l4_metadata_tcp_data_len, r0 // tcp_data_len != 0
  sne          c6, r6, r0 // tcp_rcvr_win_sz != 0
  scwle        c7, r5, r4 // tcp_seq_num_hi <= iflow_tcp_ack_num + rcvr_win_sz
  setcf        c1, [c1 & c2 & c3 & c6]
  seq          c2, r7, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  setcf        c2, [c5 & c2 & c7]
  sub          r1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, 1 // iflow_tcp_ack_num - 1
  scwle        c3, r1, r7   // iflow_tcp_ack_num -1 <= adjusted_seq_num
  scwlt        c4, r7, r4
  setcf        c3, [!c5 & c3 & c4]
  setcf        c2, [c2 | c3]
  bcf          ![c1 & c2], lb_tcp_session_state_responder_non_best
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  scwle        c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window
#if 0 /* RTT_NOT_CONSIDERED */
  phvwr        p.tcp_seqNo, r7
  scwlt.e      c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
#else /* RTT_NOT_CONSIDERED */
  seq          c2, d.u.tcp_session_state_info_d.flow_rtt_seq_check_enabled, 1
  bal.c2       r3, f_tcp_session_initiator_rtt_calculate
  phvwr        p.tcp_seqNo, r7
  scwlt.e      c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
#endif /* RTT_NOT_CONSIDERED */

lb_tcp_session_state_responder_non_best:
  add          r2, r0, r0  // zeroing the r2 register to save exceptions seen
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN, 0
  add.!c1      r5, r5, 1
  bcf          ![c5 & c6], lb_tss_r_1
  seq          c1, r7, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  bcf          [c1 & c7], lb_tss_r_tcp_state_transition
  scwle        c1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, r7
  setcf        c1, [c1 & c7]
  b.c1         lb_tss_r_tcp_state_transition
  ori.c1       r2, r2, TCP_PACKET_REORDER
  scwlt        c1, r7, d.u.tcp_session_state_info_d.iflow_tcp_ack_num
  scwle        c2, r5, d.u.tcp_session_state_info_d.iflow_tcp_ack_num
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_r_tcp_session_update
  ori.c1       r2, r2, TCP_FULL_REXMIT
  scwlt        c1, r7, d.u.tcp_session_state_info_d.iflow_tcp_ack_num
  setcf        c1, [c1 & c7]
  b.c1         lb_tss_r_tcp_state_transition
  ori.c1       r2, r2, TCP_PARTIAL_OVERLAP
  phvwr        p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  phvwr        p.capri_intrinsic_drop, 1
  b            lb_tss_r_exit
  ori          r2, r2, TCP_OUT_OF_WINDOW

lb_tss_r_1:
  bcf          ![!c5 & c6], lb_tss_r_2
  slt          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  slt          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  smeqb        c3, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN|TCP_FLAG_ACK
  seq          c4, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  bcf          [c1 & c2 & c3 & c4], lb_tss_r_tcp_state_transition
  smeqb        c3, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  bcf          [c1 & c2 & c3 & c4], lb_tss_r_tcp_state_transition
  smeqb        c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          [c1 & c2 & c3 & c4], lb_tss_r_tcp_state_transition
  sub          r1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, 1
  scwle        c1, r1, r7
  scwlt        c2, r7, r4
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_r_tcp_state_transition
  ori.!c1      r2, r2, TCP_OUT_OF_WINDOW
  phvwr.!c1    p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  b            lb_tss_r_exit
  phvwr.!c1    p.capri_intrinsic_drop, 1

lb_tss_r_2:
  setcf        c1, [c5 & !c6]
  b.!c1        lb_tss_r_3
  ori.c1       r2, r2, TCP_WIN_ZERO_DROP
  phvwr        p.control_metadata_drop_reason[DROP_TCP_WIN_ZERO_DROP], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

lb_tss_r_3:
  // Only case we will be here is if tcp_data_len == 0 and tcp_rcvr_win_sz == 0
  seq          c1, r7, d.u.tcp_session_state_info_d.iflow_tcp_ack_num
  sub          r1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, 1
  seq          c2, r7, r1
  setcf        c1, [c1 | c2]
  b.c1         lb_tss_r_tcp_state_transition
  ori.!c1      r2, r2, TCP_OUT_OF_WINDOW
  phvwr.!c1    p.control_metadata_drop_reason[DROP_TCP_OUT_OF_WINDOW], 1
  b            lb_tss_r_exit
  phvwr.!c1    p.capri_intrinsic_drop, 1


// Our goal below should be to go to per iflow_tcp_state based switch case after
// executing the least number of instructions. So we will fill delay slots
// for branch cases with the instruction from next if check assuming we get a
// good tcp state transition most of the times, like the 3 way setup handshake
// and 4 way close handshake.
lb_tss_r_tcp_state_transition:
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST, 0
  seq          c2, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  bcf          [c1 & c2], lb_tss_r_tcp_session_update

  smeqb        c1, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  b.!c1        lb_tss_r_4
  // Delay slot Instruction for lb_tss_r_4
  sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.rflow_tcp_state
  sne          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  b            lb_tss_r_exit
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_RESET

lb_tss_r_4:
  // Below instruction is coming from where we branching to this label (delay slot)
  // sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.rflow_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          ![c1 & c2], lb_tss_r_5
  // Delay slot Instruction for lb_tss_r_5
  slt          c1, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  ori          r2, r2, TCP_UNEXPECTED_SYN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_SYN], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

// Packets with FIN before we are in established state.
// TBD
lb_tss_r_5:
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  bcf          ![c1 & c2], lb_tss_r_6
  add          r1, r0, d.u.tcp_session_state_info_d.rflow_tcp_state
  nop.e
  nop

lb_tss_r_6:
  // switch case based on the rflow_tcp_state.
  .brbegin
  br           r1[2:0]
  nop
  .brcase      FLOW_STATE_INIT
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN|TCP_FLAG_ACK
  seq          c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2]
  b.!c1        lb_tss_r_init_1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  seq          c2, k.tcp_option_ws_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq          c2, k.tcp_option_timestamp_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  b            lb_tss_r_exit
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window

lb_tss_r_init_1:
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_ACK
  seq          c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2]
  b.!c1        lb_tss_r_init_2
  ori.c1       r2, r2, TCP_SPLIT_HANDSHAKE_DETECTED
  seq          c1, k.l4_metadata_tcp_split_handshake_drop, ACT_DROP
  phvwr.c1     p.control_metadata_drop_reason[DROP_TCP_SPLIT_HANDSHAKE], 1
  phvwr.c1     p.capri_intrinsic_drop, 1

  tblwr.!c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_ACK_RCVD
  tblwr.!c1     d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  b            lb_tss_r_exit
  tblwr.!c1     d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window

lb_tss_r_init_2:
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
  seq          c2, k.tcp_ackNo, 0
  setcf        c1, [c1 & c2]
  b.!c1        lb_tss_r_init_3
  ori.c1       r2, r2, TCP_SPLIT_HANDSHAKE_DETECTED
  seq          c1, k.l4_metadata_tcp_split_handshake_drop, ACT_DROP
  b.!c1        lb_tss_r_init_2_1
  phvwr.c1     p.control_metadata_drop_reason[DROP_TCP_SPLIT_HANDSHAKE], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

lb_tss_r_init_2_1:
  // Simultaneous open allowed.
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD

  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  seq          c2, k.tcp_option_ws_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq          c2, k.tcp_option_timestamp_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  b            lb_tss_r_exit
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5

lb_tss_r_init_3:
  ori          r2, r2, TCP_INVALID_RESPONDER_FIRST_PKT
  phvwr        p.control_metadata_drop_reason[DROP_TCP_INVALID_RESPONDER_FIRST_PKT], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1


  .brcase      FLOW_STATE_TCP_SYN_RCVD
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  smeqb        c2, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  seq          c3, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2 & c3]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  b.c1         lb_tss_r_exit
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  setcf        c1, [c1 & c2 & c3]
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  b.c1         lb_tss_r_tcp_session_update
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  b            lb_tss_r_exit
  nop

  .brcase      FLOW_STATE_TCP_ACK_RCVD
  smeqb        c1, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  b.!c1        lb_tss_r_exit
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  seq          c2, k.tcp_option_ws_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq          c2, k.tcp_option_timestamp_valid, 1
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  b            lb_tss_r_exit
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5

  .brcase      FLOW_STATE_TCP_SYN_ACK_RCVD
  b            lb_tss_r_exit
  nop

  .brcase      FLOW_STATE_ESTABLISHED
  smeqb        c1, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  sne          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_FIN_RCVD
  setcf        c3, [c1 & c2]
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_FIN_RCVD
  b.c3         lb_tss_r_tcp_session_update
  //phvwr.c3     p.control_metadata_dst_lport, CPU_LPORT
  nop // replace when above line is uncommented. FTE needs to FIN handling

  setcf        c3, [c1 & !c2]
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  b.c3         lb_tss_r_tcp_session_update
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD

  b            lb_tss_r_exit
  //phvwr.c3     p.control_metadata_dst_lport, CPU_LPORT
  nop // replace when above line is uncommented. FTE needs to FIN handling

  .brcase      FLOW_STATE_FIN_RCVD
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  b.!c1        lb_tss_r_tcp_session_update
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

  .brcase      FLOW_STATE_BIDIR_FIN_RCVD
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  b.!c1        lb_tss_r_tcp_session_update
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

  .brcase      FLOW_STATE_RESET
  ori          r2, r2, TCP_NON_RST_PKT_AFTER_RST
  phvwr        p.control_metadata_drop_reason[DROP_TCP_NON_RST_PKT_AFTER_RST], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1
  .brend

lb_tss_r_tcp_session_update:
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  scwle        c2, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c1, [c1 & c2]
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_sz, k.tcp_window
  phvwr        p.tcp_seqNo, r7
  scwlt       c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5

lb_tss_r_exit:
  phvwrm.e     p.l4_metadata_exceptions_seen, r2, 0xffff
  tblor        d.u.tcp_session_state_info_d.rflow_exceptions_seen, r2

// R4 - tcp_mss before calling normalization. After that rflow_tcp_ack_num + tcp_rcvr_win_sz
// R6 - tcp_rcvr_win_sz
f_tcp_session_normalization:
  seq          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_ESTABLISHED
  seq          c3, d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_ESTABLISHED
  // TBD: jrcf
  setcf        c2, [c2 & c3]
  jr.!c2       r3
  seq          c2, k.l4_metadata_tcp_data_len_gt_mss_action, \
                      NORMALIZATION_ACTION_ALLOW
  b.c2         lb_tcp_data_len_gt_win_size
  seq          c2, k.l4_metadata_tcp_data_len_gt_win_size_action, NORMALIZATION_ACTION_ALLOW
  slt          c3, r4, k.l4_metadata_tcp_data_len
  b.!c3        lb_tcp_data_len_gt_win_size
  seq          c3, k.l4_metadata_tcp_data_len_gt_mss_action, \
                      NORMALIZATION_ACTION_DROP
  phvwr.c3.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3     p.capri_intrinsic_drop, 1
  // Edit option
  // 1. Change the l4_metadata.tcp_data_len to mss
  // 2. Edit the IP Total len based on whether tunnel is terminated or not
  // 3. Update the packet_len which is used by deparser to reconstruct
  //    the packet.




lb_tcp_data_len_gt_win_size:
  b.c2         lb_tcp_unexpected_ts_option
  seq          c2, k.l4_metadata_tcp_unexpected_ts_option_action, \
                      NORMALIZATION_ACTION_ALLOW
  slt          c3, r6, k.l4_metadata_tcp_data_len
  b.!c3        lb_tcp_unexpected_ts_option
  seq          c3, k.l4_metadata_tcp_data_len_gt_win_size_action, NORMALIZATION_ACTION_DROP
  phvwr.c3.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3     p.capri_intrinsic_drop, 1
  // Edit option
  // 1. Change the l4_metadata.tcp_data_len to rcvr_win_sz
  // 2. Edit the IP Total len based on whether tunnel is terminated or not
  // 3. Update the packet_len which is used by deparser to reconstruct
  //    the packet.


lb_tcp_unexpected_ts_option:
  b.c2         lb_tcp_ts_not_present
  seq          c2, k.l4_metadata_tcp_ts_not_present_drop, ACT_ALLOW
  seq          c3, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, FALSE
  seq          c4, k.tcp_option_timestamp_valid, TRUE
  bcf          ![c3 & c4], lb_tcp_ts_not_present
  seq          c3, k.l4_metadata_tcp_unexpected_ts_option_action, \
                      NORMALIZATION_ACTION_DROP
  phvwr.c3.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3     p.capri_intrinsic_drop, 1
  // Edit option
  phvwr        p.tcp_option_timestamp_optType, 0x1
  phvwr        p.tcp_option_timestamp_optLength, 0x1
  addi         r1, r0, 0x01010101
  phvwr        p.tcp_option_timestamp_prev_echo_ts, r1
  phvwr        p.tcp_option_timestamp_ts, r1

lb_tcp_ts_not_present:
  jr.c2        r3
  seq          c3, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, TRUE
  seq          c4, k.tcp_option_timestamp_valid, FALSE
  // TBD - jrcf
  setcf        c3, [c3 & c4]
  jr.!c3       r3
  nop
  phvwr.c3.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c3     p.capri_intrinsic_drop, 1
