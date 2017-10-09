#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "iris/asm_out/ingress.h"
#include "../../p4/nw/include/defines.h"

struct ipfix_session_state_k k;
struct session_state_d       d;
struct phv_                  p;

%%

    .param      p4_flow_stats_base
    .param      p4_flow_atomic_stats_base
    .param      ipfix_flow_stats
    .param      ipfix_flow_atomic_stats

ipfix_session_state:
    seq         c1, k.ipfix_metadata_session_index, 0
    bcf         [c1], ipfix_session_state_common
    seq         c1, k.ipfix_metadata_flow_role, TCP_FLOW_INITIATOR
    bcf         [c1], ipfix_session_state_initiator
    phvwr       p.ipfix_record_ip_tcp_seq_num, d.u.tcp_session_state_info_d.rflow_tcp_seq_num
    phvwr       p.ipfix_record_ip_tcp_ack_num, d.u.tcp_session_state_info_d.rflow_tcp_ack_num
    phvwr       p.ipfix_record_ip_tcp_win_sz, d.u.tcp_session_state_info_d.rflow_tcp_win_sz
    phvwr       p.ipfix_record_ip_tcp_win_scale, d.u.tcp_session_state_info_d.rflow_tcp_win_scale
    phvwr       p.ipfix_record_ip_tcp_win_mss, d.u.tcp_session_state_info_d.rflow_tcp_mss
    phvwr       p.ipfix_record_ip_tcp_state, d.u.tcp_session_state_info_d.rflow_tcp_state
    phvwr       p.ipfix_record_ip_tcp_exceptions, d.u.tcp_session_state_info_d.rflow_exceptions_seen
    b           ipfix_session_state_common
    phvwr       p.ipfix_record_ip_tcp_rtt, d.u.tcp_session_state_info_d.rflow_rtt

ipfix_session_state_initiator:

ipfix_session_state_common:
    // table 0 : lookup flow_stats
    addi        r1, r0, loword(p4_flow_stats_base)
    addui       r1, r1, hiword(p4_flow_stats_base)
    add         r1, r1, k.ipfix_metadata_flow_index, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_stats[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    // table 1 : lookup flow stats from atomic add region
    addi        r1, r0, loword(p4_flow_atomic_stats_base)
    addui       r1, r1, hiword(p4_flow_atomic_stats_base)
    add         r1, r1, k.ipfix_metadata_flow_index, 5
    phvwr       p.common_te1_phv_table_addr, r1
    phvwri      p.common_te1_phv_table_pc, ipfix_flow_atomic_stats[33:6]
    phvwr       p.common_te1_phv_table_raw_table_size, 5
    phvwr       p.common_te1_phv_table_lock_en, 0

    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 1
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
