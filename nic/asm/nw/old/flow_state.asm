/******************************************************************************/
TABLE : flow_state 
Total 25 Instructions for Seq Number checking (16) and State checking (5)
and update table fields (5)
This is after talking to David and replacing NOPs with the best packet case
code.

After rearranging code best case is 23. (5 + 15 + 3)
Now on top of above we have to handle the most common case which is 
tcp_data_len == 0 which are ACK only packets coming back. To cover this case
we added 5 more instructions.
/******************************************************************************/
struct k {


  // Total size : 
  phv.flow_info_flow_role : 1;
  phv.flow_lkp_tcp_flags : 8;
  phv.tcp_option_ws_valid : 1;
  phv.tcp_option_ws_value : 8;
  phv.tcp_seqNo : 32;
  phv.l4_tcp_data_len : 16;
  
  phv.l4_tcp_split_handshake_action : 1;
}

ACTION : flow_state.validate_tcp_flow_state:
struct d {
  iflow_tcp_seq_num : 32; 
  iflow_tcp_seq_delta : 32; // signed
  iflow_tcp_ack_num : 32; 
  iflow_tcp_ack_delta : 32; // signed
  iflow_tcp_win_sz : 16; 
  iflow_tcp_win_scale : 4; 
  iflow_state : 4; 
  rflow_tcp_seq_num : 32;
  rflow_tcp_seq_delta : 32; // signed
  rflow_tcp_ack_num : 32;
  rflow_tcp_ack_delt : 32; // signed
  rflow_tcp_win_sz : 16;
  rflow_tcp_win_scale : 4; 
  rflow_state : 4; 
  phv.exceptions_seen : 16;
}

