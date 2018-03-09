#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct session_state_k k;
struct session_state_d d;
struct phv_               p;

%%

nop:
  nop.e
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_state_error:
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
// R2 - tcp_data_len before calling normalization, after that Exceptions seen for the packet
// R3 - Used for bal instruction to save the return address. No bal in bal usage.
// R4 - tcp_mss before calling normalization(not used now). After that rflow_tcp_ack_num + tcp_rcvr_win_sz
// R5 - used in normalization for scratch after it returns its loaded with tcp_seq_num_hi
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
  // New Instruction TBD
  bbeq         k.flow_info_metadata_flow_role, TCP_FLOW_RESPONDER, lb_tcp_session_state_responder
  seq          c1, k.l4_metadata_tcp_normalization_en, 1

lb_tcp_session_state_initiator:
  // New Instruction TBD,
  sll          r6, d.u.tcp_session_state_info_d.rflow_tcp_win_sz, d.u.tcp_session_state_info_d.rflow_tcp_win_scale // r6 = rcvr_win_sz
  add          r2, k.l4_metadata_tcp_data_len, r0 // r2 can  be modified by lb_tcp_session_initator_normalization
  seq          c2, d.{u.tcp_session_state_info_d.iflow_tcp_state,u.tcp_session_state_info_d.rflow_tcp_state}, (FLOW_STATE_ESTABLISHED << 4 | FLOW_STATE_ESTABLISHED)
  // Normalization checks
  slt          c3, d.u.tcp_session_state_info_d.rflow_tcp_mss, r2 // tcp_mss < tcp_data_len
  slt.!c3      c3, r6, r2 // tcp_rcvr_win_sz < tcp_data_len
  xor          r1, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, k.tcp_option_timestamp_valid
  seq.!c3      c3, r1, 1
  // One more normalizaiton check, if any of the sack_valid bit is set we need to make sure
  // that sack_perm_option_neogitated is TRUE, otherwise we have to enter normalization code
  seq          c4, d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, FALSE
  sne.c4       c4, k.{tcp_option_four_sack_valid...tcp_option_one_sack_valid}, r0
  setcf        c3, [c3 | c4]
  // All normalizaiton checks are in c3. c3 = TRUE (needs normaliaiton, otherwise no normalization)
  // c1 = normalizatio en, c2 = established connection, c3 = bad condition hit
  bcf          [c1 & c2 & c3], lb_tcp_session_initator_normalization
  add          r7, k.tcp_ackNo, d.u.tcp_session_state_info_d.syn_cookie_delta // r7 = adjusted_ack_num
lb_initator_normlizaiton_return:
  add          r5, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, r2  // tcp_seq_num_hi
  add          r4, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, r6 // rflow_tcp_ack_num + rcvr_win_sz
  seq          c1, k.tcp_flags, TCP_FLAG_ACK
  seq.!c1      c1, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
  sne          c5, r2, r0 // tcp_data_len != 0
  sne          c6, r6, r0 // tcp_rcvr_win_sz != 0
  scwle        c7, r5, r4 // tcp_seq_num_hi <= rflow_tcp_ack_num + rcvr_win_sz (optimized)

  setcf        c1, [c1 & c2 & c6]  // c1 = (ACK or ACK|PSH), c2 = Both Established,
  seq          c2, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
  setcf        c2, [c5 & c2 & c7]
  sub          r1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, 1 // rflow_tcp_ack_num - 1
  scwle        c3, r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}   // rflow_tcp_ack_num -1 <= tcp.seqNo
  scwlt        c4, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, r4
  setcf.!c2    c2, [!c5 & c3 & c4]
  bcf          ![c1 & c2], lb_tcp_session_state_initiator_non_best
  scwlt        c1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, k.tcp_ackNo
  scwle.c1     c1, k.tcp_ackNo, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
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
  // Since we don't want o change rest of the code, doing what we are doing earlier in
  // good packet case back here.
  // R5 OPTIM
  sub          r5, r5, 1	
 
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
  scwlt        c2, r5, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
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
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_INIT
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_SYN
  setcf        c1, [c1 & c2]
  b.c1         lb_tss_i_exit
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  // SYN Retransmit
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
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
  phvwr.c1     p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_RST_COPY
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  b            lb_tss_i_exit
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_RESET

