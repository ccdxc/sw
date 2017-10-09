#include "ipfix/asm_out/INGRESS_p.h"
#include "iris/asm_out/ingress.h"

struct flow_info_d d;
struct phv_        p;

%%

    .param      p4_session_state_base
    .param      ipfix_session_state

ipfix_flow_info:
    phvwr       p.ipfix_metadata_session_index, d.u.flow_info_d.session_state_index
    phvwr       p.ipfix_metadata_flow_role, d.u.flow_info_d.flow_role

    phvwr       p.ipfix_record_common_egress_interface, d.u.flow_info_d.dst_lport
    phvwr       p.ipfix_record_common_egress_interface_type, d.u.flow_info_d.multicast_en
    phvwr       p.ipfix_record_common_start_timestamp, d.u.flow_info_d.start_timestamp
    phvwr       p.ipfix_record_ip_role, d.u.flow_info_d.flow_role
    phvwr       p.ipfix_record_ip_ttl, d.u.flow_info_d.flow_ttl

    // table 0 : lookup session_state
    addi        r1, r0, loword(p4_session_state_base)
    addui       r1, r1, hiword(p4_session_state_base)
    add         r1, r1, d.u.flow_info_d.session_state_index, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_session_state[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
