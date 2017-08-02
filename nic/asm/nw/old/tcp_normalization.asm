/******************************************************************************/
TABLE : tcp_normalization
/******************************************************************************/
struct k {

  // Other fields that are needed in action routines.
  // Total size : 95 bits
  phv.tcp_res: 4;
  phv.tcp_flags: 8;
  phv.tcp_option_mss_valid: 1;
  phv.tcp_option_ws_valid: 1;
  phv.tcp_urgent_ptr: 16;
  phv.tcp_option_timestamp_valid: 1;
  phv.l4_tcp_data_len: 16;
  phv.tcp_option_timestamp_prev_echo_ts: 32; 
  phv.l4_tcp_mss: 16;
  phv.tcp_timestamp_negotiated: 1;
}

ACTION : flow_table.flow_hit:
ACTION : tcp_normalization.tcp_normalization:
struct d {
  // 
  //
  tcp_norm_drop_cnt1; 32;
  tcp_norm_drop_cnt2; 32;
  tcp_norm_drop_cnt3; 32;
  tcp_norm_drop_cnt4; 32;
  tcp_norm_drop_cnt5; 32;
  tcp_norm_drop_cnt6; 32;
  tcp_norm_drop_cnt7; 32;
  tcp_norm_drop_cnt8; 32;
  tcp_norm_drop_cnt9; 32;
  tcp_norm_drop_cnt10; 32;
}

sne c1 k.tcp_res, r0
phvwr.c1 control_tcp_normalization_drop, TRUE
tbladd.c1 , d.tcp_norm_drop_cnt1, 1
smeqb c1 k.tcp_flags, TCP_FLAGS_MASK_SYN, 0x00
seq c2 k.tcp_option_mss_valid, 1
bcf [c1 & c2] tcp_mss_non_syn_drop_true
nop
b tcp_mss_non_syn_drop_false:
nop
tcp_mss_non_syn_drop_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt2, 1
tcp_mss_non_syn_drop_false:
seq c2 k.tcp_option_ws_valid, 1
bcf [c1 & c2] tcp_ws_non_syn_drop_true
nop
b tcp_ws_non_syn_drop_false
nop
tcp_ws_non_syn_drop_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt3, 1
tcp_ws_non_syn_drop_false:
seq c1 k.tcp_option_timestamp_valid, 1
seq c2 k.tcp_timestamp_negotiated, FALSE
bcf [c1 & c2] tcp_timestamp_non_neg_drop_true
nop
b tcp_timestamp_non_neg_drop_false
nop
tcp_timestamp_non_neg_drop_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt4, 1
tcp_timestamp_non_neg_drop_false:
smeqb c1 k.tcp_flags, TCP_FLAGS_MASK_URG, 0x00
seq c2 k.tcp_urgent_ptr, 0
bcf [c1 & !c2] tcp_urg_ptr_non_zero_true
nop
b tcp_urg_ptr_non_zero_false:
nop
tcp_urg_ptr_non_zero_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt5, 1
tcp_urg_ptr_non_zero_false:
bcf [!c1 & c2] tcp_urg_flag_non_zero_true
nop
tcp_urg_flag_non_zero_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt6, 1
tcp_urg_flag_non_zero_false:
seq c3 k.l4_tcp_data_len, 0 // Can be moved to nop above
bcf [!c1 & !c2 & c3] tcp_urg_payload_zero_true
nop
b tcp_urg_payload_zero_false
nop
tcp_urg_payload_zero_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt7, 1
tcp_urg_payload_zero_false:
smeqb c1 k.tcp_flags, TCP_FLAGS_MASK_ACK, 0x00
seq c2 k.tcp_option_timestamp_valid, 1
seq c3 k.phv.tcp_option_timestamp_prev_echo_ts, 0
bcf [c1 & c2 & !c3] tcp_timestamp_prev_echo_ts_true
nop
b tcp_timestamp_prev_echo_ts_false
tcp_timestamp_prev_echo_ts_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt8, 1
tcp_timestamp_prev_echo_ts_false
smeqb c1 k.tcp_flags, TCP_FLAGS_MASK_RST, 0x00
seq c3 k.l4_tcp_data_len, 0 // Can be moved to nop above
bcf [!c1 & !c3] tcp_rst_flag_data_non_zero_true
nop
b tcp_rst_flag_data_non_zero_false
nop
tcp_rst_flag_data_non_zero_true:
  phvwr control_tcp_normalization_drop, TRUE
  tbladd d.tcp_norm_drop_cnt9, 1
sle c1 k.l4_tcp_mss, k.l4_tcp_data_len
phvwr.c1.e control_tcp_normalization_drop, TRUE
tbladd.c1 d.tcp_norm_drop_cnt10, 1


  