lb_tss_i_4:
  // Below instruction is coming from where we branching to this label (delay slot)
  // sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.iflow_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          ![c1 & c2], lb_tss_i_5
  // Delay slot Instruction for lb_tss_i_5
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  ori          r2, r2, TCP_UNEXPECTED_PKT
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_PKT], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1


// Packets with FIN before we are in established state.
// TBD
lb_tss_i_5:
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  bcf          ![!c1 & c2], lb_tss_i_6
  add          r1, r0, d.u.tcp_session_state_info_d.iflow_tcp_state
  // We got a FIN in pre-established state.
  ori          r2, r2, TCP_UNEXPECTED_PKT
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_PKT], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1


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
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  tblwr        d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r1
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
  phvwr.c3     p.capri_intrinsic_tm_replicate_en, 1
  b.c3         lb_tss_i_tcp_session_update
  phvwr.c3     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY
  setcf        c3, [c1 & !c2]
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  phvwr.c3     p.capri_intrinsic_tm_replicate_en, 1
  b.c3         lb_tss_i_tcp_session_update
  phvwr.c3     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY
  b            lb_tss_i_exit
  nop

  .brcase      FLOW_STATE_FIN_RCVD
  scwle        c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  b.!c1        lb_tss_i_tcp_session_update
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_i_exit
  phvwr        p.capri_intrinsic_drop, 1

  .brcase      FLOW_STATE_BIDIR_FIN_RCVD
  scwle        c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
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
  scwle        c1, d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r5
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_seq_num, r1

lb_tss_i_exit:
  phvwrm.e     p.l4_metadata_exceptions_seen, r2, 0xffff
  tblor        d.u.tcp_session_state_info_d.iflow_exceptions_seen, r2


f_tcp_session_initiator_rtt_calculate:
  jr           r3
  nop

lb_tcp_session_state_responder:
  // New Instruction TBD,
  add          r1, d.u.tcp_session_state_info_d.iflow_tcp_win_scale, r0
  sll          r6, d.u.tcp_session_state_info_d.iflow_tcp_win_sz, d.u.tcp_session_state_info_d.iflow_tcp_win_scale // r6 = rcvr_win_sz
  add          r2, k.l4_metadata_tcp_data_len, r0 // r2 can  be modified by tcp_session_normalization
  seq          c2, d.{u.tcp_session_state_info_d.iflow_tcp_state,u.tcp_session_state_info_d.rflow_tcp_state}, (FLOW_STATE_ESTABLISHED << 4 | FLOW_STATE_ESTABLISHED)
  //bal.c1       r3, f_tcp_session_normalization
    // Normalization checks
  slt          c3, d.u.tcp_session_state_info_d.iflow_tcp_mss, r2 // tcp_mss < tcp_data_len
  slt.!c3      c3, r6, r2 // tcp_rcvr_win_sz < tcp_data_len
  xor          r1, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, k.tcp_option_timestamp_valid
  seq.!c3      c3, r1, 1
  // One more normalizaiton check, if any of the sack_valid bit is set we need to make sure
  // that sack_perm_option_neogitated is TRUE, otherwise we have to enter normalization code
  seq          c4, d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, FALSE
  sne.c4       c4, k.{tcp_option_four_sack_valid...tcp_option_one_sack_valid}, r0
  setcf        c3, [c3 | c4]
  // All normalizaiton checks are in c3. c3 = TRUE (needs normaliaiton, otherwise no normalization)
  // c1 = normalizatio en, c2 = established connection, c3 = bad condition hit
  bcf          [c1 & c2 & c3], lb_tcp_session_responder_normalization
  sub          r7, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, d.u.tcp_session_state_info_d.syn_cookie_delta // r7 = adjusted_seq_num