// Good Packet Code only Start
if (tcp_data_len && rcvr_win_sz && (tcp_seq_num >= expected_seq) && (seq_num_hi <= (expected_seq + rcvr_win_sz))) {
    // non-overlapping and no inter-segment gap
    return TRUE;
// TCP packets with zero data length and just ACK bits set are also common packets
if (tcp_data_len == 0 && rcvr_win_sz > 0 && ((expected_seq -1 <= tcp_seq_num) && (tcp_seq_num < expected_seq + rcvr_win_sz)))
    return TRUE;

seq c1 k.flow_lkp_tcp_flags TCP_FLAGS_ACK
seq c2 k.flow_lkp_tcp_flags (TCP_FLAGS_ACK | TCP_FLAGS_PUSH)
seq c3 d.iflow_state TCP_STATE_ESTABLISHED
seq c4 d.rflow_state TCP_STATE_ESTABLISHED
bcf ![c1 & c2 & c3 & c4] lb_flow_state_slow_path
seq c1 k.flow_info_flow_role, TCP_FLOW_INITATOR // NOP Replacement

bcf ![c1], lb_seq_flow_role_responder
sne c1, k.l4_tcp_data_len, r0    ===> tcp_data_len != 0 // NOP Replacement.
  add r7, d.rflow_tcp_win_scale, r0
  sllv r1, d.rflow_tcp_win_sz, r7   ===> rcvr_win_sz
  sne c2, r1, r0                    ===> rcvr_win_sz != 0
  add r2, d.rflow_tcp_ack_num, r0   ===> expected_seq
  add r3, k.tcp_seqNo, r0           ===> tcp_seq_num 
  sle c3 r2, r3                     ===> expected_seq <= tcp_seq_num
  add r4, r3, k.l4_tcp_data_len     ===> seq_num_hi
  add r5, r2, r1                    ===> expected_seq + rcvr_win_sz
  sle c4 r4, r5                     ===> seq_num_hi <= expected_seq + rcvr_win_sz
  bcf [c1 & c2 & c3 & c4] lb_flow_state_write_back_initator
  nop
  // above case covers for packets with non-zero tcp_data_len but packets
  // packets with just ACK bit set with zero tcp_data_len are also common 
  // so we also need to handle that case
  subi r6, r2, 1                    ===> expected_seq - 1
  sle c5, r6, r3                    ===> (expected_seq -1 <= tcp_seq_num
  slt c6, r3, r5                    ===> (tcp_seq_num < expected_seq + rcvr_win_sz)
  bcf [!c1 & c2 & c5 & c6] lb_flow_state_write_back_initator
  nop
  b lb_flow_state_slow_path
  nop

lb_seq_flow_role_responder:
  add r7, d.iflow_tcp_win_scale, r0
  sllv r1, d.iflow_tcp_win_sz, r7   ===> rcvr_win_sz
  sne c2, r1, r0                    ===> rcvr_win_sz != 0
  add r2, d.iflow_tcp_ack_num, r0   ===> expected_seq
  add r3, k.tcp_seqNo, d.rflow_tcp_seq_delta           ===> tcp_seq_num (adjusted)
  sle c3 r2, r3                     ===> expected_seq <= tcp_seq_num
  add r4, r3, k.l4_tcp_data_len     ===> seq_num_hi
  add r5, r2, r1                    ===> expected_seq + rcvr_win_sz
  sle c4 r4, r5                     ===> seq_num_hi <= expected_seq + rcvr_win_sz
  bcf [c1 & c2 & c3 & c4] lb_flow_state_write_back_responder
  nop
  // above case covers for packets with non-zero tcp_data_len but packets
  // packets with just ACK bit set with zero tcp_data_len are also common 
  // so we also need to handle that case
  subi r6, r2, 1                    ===> expected_seq - 1
  sle c5, r6, r3                    ===> (expected_seq -1 <= tcp_seq_num
  slt c6, r3, r5                    ===> (tcp_seq_num < expected_seq + rcvr_win_sz)
  bcf [!c1 & c2 & c5 & c6] lb_flow_state_write_back_initator
  nop
  b lb_flow_state_slow_path
  nop

lb_flow_state_writeback_initator:
  tblwr d.iflow_tcp_win_sz   
  tblwr d.iflow_tcp_Ack_num
  tblwr d.iflow_tcp_seq_num

lb_flow_state_writeback_responder:

// Good Packet Code only End

// First do sequence number checks based on C Code
// r1 - rcvr_win_sz 
// r2 - expected_seq_num
// r3 - adjusted_seq_num
// r4 - tcp_seq_num_hi
// r5 - adjusted_ack_num
// r6 - expected_ack_num
//
lb_flow_seq_number_checks:
seq c1 k.flow_info_flow_role, TCP_FLOW_INITATOR
bcf ![c1], lb_seq_flow_role_responder
nop
lb_seq_flow_role_initator:
  add r7, d.rflow_tcp_win_scale, r0
  sllv r1, d.rflow_tcp_win_sz, r7
  add r2, d.rflow_tcp_ack_num, r0
  add r3, k.tcp_seqNo, r0 
  add r4, r3, k.l4_tcp_data_len
  add r5, k.tcp_ackNo, d.iflow_tcp_ack_delta
  add r6, d.iflow_tcp_ack_num, r0
  b lb_seq_check_start
  nop
lb_seq_flow_role_responder:
  add r7, d.iflow_tcp_win_scale, r0
  sllv r1, d.iflow_tcp_win_sz, r7
  add r2, d.iflow_tcp_ack_num, r0
  add r3, k.tcp_seqNo, d.rflow_tcp_seq_delta 
  add r4, r3, k.l4_tcp_data_len
  add r5, k.tcp_ackNo, r0
  add r6, d.iflow_tcp_ack_num, r0
  b lb_seq_check_start
  nop

lb_seq_check_start:
  sne c2, k.l4_tcp_data_len, r0
  sne c1, r1, r0
  .csbegin
  cswitch [c2, c1] // Flags here should be given in decreasing order
  nop
  // tcp_data_len == 0 and rcvr_win_sz == 0
  .cscase 0
    seq c1, r2, r0
    bcf [c1], lb_seq_check_done
    seq c2, r3, r2
    sub r7, r2, 1
    seq c3, r3, r7
    // c2 or c3 has to be 1 otherwise its a invalid seq
    bcf [c2 | c3], lb_seq_check_passed
    nop
    b lb_seq_check_failed
    nop
  // tcp_data_len == 0 and rcvr_win_sz != 0
  .cscase 1
    seq c1, r2, r0
    bcf [c1], lb_seq_check_syn_retransmit
    nop
    // Continue to check the seq num in range or not
    sub r7, r2, 1
    sle c2, r7, r3
    add r7, r2, r1
    slt c3, r3, r7
    bcf [c2 & c3], lb_seq_check_passed
    nop
    b lb_seq_check_failed
    nop
    lb_seq_check_syn_retransmit:
      // For now not trying to do the checks just to count the tcp_syn retransmits
      // from initator
      b lb_seq_check_passed
      nop
  // tcp_data_len != 0 and rcvr_win_sz == 0
  .cscase 2
    // Increment the counter for win_zero drops
    b lb_seq_check_failed
    nop
  // tcp_data_len != 0 and rcvr_win_sz != 0
  .cscase 3
    // Perfect Packet
    seq c1, r3, r2
    add r7, r2, r1
    sle c2, r4, r7
    bcf [c1 & c2], lb_seq_check_passed
    nop
    // In window packet but not perfect
    // c2 used from above
    slt c1, r2, r3
    scf c3, [c1 & c2]
    tbladd.c3 d.num_packet_reorders, 1
    b.c3 lb_seq_check_passed
    nop
    // left of window
    slt c1, r3, r2
    slt c2, r4, r2
    scf c3, [c1 & c2]
    tbladd.c3 d.num_full_retransmits, 1
    b.c3 lb_seq_check_passed
    nop
    // left overlap
    slt c1, r3, r2
    add r7, r2, r1
    sle c2, r4, r7
    scf c3, [c1 & c2]
    tbladd.c3 d.num_partial_overlaps, 1
    b.c3 lb_seq_check_passed
    nop
    // Packet is overlapping to the right or completely out of window.
    tbladd d.num_tcp_win_drops, 1
    b lb_seq_check_failed  
    nop


    
lb_seq_check_failed:
  // Mark for drop and exit the action routine.
  // increment a counter.

lb_seq_check_passed:
 
lb_seq_check_done:

lb_flow_state_transition:
// C Code based state transition:
// c1 = flow_role
seq c1 k.flow_info_flow_role, TCP_FLOW_INITATOR
bcf [c1], lb_flow_role_initiator
nop
b lb_flow_role_responder
nop
lb_flow_role_initator:
  seq c2 d.iflow_state FLOW_STATE_TCP_SYN_SENT
  seq c3 d.rflow_state FLOW_STATE_TCP_SYN_ACK_RCVD
  smeqb c4 k.flow_lkp_tcp_flags, TCP_FLAG_ACK, TCP_FLAG_ACK
  bcf ![c2 & c3 & c4], lb_flow_role_initiator_established_else
  nop
lb_flow_role_initiator_established:
  tblwr d.iflow_state FLOW_STATE_ESTABLISHED
  tblwr d.rflow_state FLOW_STATE_ESTABLISHED
  b flow_role_initator_responder_done
  nop
lb_flow_role_initiator_established_else:
  seq c3 d.rflow_state FLOW_STATE_TCP_SYN_RCVD
  smeqb c4 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN_ACK, TCP_FLAGS_SYN_ACK
  bcf [c2 & c3 & c4], lb_flow_role_initiator_split_handshake
  nop
  b flow_role_initator_responder_done
  nop
lb_flow_role_initiator_split_handshake:
  seq c5 k.l4_tcp_split_handshake_action, ACT_DROP
  bcf [c5], lb_flow_role_initiator_split_handshake_drop
  nop
  b lb_flow_role_initiator_split_handshake_no_drop
  mop
lb_flow_role_initiator_split_handshake_drop
  // Increment the counter
  // set the drop flag
  b flow_role_initator_responder_done
  nop
lb_flow_role_initiator_split_handshake_no_drop
  tblwr d.flow_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  b flow_role_initator_responder_done
  nop

flow_role_responder:
  seq c2 d.rflow_state FLOW_STATE_INIT
  smeqb c3 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN_ACK, TCP_FLAGS_SYN_ACK
  bcf [c2 & c3], lb_flow_role_responder_init_syn_ack_rcvd
  nop
  b lb_flow_role_responder_init_syn_ack_rcvd_else
  nop
lb_flow_role_responder_init_syn_ack_rcvd:
  tblwr d.rflow_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  seq c4 k.tcp_option_ws_valid, 1
  tblwr.c4 d.rflow_tcp_win_scale, k.tcp_option_ws_value
  b flow_role_initator_responder_done
lb_flow_role_responder_init_syn_ack_rcvd_else:
  smeqb c3 k.flow_lkp_tcp_flags, TCP_FLAGS_ACK, TCP_FLAGS_ACK
  add r1, d.iflow_tcp_seq_num, 1
  seq c4 k.tcp_ackNo, r1
  bcf [c2 & c3 & c4], lb_flow_role_responder_init_ack_rcvd
  nop
  b lb_flow_role_responder_init_ack_rcvd_else
  nop
lb_flow_role_responder_init_ack_rcvd:
  seq c5 k.l4_tcp_split_handshake_action, ACT_DROP
  bcf [c5], lb_flow_role_responder_init_ack_rcvd_split_handshake_drop
  nop
  b lb_flow_role_responder_init_ack_rcvd_split_handshake_no_drop
  mop
lb_flow_role_responder_init_ack_rcvd_split_handshake_drop
  // Increment the counter
  // set the drop flag
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_init_ack_rcvd_split_handshake_no_drop
  tblwr d.rflow_state, FLOW_STATE_TCP_ACK_RCVD
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_init_ack_rcvd_else
  smeqb c3 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN, TCP_FLAGS_SYN
  seq c4 k.tcp_ackNo, r0
  bcf [c2 & c3 & c4], lb_flow_role_responder_init_syn_rcvd
  nop
  b lb_flow_role_responder_init_syn_rcvd_else
  nop
lb_flow_role_responder_init_syn_rcvd:
  seq c5 k.l4_tcp_split_handshake_action, ACT_DROP
  bcf [c5], lb_flow_role_responder_init_syn_rcvd_split_handshake_drop
  nop
  b lb_flow_role_responder_init_syn_rcvd_split_handshake_no_drop
  mop
lb_flow_role_responder_init_syn_rcvd_split_handshake_drop
  // Increment the counter
  // set the drop flag
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_init_syn_rcvd_split_handshake_no_drop
  tblwr d.rflow_state, FLOW_STATE_TCP_SYN_RCVD
  seq c4 k.tcp_option_ws_valid, 1
  tblwr.c4 d.rflow_tcp_win_scale, k.tcp_option_ws_value
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_init_syn_rcvd_else
  seq c2 d.rflow_state FLOW_STATE_TCP_ACK_RCVD
  smeqb c3 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN, TCP_FLAGS_SYN
  seq c4 k.tcp_ackNo, r0
  bcf [c2 & c3 & c4], lb_flow_role_responder_ack_rcvd_to_syn_ack_rcvd
  nop
  b lb_flow_role_responder_ack_rcvd_to_syn_ack_rcvd_else
  nop
lb_flow_role_responder_ack_rcvd_to_syn_ack_rcvd:
  tblwr d.rflow_state, FLOW_STATE_TCP_SYN_ACK_RCVD
  // We should update the window scale here right ?
  // C code doesnt have it, check that first.
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_ack_rcvd_to_syn_ack_rcvd_else:
  seq c2 d.rflow_state FLOW_STATE_TCP_SYN_RCVD
  seq c3 k.flow_lkp_tcp_flags, TCP_FLAGS_ACK
  smeqb c4 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN_ACK, TCP_FLAGS_SYN_ACK
  seq c5 d.iflow_state FLOW_STATE_TCP_SYN_ACK_RCVD
  bcf [c2 & c3 & c4 & c5], lb_flow_role_responder_syn_rcvd_to_established
  nop
  b flow_role_initator_responder_done
  nop
lb_flow_role_responder_syn_rcvd_to_established:
  tblwr d.rflow_state, FLOW_STATE_ESTABLISHED
  tblwr d.iflow_state, FLOW_STATE_ESTABLISHED
  
flow_role_initator_responder_done:
  seq c1 d.flow_state FLOW_STATE_CLOSING
  seq c2 d.flow_state FLOW_STATE_RESET
  smeqb c3 k.flow_lkp_tcp_flags, TCP_FLAGS_SYN, TCP_FLAGS_SYN
  bcf [c1 & c3], lb_flow_state_closing_drop_syn
  nop
  bcf [c2 & c3], lb_flow_state_closing_drop_syn
  nop
  b lb_next_block_of_code
  nop
lb_flow_state_closing_drop_syn:
  // Increment the counter
  // set the drop flag

lb_next_block_of_code:
  smeqb c1 k.flow_lkp_tcp_flags, TCP_FLAGS_RST, TCP_FLAGS_RST
  tblwr.c1 d.flow_state FLOW_STATE_RESET
  tblwr.c1 d.rflow_state FLOW_STATE_RESET
  smeqb c2 k.flow_lkp_tcp_flags, TCP_FLAGS_FIN, TCP_FLAGS_FIN
  tblwr.c2 d.flow_state FLOW_STATE_CLOSING
  


