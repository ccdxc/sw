/******************************************************************************/
TABLE : tcp_normalization
/******************************************************************************/
struct k {
  standard_length : 16;
  // Other fields that are needed in action routines.
  // Total size : 95 bits
  ipv4_flags: 3;
  ip_res_action : 2;
  ip_df_action : 2;
  ip_options_action : 2;
  ipv4_hlen : 4;
  ip_new_tos_action : 2;
  dscp_en : 1;
  ipv4_totalLen : 8;
 
  icmp_request_response_action :2;
  icmp_bad_request_action : 2;
  icmp_code_removal_action : 2;

  icmp_type : 8;
  icmp_code : 8;

  flow_miss : 1;

  tcp_res_action : 2;
  tcp_nosyn_mss_action : 2;
  tcp_nosyn_ws_action : 2;
  tcp_nourg_flag_ptr_action : 2;
  tcp_urg_flag_noptr_action : 2;
  tcp_urg_flag_ptr_nopayload_action : 2;
  tcp_echots_nonzero_ack_zero_action : 2;
  tcp_rst_flag_datapresent_action : 2;
  tcp_datalen_gt_mss_action : 2;
  tcp_datalen_gt_ws_action : 2;
  tcp_noneg_ts_present_action : 2;
  tcp_neg_ts_not_present_action : 2;
  tcp_flags_combination : 2;
  