lb_responder_normalization_return:
  add          r5, r7, r2  // tcp_seq_num_hi
  add          r4, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, r6 // iflow_tcp_ack_num + rcvr_win_sz
  seq          c1, k.tcp_flags, TCP_FLAG_ACK
  seq.!c1      c1, k.tcp_flags, (TCP_FLAG_ACK | TCP_FLAG_PSH)
  sne          c5, r2, r0 // tcp_data_len != 0
  sne          c6, r6, r0 // tcp_rcvr_win_sz != 0
  scwle        c7, r5, r4 // tcp_seq_num_hi <= iflow_tcp_ack_num + rcvr_win_sz (optimized)
  setcf        c1, [c1 & c2 & c6]
  seq          c2, r7, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
  setcf        c2, [c5 & c2 & c7]
  sub          r1, d.u.tcp_session_state_info_d.iflow_tcp_ack_num, 1 // iflow_tcp_ack_num - 1
  scwle        c3, r1, r7   // iflow_tcp_ack_num -1 <= adjusted_seq_num
  scwlt        c4, r7, r4
  setcf.!c2    c2, [!c5 & c3 & c4]
  bcf          ![c1 & c2], lb_tcp_session_state_responder_non_best
  scwlt        c1, d.u.tcp_session_state_info_d.rflow_tcp_ack_num, k.tcp_ackNo
  scwle.c1     c1, k.tcp_ackNo, d.u.tcp_session_state_info_d.iflow_tcp_seq_num
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
  // Since we don't want o change rest of the code, doing what we are doing earlier in
  // good packet case back here.
  // R5 OPTIM
  sub          r5, r5, 1	
 
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
  scwlt        c2, r5, d.u.tcp_session_state_info_d.iflow_tcp_ack_num
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
  smeqb        c3, k.tcp_flags, TCP_FLAG_RST, TCP_FLAG_RST
  setcf        c3, [c1 & c2 & c3 & !c4]
  ori.c3       r2, r2, TCP_RST_WITH_INVALID_ACK_NUM
  phvwr.c3     p.control_metadata_drop_reason[DROP_TCP_RST_WITH_INVALID_ACK_NUM], 1
  b.c3         lb_tss_r_exit
  phvwr.c3     p.capri_intrinsic_drop, 1
  smeqb        c3, k.tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  bcf          [c1 & c2 & c3 & c4], lb_tss_r_tcp_state_transition
  smeqb        c3, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          [c1 & c2 & c3], lb_tss_r_tcp_state_transition
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
  phvwr.c1     p.capri_intrinsic_tm_replicate_en, 1
  phvwr.c1     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_RST_COPY
  tblwr.c1     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_RESET
  b            lb_tss_r_exit
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_RESET

lb_tss_r_4:
  // Below instruction is coming from where we branching to this label (delay slot)
  // sle          c1, FLOW_STATE_ESTABLISHED, d.u.tcp_session_state_info_d.rflow_tcp_state
  smeqb        c2, k.tcp_flags, TCP_FLAG_SYN, TCP_FLAG_SYN
  bcf          ![c1 & c2], lb_tss_r_5
  // Delay slot Instruction for lb_tss_r_5
  smeqb        c2, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  ori          r2, r2, TCP_UNEXPECTED_PKT
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_PKT], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

