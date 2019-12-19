#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "p4/asm_out/ingress.h"

struct ipfix_flow_info_k k;
struct flow_info_d       d;
struct phv_              p;

%%

    .param      p4_session_state_base
    .param      ipfix_session_state

ipfix_flow_info:
    bbeq        k.ipfix_metadata_scan_complete, 1, ipfix_flow_info_complete

    // check if the flow has to be exported by this exporter
    crestore    [c4-c1], k.ipfix_metadata_export_en, 0xF
    seq.c1      c1, k.ipfix_metadata_export_id, d.u.flow_info_d.export_id1
    seq.c2      c2, k.ipfix_metadata_export_id, d.u.flow_info_d.export_id2
    seq.c3      c3, k.ipfix_metadata_export_id, d.u.flow_info_d.export_id3
    seq.c4      c4, k.ipfix_metadata_export_id, d.u.flow_info_d.export_id4
    bcf         [!c1&!c2&!c3&!c4], ipfix_flow_info_exit

    phvwr       p.ipfix_metadata_session_index, d.u.flow_info_d.session_state_index
    phvwr       p.ipfix_metadata_flow_role, d.u.flow_info_d.flow_role

    phvwr       p.ipfix_record_common_egress_interface, d.u.flow_info_d.dst_lport
    // phvwr       p.ipfix_record_common_egress_interface_type, d.u.flow_info_d.multicast_en
    phvwr       p.ipfix_record_common_start_timestamp, d.u.flow_info_d.start_timestamp
    phvwr       p.ipfix_record_ip_role, d.u.flow_info_d.flow_role
    phvwr       p.ipfix_record_ip_ttl, d.u.flow_info_d.flow_ttl

ipfix_flow_info_complete:
    // table 0 : lookup session_state
    addi        r1, r0, loword(p4_session_state_base)
    addui       r1, r1, hiword(p4_session_state_base)
    add         r1, r1, d.u.flow_info_d.session_state_index, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_session_state[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    // enable table 0 in next stage
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0x8
    nop

ipfix_flow_info_exit:
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr       p.p4_intr_global_drop, 1
