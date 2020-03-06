#include "ipfix_metadata.p4"

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0          ipfix_start
#define tx_table_s1_t0          ipfix_flow_hash
#define tx_table_s1_t1          ipfix_export_packet
#define tx_table_s2_t0          ipfix_flow_info
#define tx_table_s3_t1          ipfix_read_exported_flow_stats
#define tx_table_s3_t0          ipfix_session_state
#define tx_table_s4_t1          ipfix_flow_stats
#define tx_table_s5_t0          ipfix_create_record
#define tx_table_s6_t1          ipfix_update_exported_flow_stats

#define tx_table_s0_t0_action	ipfix_start
#define tx_table_s1_t0_action	ipfix_flow_hash
#define tx_table_s1_t1_action   ipfix_export_packet
#define tx_table_s2_t0_action	ipfix_flow_info
#define tx_table_s3_t0_action	ipfix_session_state
#define tx_table_s3_t1_action   ipfix_read_exported_flow_stats
#define tx_table_s4_t1_action	ipfix_flow_stats
#define tx_table_s5_t0_action	ipfix_create_record
#define tx_table_s6_t1_action   ipfix_update_exported_flow_stats

#include "../common-p4+/common_txdma.p4"

@pragma scratch_metadata
metadata ipfix_scratch_metadata_t scratch_metadata;
@pragma scratch_metadata
metadata ipfix_qstate_metadata_t qstate_metadata;

@pragma pa_header_union ingress common_global
metadata ipfix_metadata_t ipfix_metadata;

@pragma dont_trim
@pragma pa_header_union ingress common_t0_s2s
metadata ipfix_flow_hash_metadata_t ipfix_t0_metadata;

@pragma dont_trim
@pragma pa_header_union ingress common_t1_s2s
metadata ipfix_flow_hash_metadata_t ipfix_t1_metadata;

@pragma dont_trim
@pragma pa_header_union ingress to_stage_5
metadata ipfix_s5_metadata_t ipfix_s5_metadata;

@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t ipfix_app_header;

@pragma dont_trim
@pragma pa_header_union ingress to_stage_4
metadata ipfix_exported_stats_metadata_t ipfix_exported_permit_stats1;
@pragma dont_trim
@pragma pa_header_union ingress to_stage_6
metadata ipfix_exported_stats_metadata_t ipfix_exported_permit_stats2;
@pragma dont_trim
@pragma pa_header_union ingress common_t1_s2s
metadata ipfix_exported_stats_metadata_t ipfix_exported_drop_stats;

@pragma dont_trim
metadata ipfix_record_header_t ipfix_record_header;
@pragma dont_trim
metadata ipfix_record_ipv4_t ipfix_record_ipv4;
@pragma dont_trim
metadata ipfix_record_ip_t ipfix_record_ip;
@pragma dont_trim
metadata ipfix_record_common_t ipfix_record_common;
@pragma dont_trim
metadata ipfix_record_ipv6_t ipfix_record_ipv6;
@pragma dont_trim
metadata ipfix_record_nonip_t ipfix_record_nonip;
@pragma dont_trim
metadata ipfix_pad_t ipfix_pad;

@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd3;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd4;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd5;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem_cmd6;

@pragma dont_trim
@pragma pa_header_union ingress phv2mem_cmd1
metadata dma_cmd_phv2pkt_t phv2pkt_cmd1;
@pragma dont_trim
@pragma pa_header_union ingress phv2mem_cmd2
metadata dma_cmd_mem2pkt_t mem2pkt_cmd;

action ipfix_start(rsvd, cos_a, cos_b, cos_sel, eval_last, host_rings,
                   total_rings, pid, pindex, cindex, pktaddr, pktsize,
                   seq_no, domain_id, ipfix_hdr_offset, next_record_offset,
                   flow_hash_table_type, flow_hash_index_next,
                   flow_hash_index_max, flow_hash_overflow_index_max,
                   export_time) {
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
    modify_field(qstate_metadata.pktaddr, pktaddr);
    modify_field(qstate_metadata.pktsize, pktsize);
    modify_field(qstate_metadata.seq_no, seq_no);
    modify_field(qstate_metadata.domain_id, domain_id);
    modify_field(qstate_metadata.ipfix_hdr_offset, ipfix_hdr_offset);
    modify_field(qstate_metadata.next_record_offset, next_record_offset);
    modify_field(qstate_metadata.flow_hash_table_type, flow_hash_table_type);
    modify_field(qstate_metadata.flow_hash_index_next, flow_hash_index_next);
    modify_field(qstate_metadata.flow_hash_index_max, flow_hash_index_max);
    modify_field(qstate_metadata.flow_hash_overflow_index_max,
                 flow_hash_overflow_index_max);
    modify_field(qstate_metadata.export_time, export_time);

    modify_field(ipfix_metadata.qstate_addr, p4_txdma_intr.qstate_addr);
    modify_field(ipfix_metadata.export_id, p4_txdma_intr.qid);
}

action ipfix_flow_hash() {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
}

action ipfix_flow_info() {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.flow_index, ipfix_metadata.flow_index);
    modify_field(scratch_metadata.export_id, ipfix_metadata.export_id);
    modify_field(scratch_metadata.export_en, ipfix_metadata.export_en);
}