// Packets with FIN before we are in established state.
// TBD
lb_tss_r_5:
  bcf          ![!c1 & c2], lb_tss_r_6
  add          r1, r0, d.u.tcp_session_state_info_d.rflow_tcp_state
  // We got a FIN in pre-established state.
  ori          r2, r2, TCP_UNEXPECTED_PKT
  phvwr        p.control_metadata_drop_reason[DROP_TCP_UNEXPECTED_PKT], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

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
  // Begin: Can make a macro to handle options in syn from responder
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  seq.c1       c1, k.tcp_option_ws_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq.c1       c1, k.tcp_option_timestamp_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_sack_perm_option_sent, 1
  seq.c1       c1, k.tcp_option_sack_perm_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, 1
  seq          c1, k.tcp_option_mss_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_mss, k.tcp_option_mss_value
  // End: 
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r1
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
  // Begin: Can make a macro to handle options in syn from responder
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  seq.c1       c1, k.tcp_option_ws_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq.c1       c1, k.tcp_option_timestamp_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_sack_perm_option_sent, 1
  seq.c1       c1, k.tcp_option_sack_perm_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, 1
  seq          c1, k.tcp_option_mss_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_mss, k.tcp_option_mss_value
  // End: 
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  b            lb_tss_r_exit
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r1

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
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD
  // Begin: Can make a macro to handle options in syn from responder
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ws_option_sent, 1
  seq.c1       c1, k.tcp_option_ws_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_win_scale, k.tcp_option_ws_value
  tblwr.!c1    d.u.tcp_session_state_info_d.iflow_tcp_win_scale, 0
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_ts_option_sent, 1
  seq.c1       c1, k.tcp_option_timestamp_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, 1
  seq          c1, d.u.tcp_session_state_info_d.iflow_tcp_sack_perm_option_sent, 1
  seq.c1       c1, k.tcp_option_sack_perm_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, 1
  seq          c1, k.tcp_option_mss_valid, 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_mss, k.tcp_option_mss_value
  // End: 
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  b            lb_tss_r_exit
  tblwr        d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r1

  .brcase      FLOW_STATE_TCP_SYN_ACK_RCVD
  b            lb_tss_r_exit
  nop

  .brcase      FLOW_STATE_ESTABLISHED
  smeqb        c1, k.tcp_flags, TCP_FLAG_FIN, TCP_FLAG_FIN
  sne          c2, d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_FIN_RCVD
  setcf        c3, [c1 & c2]
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_FIN_RCVD
  phvwr.c3     p.capri_intrinsic_tm_replicate_en, 1
  b.c3         lb_tss_r_tcp_session_update
  phvwr.c3     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY
  setcf        c3, [c1 & !c2]
  tblwr.c3     d.u.tcp_session_state_info_d.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  tblwr.c3     d.u.tcp_session_state_info_d.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD
  phvwr.c3     p.capri_intrinsic_tm_replicate_en, 1
  b.c3         lb_tss_r_tcp_session_update
  phvwr.c3     p.capri_intrinsic_tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY
  b            lb_tss_r_exit
  nop

  .brcase      FLOW_STATE_FIN_RCVD
  scwle        c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  b.!c1        lb_tss_r_tcp_session_update
  ori.c1       r2, r2, TCP_DATA_AFTER_FIN
  phvwr        p.control_metadata_drop_reason[DROP_TCP_DATA_AFTER_FIN], 1
  b            lb_tss_r_exit
  phvwr        p.capri_intrinsic_drop, 1

  .brcase      FLOW_STATE_BIDIR_FIN_RCVD
  scwle        c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
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
  scwle        c1, d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r5
  add          r1, r5, 1 // tcp_seq_num_hi + 1
  tblwr.c1     d.u.tcp_session_state_info_d.rflow_tcp_seq_num, r1

lb_tss_r_exit:
  phvwrm.e     p.l4_metadata_exceptions_seen, r2, 0xffff
  tblor        d.u.tcp_session_state_info_d.rflow_exceptions_seen, r2


#if 0

// This code is run in the fast path or good packet case.

// R4 - tcp_mss before calling normalization. After that rflow_tcp_ack_num + tcp_rcvr_win_sz
// R6 - tcp_rcvr_win_sz
// R2 - k.l4_metoadata_tcp_data_len
// if ((tcp_mss < tcp_data_len) or (tcp_rcvr_win_sz < tcp_data_len) or
//     (tcp_ts_option_negotiated == TRUE amd tcp_option_timestamp_valid == FALSE) or
//     (tcp_ts_option_negotiated == FALSE amd tcp_option_timestamp_valid == TRUE) or
  slt          c3, r4, r2 // tcp_mss < tcp_data_len
  slt.!c3      c3, r6, r2 // tcp_rcvr_win_sz < tcp_data_len
  xor          r1, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, k.tcp_option_timestamp_valid
  seq.!c3      c3, r1, 1
  bcf.c3       f_tcp_session_normalization

#endif /* 0 */

