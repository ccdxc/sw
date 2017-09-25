#include "ipfix_metadata.p4"

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0          ipfix_start
#define tx_table_s1_t0          ipfix_flow_hash
#define tx_table_s2_t0          ipfix_flow_info
#define tx_table_s3_t0          ipfix_session_state
#define tx_table_s4_t0          ipfix_flow_stats

#define tx_table_s0_t0_action	ipfix_start
#define tx_table_s1_t0_action	ipfix_flow_hash
#define tx_table_s2_t0_action	ipfix_flow_info
#define tx_table_s3_t0_action	ipfix_session_state
#define tx_table_s4_t0_action	ipfix_flow_stats

#include "../common-p4+/common_txdma.p4"

@pragma scratch_metadata
metadata ipfix_scratch_metadata_t scratch_metadata;
@pragma scratch_metadata
metadata ipfix_qstate_metadata_t qstate_metadata;

@pragma pa_header_union ingress common_global
metadata ipfix_metadata_t ipfix_metadata;
@pragma pa_header_union ingress common_t0_s2s
metadata ipfix_t0_metadata_t ipfix_t0_metadata;

@pragma dont_trim
metadata ipfix_ipv6_udp_header_t ipfix_ipv6_udp;
@pragma dont_trim
metadata ipfix_ipv4_udp_header_t ipfix_ipv4_udp;
@pragma dont_trim
metadata ipfix_record_header_t ipfix_record_header;
@pragma dont_trim
metadata ipfix_record_common_t ipfix_record_common;
@pragma dont_trim
metadata ipfix_record_ipv4_t ipfix_record_ipv4;
@pragma dont_trim
metadata ipfix_record_ipv6_t ipfix_record_ipv6;
@pragma dont_trim
metadata ipfix_record_ip_t ipfix_record_ip;
@pragma dont_trim
metadata ipfix_record_nonip_t ipfix_record_nonip;

@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd3;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t mem2pkt_cmd;

action ipfix_start(rsvd, cos_a, cos_b, cos_sel, eval_last, host_rings,
                   total_rings, pid, pindex, cindex, eindex, pktaddr, pktsize) {
    modify_field(qstate_metadata.rsvd, rsvd);
    modify_field(qstate_metadata.cos_a, cos_a);
    modify_field(qstate_metadata.cos_b, cos_b);
    modify_field(qstate_metadata.cos_sel, cos_sel);
    modify_field(qstate_metadata.eval_last, eval_last);
    modify_field(qstate_metadata.host_rings, host_rings);
    modify_field(qstate_metadata.total_rings, total_rings);
    modify_field(qstate_metadata.pid, pid);
    modify_field(qstate_metadata.pindex, pindex);
    modify_field(qstate_metadata.cindex, cindex);

    modify_field(ipfix_metadata.qstate_addr, p4_txdma_intr.qstate_addr);
    modify_field(ipfix_t0_metadata.eindex, eindex);
    modify_field(ipfix_t0_metadata.pktaddr, pktaddr);
    modify_field(ipfix_t0_metadata.pktsize, pktsize);
}

action ipfix_flow_hash() {
}

action ipfix_flow_info() {
}

action ipfix_session_state() {
    modify_field(scratch_metadata.flow_index, ipfix_metadata.flow_index);
    modify_field(scratch_metadata.session_index, ipfix_metadata.session_index);
    modify_field(scratch_metadata.flow_role, ipfix_metadata.flow_role);
}

action ipfix_flow_stats() {
}