  tcp_res: 4;
  tcp_urgent_ptr : 16;
  tcp_flags: 8;
  tcp_option_mss_valid: 1;
  tcp_option_ws_valid: 1;
  tcp_urgent_ptr: 16; // parser
  tcp_option_timestamp_valid: 1; // parser
  l4_tcp_data_len: 16; // computed by prev stage
  tcp_option_timestamp_prev_echo_ts: 32; //parser  
  tcp_option_timestamp_ts: 32; //parser
  l4_tcp_mss: 16;  // comes from flow table
  l4_tcp_ws: 16;   // comes from flow table
  tcp_timestamp_negotiated: 1; // comes from flow table
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


smeqb c1, k.ipv4_flags, 0x3, 0
add r1,r0,k.ipv4_hlen
sne c2,k.ipv4_hlen,5
sne c3,k.ip_new_tos_action,0
bcf [c1 | c2 | c3] possible_bad_packet
seq c4,k.vlan_valid, TRUE
add r2,r0,k.ipv4_totalLen
addi r3,r2,18
addi r4,r2,14
seq c5,k.standard_packet_length,r3
seq c6,k.standard_packet_length,r4
setcf c1, c4&c5     
setcf c2, !c4&c6
bcf [c1 | c2] possible_bad_packet
seq c1,k.valid_icmp,TRUE
bcf [!c1] tcp_checks_begin // go to TCP checks if ICMP is not valid
seq c2,k.flow_miss,TRUE
//Can use david's new instruction for below code to compare 6 values in one instruction
seq c3,k.icmp_type, 0
seq c4,k.icmp_type, 14 
seq c5,k.icmp_type, 16 
seq c6,k.icmp_type, 18 
setcf c7, c3|c4|c5|c6
seq c3, k.icmp_type, 34
seq c4, k.icmp_type, 36
seq c5, k.icmp_type, 38
setcf c6, c3|c4|c5
setcf c3, c6|c7
bcf [c1 & c2 & c3] possible_bad_packet
smeqb c2,k.icmp_type,0xff,4
smeqb c3,k.icmp_type,0xff,6
sle c4,15,k.icmp_type
sle c5,k.icmp_type,39
setcf c6, c2 | c3 | c4 | c5
bcf [c1 & c6] possible_bad_packet
smeqb c2,k.icmp_type,0xff,8
smeqb c3,k.icmp_type,0xff,13
sne c4,k.icmp_code,0
setcf c5, c2| c3
bcf [c1 & c4 & c5] possible_bad_packet
nop 
// if packet is icmp - you can go off to end - skipping tcp
b other_normalization_checks_begin
nop
tcp_checks_begin:
add r2,r0,k.tcp_res_action
seq c1,k.tcp_valid,TRUE
bcf [!c1] other_normalization_checks_begin 
seq c2,k.tcp_res,r0
bcf [c1 & c2] possible_bad_packet
smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, TRUE
smeqb c4,k.tcp_option_mss_valid,TRUE
smeqb c5,k.tcp_option_ws_valid,TRUE
bcf [c1 & !c3 & c4] possible_bad_packet
smeqb c6,k.tcp_flags,TCP_FLAGS_MASK_URG, TRUE
bcf [c1 & !c3 & c5] possible_bad_packet
seq c7,k.tcp_urg_ptr,0
bcf [c1 & c6 & c7] possible_bad_packet
seq c2,k.tcp_data_len,0
bcf [c1 & !c6 & !c7] possible_bad_packet
smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_RST, TRUE
bcf [c1 & c6 & c7 & c2] possible_bad_packet
smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_ACK, TRUE
bcf [c1 & !c2 & c3] possible_bad_packet
seq c5,k.tcp_timestamp_negotiated,TRUE
seq c6,k.tcp_timestamp_option_valid,TRUE
seq c7,k.tcp_timestamp_prev_echo_ts,0
bcf [c1 & !c4 & c5 & c6 & c7] possible_bad_packet
sle c4,k.tcp_data_len,k.l4_tcp_mss
sle c2,k.tcp_data_len,k.l4_tcp_ws
bcf [c1 & c4] possible_bad_packet
smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, TRUE
bcf [c1 & c2] possible_bad_packet
smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_ACK, TRUE
bcf [c1 & c5 & !c6] possible_bad_packet
smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_FIN, TRUE
bcf [c1 & !c5 & c6] possible_bad_packet
smeqb c7,k.tcp_flags,TCP_FLAGS_MASK_RST, TRUE
bcf [c1 & !c3 & !c4] possible_bad_packet
smeqb c5,k.tcp_flags,0xff,0xff
bcf [c1 & c2 & c3] possible_bad_packet
smeqb c6,k.tcp_flags,0xff,0
bcf [c1 & c2 & c7] possible_bad_packet
nop 
bcf [c1 & c5] possible_bad_packet
nop
bcf [c1 & c6] possible_bad_packet
nop
other_normalization_checks_begin:
nop

// bad packet big code starts here.
possible_bad_packet:
   
add r2,r0,k.ip_res_action
br r2,ip_norm_res_action
smeqb  c1 k.ipv4_flags,IP_FLAGS_RSVD_MASK, 0x1
ip_norm_res_action0:
    b ip_df_action_begin
    nop // can be replaced with "add r2,r0,k.ip_df_action"
ip_norm_res_action1:
    phvwr.c1.e control_ip_normalization_drop,TRUE // change this here - .e is unconditional.
    nop
    b ip_df_action_begin
    nop
ip_norm_res_action2:
    phvwr.c1.e control_ipv4_normalization_cpu_reason,IP_NORMALIZATION_REASON_RSVD
    nop
    b ip_df_action_begin
    nop
ip_norm_res_action3:
   bcf !c1,ip_df_action_begin
   add r2,r0,k.ipv4_flags
   andi r2,r2,6
   phvwr phv.ipv4_flags,r2
   b ip_df_action_begin
   nop // can be replaced with "add r2,r0,k.ip_df_action"


ip_df_action_begin:
add r2,r0,k.ip_df_action
br r2,ip_norm_df_action
smeqb  c1 k.ipv4_flags,IP_FLAGS_DF_MASK, 0x1
ip_norm_df_action0:
    b ip_options_action_begin
    nop
ip_norm_res_action1:
    phvwr.c1.e control_ip_normalization_drop,TRUE
    nop
    b ip_options_action_begin
    nop
ip_norm_res_action2:
    phvwr.c1.e control_ipv4_normalization_cpu_reason,IP_NORMALIZATION_REASON_DF
    nop
    b ip_options_action_begin
    nop
ip_norm_res_action3:
   bcf !c1, ip_options_action_begin
   add r2,r0,k.ipv4_flags
   andi r2,r2,5
   phvwr phv.ipv4_flags,r2
   b ip_options_action_begin
   nop

ip_options_action_begin:
add r2,r0,k.ip_options_action
br r2, ip_norm_options_action
add r3,r0,k.ipv4_hlen
ip_norm_options_action0:
    b ip_tos_rewrite_action_begin
    nop
ip_norm_options_action1:
    bgti r3,5,ip_options_present_drop
    nop
    b ip_tos_rewrite_action_begin  
    nop
    ip_options_present_drop:         
        phvwr.e control_ip_normalization_drop,TRUE
        nop
ip_norm_options_action2:    
    bgti r3,5,ip_options_present_copy
    nop
    b ip_tos_rewrite_action_begin  
    nop
    ip_options_present_copy:         
        phvwr.e control_ipv4_normalization_cpu_reason,IP_NORMALIZATION_REASON_OPTIONS
        nop
ip_norm_options_action3:
    bgti r3,5,ip_options_present_edit
    nop
    b ip_tos_rewrite_action_begin
    nop
    ip_options_present_edit:
        // set the bits for p4+

ip_tos_rewrite_action_begin:
add r2,r0,k.ip_new_tos_action 
beqi r2,NORMALIZATION_ACTION_EDIT, ip_tos_rewrite_action
add r3,r0,k.dscp_en
ip_tos_rewrite_action0:
    b ip_header_length_begin
    nop
ip_tos_rewrite_action1:      // not supported - just place holder
    b ip_header_length_begin
    nop
ip_tos_rewrite_action2:     // not supported - just place holder
    b ip_header_length_begin
    nop
ip_tos_rewrite_action3:
       phvwr phv.dscp_en,r3
       b ip_header_length_begin
       nop


ip_header_length_begin:
add r2,r0,k.ip_payload_action
br r2, ip_norm_payload_action
seq c1,k.vlan_valid,TRUE
ip_norm_payload_action0:
    b icmp_bad_responses_begin
    nop
ip_norm_payload_action1:
    add r3,r0,k.ipv4_totalLen
    addi r4,r3,18
    seq c2,r4,k.standard_packet_length
    addi r5,r3,14
    seq c3,r5,k.standard_packet_length
    bcf [c1 & !c2] ip_norm_payload_drop_vlan_valid_len_mismatch
    nop
    bcf [!c1 & !c3] ip_norm_payload_drop_vlan_valid_len_mismatch
    nop
    b icmp_bad_responses_begin
    nop
    ip_norm_payload_drop_vlan_valid_len_mismatch:
        phvwr.e control_ip_normalization_drop,TRUE 
        nop
ip_norm_payload_action2:
    add r3,r0,k.ipv4_totalLen
    addi r4,r3,18
    seq c2,r4,k.standard_packet_length
    addi r5,r3,14
    seq c3,r5,k.standard_packet_length
    bcf [c1 & !c2] ip_norm_payload_cpu_vlan_valid_len_mismatch
    nop
    bcf [!c1 & !c3] ip_norm_payload_cpu_vlan_valid_len_mismatch
    nop
    b icmp_bad_responses_begin
    nop
    ip_norm_payload_cpu_vlan_valid_len_mismatch:
        phvwr.e control_ipv4_normalization_cpu_reason,IP_NORMALIZATION_REASON_TOTAL_LENGTH 
        nop
ip_norm_payload_action3:
    add r3,r0,k.ipv4_totalLen
    addi r4,r3,18
    seq c2,r4,k.standard_packet_length
    addi r5,r3,14
    seq c3,r5,k.standard_packet_length
    bcf [c1 & !c2] ip_norm_payload_edit_vlan_valid_len_mismatch
    nop
    bcf [!c1 & !c3] ip_norm_payload_edit_vlan_valid_len_mismatch
    nop
    b icmp_bad_responses_begin
    nop
    ip_norm_payload_edit_vlan_valid_len_mismatch:
        // set p4+ metadata and send it to p4+
        nop

icmp_bad_responses_begin:
add r2,r0,k.icmp_request_response_action
br r2, icmp_request_response_action 
seq c1,k.icmp_valid,TRUE
icmp_request_response_action0:
    b icmp_bad_request_begin
    nop
icmp_request_response_action1:
    smeqb c2,k.icmp_type,0xff,0
    smeqb c3,k.icmp_type,0xff,14
    smeqb c4,k.icmp_type,0xff,16
    smeqb c5,k.icmp_type,0xff,18
    setcf c6, c2|c3|c4|c5
    smeqb c2,k.icmp_type,0xff,34
    smeqb c3,k.icmp_type,0xff,36
    smeqb c4,k.icmp_type,0xff,38
    setcf c5, c2|c3|c4|c6
    smeqb c6, k.flow_miss,0x1,1
    bcf [c1 & c5 & c6] icmp_request_response_action_drop
    nop
    b icmp_bad_request_begin 
    nop
    icmp_request_response_action_drop:
        phvwr.e control_ip_normalization_drop,TRUE
        nop
icmp_request_response_action2:
    smeqb c2,k.icmp_type,0xff,0
    smeqb c3,k.icmp_type,0xff,14
    smeqb c4,k.icmp_type,0xff,16
    smeqb c5,k.icmp_type,0xff,18
    setcf c6, c2|c3|c4|c5
    smeqb c2,k.icmp_type,0xff,34
    smeqb c3,k.icmp_type,0xff,36
    smeqb c4,k.icmp_type,0xff,38
    setcf c5, c2|c3|c4|c6
    smeqb c6, k.flow_miss,0x1,1
    bcf [c1 & c5 & c6] icmp_request_response_action_cpu
    nop
    b icmp_bad_request_begin 
    nop
    icmp_request_response_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,ICMP_NORMALIZATION_REASON_REQ_RESP
        nop
icmp_request_response_action3:
    //Remove the unsupported options in p4+ or add parsing for all of the deprecated icmp options and the format
    // For now do-nothing
    b icmp_bad_request_begin 
    nop

icmp_bad_request_begin:
add r2,r0,k.icmp_bad_request_action
br r2, icmp_bad_request_action 
seq c1,k.icmp_valid,TRUE
icmp_bad_request_action0:
    b icmp_code_removal_begin
    nop
icmp_bad_request_action1:
    smeqb c2,k.icmp_type,0xff,4 
    smeqb c3,k.icmp_type,0xff,6 
    addi r3,r0,15            //icmp_type <= 15 and gt >= 39
    sle c4,r3,k.icmp_type
    addi r4,r0,39
    sle c5,k.icmp_type,r4
    setcf c6, c2|c3
    bcf [c1 & c4 & c5 & c6] icmp_bad_request_action_drop
    nop
    b icmp_code_removal_begin 
    nop
    icmp_bad_request_action_drop:
        phvwr.e control_ip_normalization_drop,TRUE
        nop
icmp_bad_request_action2:     
    smeqb c2,k.icmp_type,0xff,4 
    smeqb c3,k.icmp_type,0xff,6 
    addi r3,r0,15            //icmp_type <= 15 and gt >= 39
    sle c4,r3,k.icmp_type
    addi r4,r0,39
    sle c5,k.icmp_type,r4
    setcf c6, c2|c3
    bcf [c1 & c4 & c5 & c6] icmp_bad_request_action_cpu
    nop
    b icmp_code_removal_begin 
    nop
    icmp_bad_request_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,ICMP_NORMALIZATION_REASON_BAD_REQ
        nop
icmp_bad_request_action2:     
    //Remove the unsupported options in p4+ or add parsing for all of the deprecated icmp options and the format
    // For now do-nothing
    b icmp_code_removal_begin 
    nop

icmp_code_removal_begin:
add r2,r0,k.icmp_code_removal_action
br r2, icmp_code_removal_action 
seq c1,k.icmp_valid,TRUE
icmp_code_removal_action0:
   b tcp_reserved_begin
   nop
icmp_code_removal_action1:
    smeqb c2,k.icmp_type,0xff,8 
    smeqb c3,k.icmp_type,0xff,13
    sne c4,k.icmp_code,r0
    setcf c5, c2|c3
    bcf [c1 & c4 & c5] icmp_code_removal_action_drop
    nop
    b tcp_reserved_begin
    nop
    icmp_code_removal_action_drop:
        phvwr.e control_ip_normalization_drop,TRUE
        nop 
icmp_code_removal_action2:
    smeqb c2,k.icmp_type,0xff,8 
    smeqb c3,k.icmp_type,0xff,13
    sne c4,k.icmp_code,r0
    setcf c5, c2|c3
    bcf [c1 & c4 & c5] icmp_code_removal_action_cpu
    nop
    b tcp_reserved_begin
    nop
    icmp_code_removal_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,ICMP_NORMALIZATION_REASON_CODE_NON_ZERO
        nop 
icmp_code_removal_action3:
    smeqb c2,k.icmp_type,0xff,8 
    smeqb c3,k.icmp_type,0xff,13
    sne c4,k.icmp_code,r0
    setcf c5, c2|c3
    bcf [c1 & c4 & c5] icmp_code_removal_action_edit
    nop
    b tcp_reserved_begin
    nop
    icmp_code_removal_action_edit:
        phvwr phv.icmp_code,r0
        nop

tcp_reserved_begin:
add r2,r0,k.tcp_res_action
br r2, tcp_res_action 
seq c1,k.tcp_res,r0
tcp_res_action0:
    b tcp_mss_syn_begin
    nop
tcp_res_action1:
    bcf c1, tcp_res_action_drop
    nop
    b tcp_mss_syn_begin
    nop
    tcp_res_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_res_action2:
    bcf c1, tcp_res_action_cpu
    nop
    b tcp_mss_syn_begin
    nop
    tcp_res_action_cpu:
        phvwr.e control_tcp_normalization_cpu_reason,TCP_NORMALIZATION_REASON_TCP_RES
        nop
tcp_res_action3:
    bcf c1, tcp_res_action_edit
    nop
    b tcp_mss_syn_begin
    nop
    tcp_res_action_edit:
       phvwr phv.tcp_res,r0
       nop

tcp_mss_syn_begin:
add r2,r0,k.tcp_nosyn_mss_action
br r2, tcp_nosyn_mss_action 
seq c1,k.tcp_valid,TRUE
tcp_nosyn_mss_action0:
    b tcp_ws_syn_begin 
    nop
tcp_nosyn_mss_action1:
    seq c2,phv.tcp_option_mss_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_mss_action_drop
    nop 
    b tcp_ws_syn_begin
    nop 
    tcp_nosyn_mss_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_nosyn_mss_action2:
    seq c2,k.tcp_option_mss_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_mss_action_cpu
    nop 
    b tcp_ws_syn_begin
    nop 
    tcp_nosyn_mss_action_cpu:
        phvwr.e control_tcp_normalization_cpu_reason,TCP_NORMALIZATION_REASON_TCP_NOSYN_MSS
        nop
tcp_nosyn_mss_action3:
    seq c2,k.tcp_option_mss_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_mss_action_edit
    nop 
    b tcp_ws_syn_begin
    nop 
    tcp_nosyn_mss_action_edit:
        //p4+ handoff
        nop

// we can reuse the conditional flags from above if we want.
tcp_ws_syn_begin:     
add r2,r0,k.tcp_nosyn_ws_action
br r2, tcp_nosyn_ws_action 
seq c1,k.tcp_valid,TRUE
tcp_nosyn_ws_action0:
    b tcp_urgflag_set_not_urgptr_begin
    nop
tcp_nosyn_ws_action1:
    seq c2,k.tcp_option_ws_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_ws_action_drop
    nop 
    b tcp_urgflag_set_not_urgptr_begin
    nop 
    tcp_nosyn_ws_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_nosyn_ws_action2:
    seq c2,k.tcp_option_ws_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_ws_action_cpu
    nop 
    b tcp_urgflag_set_not_urgptr_begin
    nop 
    tcp_nosyn_ws_action_cpu:
        phvwr.e control_tcp_normalization_cpu_reason,TCP_NORMALIZATION_REASON_TCP_NOSYN_WS
        nop
tcp_nosyn_ws_action3:
    seq c2,k.tcp_option_ws_valid,TRUE
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_SYN, 0
    bcf [c1 & c2 & c3] tcp_nosyn_ws_action_edit
    nop 
    b tcp_urgflag_set_not_urgptr_action 
    nop 
    tcp_nosyn_ws_action_edit:
        //p4+ handoff
        nop

tcp_urgflag_set_not_urgptr_begin:
add r2,r0,k.tcp_nourg_flag_ptr_action
br r2, tcp_nourg_flag_ptr_action 
seq c1,k.tcp_valid,TRUE
tcp_nourg_flag_ptr_action0:
    b tcp_urg_flag_noptr_begin
    nop
tcp_nourg_flag_ptr_action1:
    sne c2,k.tcp_urgent_ptr, 0
    smeqb c3,k.tcp_flags, TCP_FLAGS_MASK_URG, 0
    bcf [c1 & c2 & c3] tcp_nourg_flag_ptr_action_drop
    nop
    b tcp_urg_flag_noptr_begin
    nop 
    tcp_nourg_flag_ptr_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_nourg_flag_ptr_action2:
    sne c2,k.tcp_urgent_ptr, 0
    smeqb c3,k.tcp_flags, TCP_FLAGS_MASK_URG, 0
    bcf [c1 & c2 & c3] tcp_nourg_flag_ptr_action_cpu
    nop
    b tcp_urg_flag_noptr_begin
    nop 
    tcp_nourg_flag_ptr_action_cpu:
        phvwr.e control_tcp_normalization_cpu_reason,TCP_NORMALIZATION_REASON_TCP_NOURG_URGPTR
        nop 
tcp_nourg_flag_ptr_action3:
    sne c2,k.tcp_urgent_ptr, 0
    smeqb c3,k.tcp_flags, TCP_FLAGS_MASK_URG, 0
    bcf [c1 & c2 & c3] tcp_nourg_flag_ptr_action_edit
    nop
    b tcp_urg_flag_noptr_begin
    nop 
    tcp_nourg_flag_ptr_action_edit:
        phvwr phv.tcp_urgent_ptr,0

tcp_urg_flag_noptr_begin:
add r2,r0,k.tcp_urg_flag_noptr_action
br r2, tcp_urg_flag_noptr_action 
seq c1,k.tcp_valid,TRUE
tcp_urg_flag_noptr_action0:
    b tcp_urg_flag_ptr_nopayload_begin
    nop
tcp_urg_flag_noptr_action1:
    smeqb c2,k.tcp_flags, TCP_FLAGS_MASK_URG,1
    seq c3,k.tcp_urgent_ptr, 0
    bcf [c1 & c2 & c3] tcp_urg_flag_noptr_action_drop
    nop
    b tcp_urg_flag_ptr_nopayload_begin
    nop
    tcp_urg_flag_noptr_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_urg_flag_noptr_action2:
    smeqb c2,k.tcp_flags, TCP_FLAGS_MASK_URG,1
    seq c3,k.tcp_urgent_ptr, 0
    bcf [c1 & c2 & c3] tcp_urg_flag_noptr_action_cpu
    nop
    b tcp_urg_flag_ptr_nopayload_begin
    nop
    tcp_urg_flag_noptr_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_TCP_URG_NOURGPTR
        nop 
tcp_urg_flag_noptr_action3:
    smeqb c2,k.tcp_flags, TCP_FLAGS_MASK_URG,1
    seq c3,k.tcp_urgent_ptr, 0
    bcf [c1 & c2 & c3] tcp_urg_flag_noptr_action_edit
    nop
    b tcp_urg_flag_ptr_nopayload_begin
    nop
    tcp_urg_flag_noptr_action_edit:
       and r3,k.tcp_flags,~TCP_FLAGS_MASK_URG // i know I cannot use ~ - will change it to corresponding value
       phvwr phv.tcp_flags, r3

tcp_urg_flag_ptr_nopayload_begin:
add r2,r0,k.tcp_urg_flag_ptr_nopayload_action
br r2, tcp_urg_flag_noptr_action 
seq c1,k.tcp_valid,TRUE
tcp_urg_flag_ptr_nopayload_action0:
    b tcp_echots_nonzero_ack_zero_begin
    nop
tcp_urg_flag_ptr_nopayload_action1:
    sne c2,k.tcp_urgent_ptr,0
    seq c3,k.tcp_data_len,0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_URG,1
    bcf [c1 & c2 & c3 & c4] tcp_urg_flag_ptr_nopayload_action_drop
    nop
    b tcp_echots_nonzero_ack_zero_begin
    nop
    tcp_urg_flag_ptr_nopayload_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_urg_flag_ptr_nopayload_action2:
    sne c2,k.tcp_urgent_ptr,0
    seq c3,k.tcp_data_len,0
    smeqb c4,phv.tcp_flags,TCP_FLAGS_MASK_URG,1
    bcf [c1 & c2 & c3 & c4] tcp_urg_flag_ptr_nopayload_action_cpu
    nop
    b tcp_echots_nonzero_ack_zero_begin
    nop
    tcp_urg_flag_ptr_nopayload_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_URG_NOPAYLOAD
        nop 
tcp_urg_flag_ptr_nopayload_action3:
    sne c2,k.tcp_urgent_ptr,0
    seq c3,k.tcp_data_len,0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_URG,1
    bcf [c1 & c2 & c3 & c4] tcp_urg_flag_ptr_nopayload_action_edit
    nop
    b tcp_echots_nonzero_ack_zero_begin
    nop
    tcp_urg_flag_ptr_nopayload_action_edit:
        phvwr phv.tcp_urgent_ptr,r0

tcp_echots_nonzero_ack_zero_begin:
add r2,r0,k.tcp_echots_nonzero_ack_zero_action 
br r2,tcp_echots_nonzero_ack_zero_action  
seq c1,k.tcp_valid,TRUE
tcp_echots_nonzero_ack_zero_action0:
    b tcp_rst_flag_datapresent_begin
    nop
tcp_echots_nonzero_ack_zero_action1:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    sne c3,k.tcp_option_timestamp_prev_echo_ts,0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_ACK,0
    bcf [c1 & c2 & c3 & c4] tcp_echots_nonzero_ack_zero_action_drop
    nop 
    b tcp_rst_flag_datapresent_begin
    nop
    tcp_echots_nonzero_ack_zero_action_drop:                 
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_echots_nonzero_ack_zero_action2:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    sne c3,k.tcp_option_timestamp_prev_echo_ts,0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_ACK,0
    bcf [c1 & c2 & c3 & c4] tcp_echots_nonzero_ack_zero_action_cpu
    nop 
    b tcp_rst_flag_datapresent_begin
    nop
    tcp_echots_nonzero_ack_zero_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_URG_NOPAYLOAD
        nop 
tcp_echots_nonzero_ack_zero_action3:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    sne c3,k.tcp_option_timestamp_prev_echo_ts,0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_ACK,0
    bcf [c1 & c2 & c3 & c4] tcp_echots_nonzero_ack_zero_action_edit
    nop 
    b tcp_rst_flag_datapresent_begin
    nop
    tcp_echots_nonzero_ack_zero_action_edit:
        addi r3,r0,0x01010101
        addi r4,r0,0x0101
        phvwr phv.tcp_option_timestamp_prev_echo_ts, r3 
        phvwr phv.tcp_option_timestamp_ts, r3
        phvwr phv[tcp_option_timestamp_optType...tcp_option_timestamp_optLength],r4 

tcp_rst_flag_datapresent_begin:
add r2,r0,k.tcp_rst_flag_datapresent_action
br r2,tcp_echots_nonzero_ack_zero_action  
seq c1,k.tcp_valid,TRUE
tcp_rst_flag_datapresent_action0:
    b tcp_datalen_gt_mss_begin
    nop
tcp_rst_flag_datapresent_action1:
    smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_RST,1
    sne c3,k.tcp_data_len,0
    bcf [c1 & c2 & c3] tcp_rst_flag_datapresent_action_drop
    nop
    b tcp_datalen_gt_mss_begin
    nop
    tcp_rst_flag_datapresent_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_rst_flag_datapresent_action2:         
    smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_RST,1
    sne c3,k.tcp_data_len,0
    bcf [c1 & c2 & c3] tcp_rst_flag_datapresent_action_cpu
    nop
    b tcp_datalen_gt_mss_begin
    nop
    tcp_rst_flag_datapresent_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_RST_PAYLOAD
        nop 
tcp_rst_flag_datapresent_action3:         
    smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_RST,1
    sne c3,k.tcp_data_len,0
    bcf [c1 & c2 & c3] tcp_rst_flag_datapresent_action_edit
    nop
    b tcp_datalen_gt_mss_begin
    nop
    tcp_rst_flag_datapresent_action_edit:
    // send it to p4+

tcp_datalen_gt_mss_begin:
add r2,r0,k.tcp_datalen_gt_mss_action
br r2,tcp_datalen_gt_mss_action
seq c1,k.tcp_valid,TRUE
tcp_datalen_gt_mss_action0:
    b tcp_datalen_gt_ws_begin
    nop
tcp_datalen_gt_mss_action1:
    sle c2,k.tcp_data_len,k.l4_tcp_mss
    nop
    bcf [c1 & c2] tcp_datalen_gt_mss_action_drop
    nop 
    b tcp_datalen_gt_ws_action
    nop
    tcp_datalen_gt_mss_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_datalen_gt_mss_action2:
    sle c2,k.tcp_data_len,k.l4_tcp_mss
    nop
    bcf [c1 & c2] tcp_datalen_gt_mss_action_cpu
    nop 
    b tcp_datalen_gt_ws_begin
    nop
    tcp_datalen_gt_mss_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_PAYLOAD_GT_MSS
        nop 
tcp_datalen_gt_mss_action3:
    sle c2,k.tcp_data_len,k.l4_tcp_mss
    nop
    bcf [c1 & c2] tcp_datalen_gt_mss_action_edit
    nop 
    b tcp_datalen_gt_ws_begin
    nop
    tcp_datalen_gt_mss_action_edit:
    // send it to p4+ for chopping

tcp_datalen_gt_ws_begin:                 
add r2,r0,k.tcp_datalen_gt_ws_action
br r2,tcp_datalen_gt_ws_action
seq c1,k.tcp_valid,TRUE
tcp_datalen_gt_ws_action0:
    b tcp_noneg_ts_present_begin
    nop
tcp_datalen_gt_ws_action1:
    sle c2,k.tcp_data_len,k.l4_tcp_ws
    bcf [c1 & c2] tcp_datalen_gt_ws_action_drop
    nop 
    b tcp_noneg_ts_present_begin
    nop
    tcp_datalen_gt_ws_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_datalen_gt_ws_action2:
    sle c2,k.tcp_data_len,k.l4_tcp_ws
    bcf [c1 & c2] tcp_datalen_gt_ws_action_cpu
    nop 
    b tcp_noneg_ts_present_begin 
    nop
    tcp_datalen_gt_ws_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_PAYLOAD_GT_WS
        nop 
tcp_datalen_gt_ws_action3:
    sle c2,k.tcp_data_len,k.l4_tcp_ws
    bcf [c1 & c2] tcp_datalen_gt_ws_action_edit
    nop 
    b tcp_noneg_ts_present_begin 
    nop
    tcp_datalen_gt_mss_action_edit:
    // send it to p4+ for chopping

tcp_noneg_ts_present_begin:
add r2,r0,k.tcp_noneg_ts_present_action
br r2,tcp_noneg_ts_present_action
seq c1,k.tcp_valid,TRUE
tcp_noneg_ts_present_action0:
    b tcp_neg_ts_not_present_begin
    nop
tcp_noneg_ts_present_action1:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    seq c3,k.tcp_timestamp_negotiated, 0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_SYN,0
    bcf [c1 & c2 & c3 & c4] tcp_noneg_ts_present_action_drop
    nop
    b tcp_neg_ts_not_present_begin
    nop 
    tcp_noneg_ts_present_action_drop:     
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_noneg_ts_present_action2:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    seq c3,k.tcp_timestamp_negotiated, 0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_SYN,0
    bcf [c1 & c2 & c3 & c4] tcp_noneg_ts_present_action_cpu
    nop
    b tcp_neg_ts_not_present_begin
    nop 
    tcp_noneg_ts_present_action_cpu:     
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_NONNEG_TS_PRESENT
        nop 
tcp_noneg_ts_present_action3:
    seq c2,k.tcp_option_timestamp_valid,TRUE
    seq c3,k.tcp_timestamp_negotiated, 0
    smeqb c4,k.tcp_flags,TCP_FLAGS_MASK_SYN,0
    bcf [c1 & c2 & c3 & c4] tcp_noneg_ts_present_action_edit
    nop
    b tcp_neg_ts_not_present_begin
    nop 
    tcp_noneg_ts_present_action_edit: 
        addi r3,r0,0x01010101
        addi r4,r0,0x0101
        phvwr phv.tcp_option_timestamp_prev_echo_ts, r3 
        phvwr phv.tcp_option_timestamp_ts, r3
        phvwr phv[tcp_option_timestamp_optType...tcp_option_timestamp_optLength],r4 

tcp_neg_ts_not_present_begin:        
add r2,r0,k.tcp_neg_ts_not_present_action
br r2,tcp_neg_ts_not_present_action
seq c1,k.tcp_valid,TRUE
tcp_neg_ts_not_present_action0:
    b tcp_flags_combination_begin
    nop
tcp_neg_ts_not_present_action1:
    seq c2,k.tcp_option_timestamp_valid,0
    seq c3,k.tcp_timestamp_negotiated, 1
    bcf [c1 & c2 & c3] tcp_neg_ts_not_present_action_drop
    nop 
    b tcp_flags_combination_begin
    nop
    tcp_neg_ts_not_present_action_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_neg_ts_not_present_action2:
    seq c2,k.tcp_option_timestamp_valid,0
    seq c3,k.tcp_timestamp_negotiated, 1
    bcf [c1 & c2 & c3] tcp_neg_ts_not_present_action_cpu
    nop 
    b tcp_flags_combination_begin
    nop
    tcp_neg_ts_not_present_action_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_NEG_TS_NOT_PRESENT
        nop 
tcp_neg_ts_not_present_action3:
    b tcp_flags_combination_begin
    nop
// No edit option for this


tcp_flags_combination_begin:
add r2,r0,k.tcp_flags_combination
br r2, tcp_flags_combination
seq c1,k.tcp_valid,TRUE
tcp_flags_combination0:
    b normalization_actions_end
    nop
tcp_flags_combination1:
    smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_SYN,1
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_FIN,1
    setcf c4, c2&c3
    smeqb c5,k.tcp_flags,TCP_FLAGS_MASK_RST,1
    setcf c6, c2&c5
    setcf c2, c4|c6
    smeqb c3,k.tcp_flags,0xff,0xff
    smeqb c4,k.tcp_flags,0xff,0
    setcf c5, c3|c4
    setcf c4, c2|c5
    bcf [c1 & c4] tcp_flags_combination_drop
    nop
    b normalization_actions_end
    nop
    tcp_flags_combination_drop:
        phvwr.e control_tcp_normalization_drop,TRUE
        nop
tcp_flags_combination2:
    smeqb c2,k.tcp_flags,TCP_FLAGS_MASK_SYN,1
    smeqb c3,k.tcp_flags,TCP_FLAGS_MASK_FIN,1
    setcf c4, c2&c3
    smeqb c5,k.tcp_flags,TCP_FLAGS_MASK_RST,1
    setcf c6, c2&c5
    setcf c2, c4|c6
    smeqb c3,k.tcp_flags,0xff,0xff
    smeqb c4,k.tcp_flags,0xff,0
    setcf c5, c3|c4
    setcf c4, c2|c5
    bcf [c1 & c2] tcp_flags_combination_cpu
    nop
    b normalization_actions_end
    nop
    tcp_flags_combination_cpu:
        phvwr.e control_ipv4_normalization_cpu_reason,TCP_NORMALIZATION_REASON_BAD_FLAGS
        nop 
tcp_flags_combination3:
    b normalization_actions_end
    nop

normalization_actions_end:     