// We come here only in case we have hit a bad packet. We will normalize the 
// initator and responder values and go into a common code.
// R4 - Copy the tcp_mss values, was not copied in good packet path
// R6 - tcp_rcvr_win_sz - already copied
// R2 - tcp_data_len - already copied
lb_tcp_session_initator_normalization:
  add          r4, d.u.tcp_session_state_info_d.rflow_tcp_mss, r0
  b            lb_tcp_data_len_gt_mss_size
  seq          c2, k.l4_metadata_tcp_data_len_gt_mss_action, NORMALIZATION_ACTION_ALLOW

lb_tcp_session_responder_normalization:
  add          r4, d.u.tcp_session_state_info_d.iflow_tcp_mss, r0
  b            lb_tcp_data_len_gt_mss_size
  seq          c2, k.l4_metadata_tcp_data_len_gt_mss_action, NORMALIZATION_ACTION_ALLOW


lb_tcp_data_len_gt_mss_size:
  b.c2         lb_tcp_data_len_gt_win_size
  seq          c2, k.{l4_metadata_tcp_data_len_gt_win_size_action_sbit0_ebit0, \
                      l4_metadata_tcp_data_len_gt_win_size_action_sbit1_ebit1},\
                      NORMALIZATION_ACTION_ALLOW
  slt          c3, r4, r2
  b.!c3        lb_tcp_data_len_gt_win_size
  seq          c4, k.l4_metadata_tcp_data_len_gt_mss_action, NORMALIZATION_ACTION_DROP
  phvwr.c4.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c4     p.capri_intrinsic_drop, 1
  sub          r1, r2, r4 // r1 = tcp_data_len - mss
  sub          r5, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1 // r5 = k.capri_p4_intrinsic_packet_len - r1
  phvwr        p.capri_p4_intrinsic_packet_len, r5
  phvwr        p.capri_deparser_len_trunc_pkt_len, r4 // This is the payload length after TCP options which is nothing but MSS.
  sub          r5, k.ipv4_totalLen, r1   // r5 = k.ipv4_totalLen - r1
  phvwr        p.ipv4_totalLen, r5
  seq          c1, k.tunnel_metadata_tunnel_terminate, TRUE
  sub.c1       r5, k.udp_len, r1   // r5 = k.udp.len - r1
  phvwr.c1     p.udp_len, r5
  sub.c1       r5, k.inner_ipv4_totalLen, r1   // r5 = k.inner_ipv4_totalLen - r1
  phvwr.c1     p.inner_ipv4_totalLen, r5
  phvwrmi      p.control_metadata_checksum_ctl, CHECKSUM_L3_L4_UPDATE_MASK, CHECKSUM_L3_L4_UPDATE_MASK 
  // Update the checksum calculation.
  // Finally update the tcp_data_len to MSS value which will be used by
  // connection tracking code
  add          r2, r4, r0 // Updating tcp_data_len to mss
  // Update the phv with the new tcp data len as its used in checksum calculation
  phvwr        p.l4_metadata_tcp_data_len, r2
  phvwr        p.capri_intrinsic_payload, 0
  phvwr        p.capri_deparser_len_trunc, 1

