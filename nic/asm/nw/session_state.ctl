#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct session_state_k k;
struct session_state_d d;
struct phv_        p;

k = {
 l4_metadata_tcp_data_len = 100;
 l4_metadata_tcp_rcvr_win_sz = 4096;
 l4_metadata_tcp_mss = 1480;
 tcp_option_timestamp_valid = 0;
 flow_info_metadata_flow_role = 0; // Initator
 tcp_seqNo_sbit0_ebit15 = 0x1234;
 tcp_ackNo = 0x2234;
 tcp_flags = 0x18; // ACK + PSH
 tcp_window = 4096;
};

d = {
 u.tcp_session_state_info_d.iflow_tcp_seq_num = 0x1234;
 u.tcp_session_state_info_d.iflow_tcp_ack_num = 0x2234;
 u.tcp_session_state_info_d.iflow_tcp_win_sz = 8192;
 u.tcp_session_state_info_d.iflow_tcp_win_scale = 0;
 u.tcp_session_state_info_d.iflow_tcp_mss = 1480;
 u.tcp_session_state_info_d.iflow_tcp_state = FLOW_STATE_ESTABLISHED;
 u.tcp_session_state_info_d.rflow_tcp_seq_num = 0x2234;
 u.tcp_session_state_info_d.rflow_tcp_ack_num = 0x1234;
 u.tcp_session_state_info_d.rflow_tcp_win_sz = 8192;
 u.tcp_session_state_info_d.rflow_tcp_win_scale = 0;
 u.tcp_session_state_info_d.rflow_tcp_mss = 1480;
 u.tcp_session_state_info_d.rflow_tcp_state = FLOW_STATE_ESTABLISHED;
 u.tcp_session_state_info_d.syn_cookie_delta = 0;
};