action ipfix_session_state() {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.flow_index, ipfix_metadata.flow_index);
    modify_field(scratch_metadata.session_index, ipfix_metadata.session_index);
    modify_field(scratch_metadata.flow_role, ipfix_metadata.flow_role);
}

action ipfix_flow_stats() {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.qstate_addr, ipfix_metadata.qstate_addr);
    modify_field(scratch_metadata.counter64, ipfix_exported_permit_stats1.pkts);
    modify_field(scratch_metadata.counter64, ipfix_exported_permit_stats1.byts);
    modify_field(scratch_metadata.counter64, ipfix_exported_drop_stats.pkts);
    modify_field(scratch_metadata.counter64, ipfix_exported_drop_stats.byts);
}

action ipfix_read_exported_flow_stats(permit_packets, permit_bytes,
                                      drop_packets, drop_bytes) {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.counter64, permit_packets);
    modify_field(scratch_metadata.counter64, permit_bytes);
    modify_field(scratch_metadata.counter64, drop_packets);
    modify_field(scratch_metadata.counter64, drop_bytes);
}

action ipfix_update_exported_flow_stats(permit_packets, permit_bytes,
                                        drop_packets, drop_bytes) {
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.counter64, ipfix_exported_permit_stats2.pkts);
    modify_field(scratch_metadata.counter64, ipfix_exported_permit_stats2.byts);
    modify_field(scratch_metadata.counter64, ipfix_exported_drop_stats.pkts);
    modify_field(scratch_metadata.counter64, ipfix_exported_drop_stats.byts);
    modify_field(scratch_metadata.counter64, permit_packets);
    modify_field(scratch_metadata.counter64, permit_bytes);
    modify_field(scratch_metadata.counter64, drop_packets);
    modify_field(scratch_metadata.counter64, drop_bytes);
}

action ipfix_create_record(pc, rsvd, cos_a, cos_b, cos_sel, eval_last,
                           host_rings, total_rings, pid, pindex, cindex,
                           pktaddr, pktsize, seq_no, domain_id,
                           ipfix_hdr_offset, next_record_offset,
                           flow_hash_table_type, flow_hash_index_next,
                           flow_hash_index_max, flow_hash_overflow_index_max,
                           export_time) {
    modify_field(qstate_metadata.pc, pc);
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
    modify_field(qstate_metadata.pktaddr, pktaddr);
    modify_field(qstate_metadata.pktsize, pktsize);
    modify_field(qstate_metadata.seq_no, seq_no);
    modify_field(qstate_metadata.domain_id, domain_id);
    modify_field(qstate_metadata.ipfix_hdr_offset, ipfix_hdr_offset);
    modify_field(qstate_metadata.next_record_offset, next_record_offset);
    modify_field(qstate_metadata.flow_hash_table_type, flow_hash_table_type);
    modify_field(qstate_metadata.flow_hash_index_next, flow_hash_index_next);
    modify_field(qstate_metadata.flow_hash_index_max, flow_hash_index_max);
    modify_field(qstate_metadata.flow_hash_overflow_index_max,
                 flow_hash_overflow_index_max);
    modify_field(qstate_metadata.export_time, export_time);

    modify_field(scratch_metadata.flow_type, ipfix_metadata.flow_type);
    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.export_id, ipfix_metadata.export_id);
    modify_field(scratch_metadata.qstate_addr, ipfix_metadata.qstate_addr);
    modify_field(scratch_metadata.flow_hash_table_type,
                 ipfix_t0_metadata.flow_hash_table_type);
    modify_field(scratch_metadata.flow_hash_index_next,
                 ipfix_t0_metadata.flow_hash_index_next);
    modify_field(scratch_metadata.qstate_addr,
                 ipfix_t0_metadata.exported_stats_addr);
}

action ipfix_export_packet(pc, rsvd, cos_a, cos_b, cos_sel, eval_last,
                           host_rings, total_rings, pid, pindex, cindex,
                           pktaddr, pktsize, seq_no, domain_id,
                           ipfix_hdr_offset, next_record_offset,
                           flow_hash_table_type, flow_hash_index_next,
                           flow_hash_index_max, flow_hash_overflow_index_max,
                           export_time) {
    modify_field(qstate_metadata.pc, pc);
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
    modify_field(qstate_metadata.pktaddr, pktaddr);
    modify_field(qstate_metadata.pktsize, pktsize);
    modify_field(qstate_metadata.seq_no, seq_no);
    modify_field(qstate_metadata.domain_id, domain_id);
    modify_field(qstate_metadata.ipfix_hdr_offset, ipfix_hdr_offset);
    modify_field(qstate_metadata.next_record_offset, next_record_offset);
    modify_field(qstate_metadata.flow_hash_table_type, flow_hash_table_type);
    modify_field(qstate_metadata.flow_hash_index_next, flow_hash_index_next);
    modify_field(qstate_metadata.flow_hash_index_max, flow_hash_index_max);
    modify_field(qstate_metadata.flow_hash_overflow_index_max,
                 flow_hash_overflow_index_max);
    modify_field(qstate_metadata.export_time, export_time);

    modify_field(scratch_metadata.scan_complete, ipfix_metadata.scan_complete);
    modify_field(scratch_metadata.export_id, ipfix_metadata.export_id);
    modify_field(scratch_metadata.qstate_addr, ipfix_metadata.qstate_addr);
}