lb_tcp_data_len_gt_win_size:
  b.c2         lb_tcp_unexpected_sack_option
  seq          c2, k.l4_metadata_tcp_unexpected_sack_option_action, \
                      NORMALIZATION_ACTION_ALLOW
  slt          c3, r6, r2
  b.!c3        lb_tcp_unexpected_sack_option
  seq          c4, k.{l4_metadata_tcp_data_len_gt_win_size_action_sbit0_ebit0, \
                      l4_metadata_tcp_data_len_gt_win_size_action_sbit1_ebit1},\
                      NORMALIZATION_ACTION_DROP
  phvwr.c4.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c4     p.capri_intrinsic_drop, 1
  // Edit
  sub          r1, r2, r6 // r1 = tcp_data_len - rcvr_win_sz
  sub          r5, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1 // r5 = k.capri_p4_intrinsic_packet_len - r1
  phvwr        p.capri_p4_intrinsic_packet_len, r5
  phvwr        p.capri_deparser_len_trunc_pkt_len, r6 // This is the payload length after TCP options which is nothing but MSS.
  sub          r5, k.ipv4_totalLen, r1   // r5 = k.ipv4_totalLen - r1
  phvwr        p.ipv4_totalLen, r5
  seq          c1, k.tunnel_metadata_tunnel_terminate, TRUE
  sub.c1       r5, k.udp_len, r1   // r5 = k.udp.len - r1
  phvwr.c1     p.udp_len, r5
  sub.c1       r5, k.inner_ipv4_totalLen, r1   // r5 = k.inner_ipv4_totalLen - r1
  phvwr.c1     p.inner_ipv4_totalLen, r5
  phvwrmi      p.control_metadata_checksum_ctl, CHECKSUM_L3_L4_UPDATE_MASK, CHECKSUM_L3_L4_UPDATE_MASK 
  // Finally update the tcp_data_len to MSS value which will be used by
  // connection tracking code
  add          r2, r6, r0 // Updating tcp_data_len to rcvr_win_sz
  // Update the phv with the new tcp data len as its used in checksum calculation
  phvwr        p.l4_metadata_tcp_data_len, r2
  phvwr        p.capri_intrinsic_payload, 0
  phvwr        p.capri_deparser_len_trunc, 1


// Add the sack related normalization check here. We can have SACK option only
// if the SACK_PERM is negotiated.
// Write the code
lb_tcp_unexpected_sack_option:
  b.c2         lb_tcp_unexpected_ts_option
  seq          c2, k.l4_metadata_tcp_unexpected_ts_option_action, \
                      NORMALIZATION_ACTION_ALLOW
  seq          c3, d.u.tcp_session_state_info_d.tcp_sack_perm_option_negotiated, FALSE
  sne          c4, k.{tcp_option_four_sack_valid...tcp_option_one_sack_valid}, r0
  bcf          ![c3 & c4], lb_tcp_unexpected_ts_option
  seq          c4, k.l4_metadata_tcp_unexpected_sack_option_action, \
                      NORMALIZATION_ACTION_DROP
  phvwr.c4.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c4     p.capri_intrinsic_drop, 1
  // Edit option - Remove the sack option and we have to also
  // delete the blob so that the tcp_options_fixup code recomputes
  // the new options and all the header lengths.
  phvwr        p.{tcp_option_four_sack_valid...tcp_option_one_sack_valid}, 0
  phvwr        p.tcp_options_blob_valid, 0

lb_tcp_unexpected_ts_option:
  b.c2         lb_tcp_ts_not_present
  seq          c2, k.l4_metadata_tcp_ts_not_present_drop, ACT_ALLOW
  seq          c3, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, FALSE
  seq          c4, k.tcp_option_timestamp_valid, TRUE
  bcf          ![c3 & c4], lb_tcp_ts_not_present
  seq          c5, k.l4_metadata_tcp_unexpected_ts_option_action, \
                      NORMALIZATION_ACTION_DROP
  phvwr.c5.e   p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr.c5     p.capri_intrinsic_drop, 1
  // Edit option - Remove the timestamp option and we have to also
  // delete the blob so that the tcp_options_fixup code recomputes
  // the new options and all the header lengths.
  phvwr        p.tcp_option_timestamp_valid, 0
  phvwr        p.tcp_options_blob_valid, 0

lb_tcp_ts_not_present:
  seq          c1, k.flow_info_metadata_flow_role, TCP_FLOW_INITIATOR
  bcf          [c1 & c2], lb_initator_normlizaiton_return
  bcf          [!c1 & c2], lb_responder_normalization_return
  seq          c3, d.u.tcp_session_state_info_d.tcp_ts_option_negotiated, TRUE
  seq          c4, k.tcp_option_timestamp_valid, FALSE
  setcf        c3, [c3 & c4]
  bcf          [c1 & !c3], lb_initator_normlizaiton_return
  bcf          [!c1 & !c3], lb_responder_normalization_return
  nop
  phvwr.e      p.control_metadata_drop_reason[DROP_TCP_NORMALIZATION], 1
  phvwr        p.capri_intrinsic_drop, 1
