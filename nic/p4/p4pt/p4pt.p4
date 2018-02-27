
#include "p4pt_metadata.p4"
#include "../common-p4+/common_rxdma_dummy.p4"

/* 
 * k and d vectors for various tables in all stages
 * p4pt for now only use 5 stages and one table across all of them
 * following needs adjustment if we launch multiple lookups in any stage
 */
//#define common_p4plus_stage0_app_header_table	p4pt_start
#define rx_table_s1_t0          p4pt_update_tcb
#define rx_table_s2_t0          p4pt_update_tcb_rec
#define rx_table_s3_t0          p4pt_update_tcb_stats
#define rx_table_s4_t0          p4pt_update_read_latency_distribution
#define rx_table_s5_t0          p4pt_update_write_latency_distribution

#define common_p4plus_stage0_app_header_table_action_dummy	p4pt_start
#define rx_table_s1_t0_action	p4pt_update_tcb
#define rx_table_s2_t0_action	p4pt_update_tcb_rec
#define rx_table_s3_t0_action	p4pt_update_tcb_stats
#define rx_table_s4_t0_action	p4pt_update_read_latency_distribution
#define rx_table_s5_t0_action	p4pt_update_write_latency_distribution

#include "../common-p4+/common_rxdma.p4"

/* 
 * key vectors
 */ 

// custom app headers for every protocol that needs parsing
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4pt_iscsi_app_header_t p4pt_iscsi_app_header;
@pragma scratch_metadata
metadata p4pt_iscsi_app_header_t p4pt_scratch_app;

// @pragma dont_trim
@pragma pa_header_union ingress common_global
metadata p4pt_global_t p4pt_global;
@pragma scratch_metadata
metadata p4pt_global_t p4pt_global_scratch;

// @pragma dont_trim
@pragma pa_header_union ingress common_t0_s2s
metadata p4pt_s2s_t p4pt_s2s;
@pragma scratch_metadata
metadata p4pt_s2s_t p4pt_s2s_scratch;


/*
 * data vectors
 */

// stage 1
@pragma scratch_metadata
metadata p4pt_tcb_t p4pt_tcb;

// stage 2
@pragma scratch_metadata
metadata p4pt_tcb_iscsi_rec_t p4pt_tcb_iscsi_rec;

// stage 3
@pragma scratch_metadata
metadata p4pt_tcb_iscsi_stats_t p4pt_tcb_iscsi_stats;

// stage 4
@pragma scratch_metadata
metadata p4pt_tcb_iscsi_read_latency_distribution_t p4pt_tcb_iscsi_read_latency_distribution;

// stage 5
@pragma scratch_metadata
metadata p4pt_tcb_iscsi_write_latency_distribution_t p4pt_tcb_iscsi_write_latency_distribution;


#define COPY_GLOBAL_KEY \
    modify_field(p4pt_global_scratch.qid, p4pt_global.qid); \
    modify_field(p4pt_global_scratch.p4pt_idx, p4pt_global.p4pt_idx); \
    modify_field(p4pt_global_scratch.rec_idx, p4pt_global.rec_idx); \
    modify_field(p4pt_global_scratch.latency, p4pt_global.latency); \
    modify_field(p4pt_global_scratch.exit, p4pt_global.exit); \
    modify_field(p4pt_global_scratch.reserved, p4pt_global.reserved);

#define COPY_S2S_KEY \
    modify_field(p4pt_s2s_scratch.req, p4pt_s2s.req); \
    modify_field(p4pt_s2s_scratch.resp, p4pt_s2s.resp); \
    modify_field(p4pt_s2s_scratch.read, p4pt_s2s.read); \
    modify_field(p4pt_s2s_scratch.write, p4pt_s2s.write); \
    modify_field(p4pt_s2s_scratch.reserved, p4pt_s2s.reserved); \
    modify_field(p4pt_s2s_scratch.status, p4pt_s2s.status); \
    modify_field(p4pt_s2s_scratch.data_length, p4pt_s2s.data_length); \
    modify_field(p4pt_s2s_scratch.lun, p4pt_s2s.lun); \
    modify_field(p4pt_s2s_scratch.tag_id, p4pt_s2s.tag_id);

action p4pt_start () {
    modify_field(p4pt_scratch_app.p4plus_app_id, p4pt_iscsi_app_header.p4plus_app_id);
    modify_field(p4pt_scratch_app.table0_valid, p4pt_iscsi_app_header.table0_valid);
    modify_field(p4pt_scratch_app.table1_valid, p4pt_iscsi_app_header.table1_valid);
    modify_field(p4pt_scratch_app.table2_valid, p4pt_iscsi_app_header.table2_valid);
    modify_field(p4pt_scratch_app.table3_valid, p4pt_iscsi_app_header.table3_valid);
    modify_field(p4pt_scratch_app.p4pt_idx, p4pt_iscsi_app_header.p4pt_idx);
    modify_field(p4pt_scratch_app.payload_len, p4pt_iscsi_app_header.payload_len);
    modify_field(p4pt_scratch_app.pad, p4pt_iscsi_app_header.pad);
    modify_field(p4pt_scratch_app.flow_role, p4pt_iscsi_app_header.flow_role);
    modify_field(p4pt_scratch_app.flow_dir, p4pt_iscsi_app_header.flow_dir);
    modify_field(p4pt_scratch_app.skip0, p4pt_iscsi_app_header.skip0);
    modify_field(p4pt_scratch_app.opcode, p4pt_iscsi_app_header.opcode);
    modify_field(p4pt_scratch_app.skip1, p4pt_iscsi_app_header.skip1);
    modify_field(p4pt_scratch_app.cmd_read, p4pt_iscsi_app_header.cmd_read);
    modify_field(p4pt_scratch_app.cmd_write, p4pt_iscsi_app_header.cmd_write);
    modify_field(p4pt_scratch_app.skip2, p4pt_iscsi_app_header.skip2);
    modify_field(p4pt_scratch_app.resp_status, p4pt_iscsi_app_header.resp_status);
    modify_field(p4pt_scratch_app.skip3, p4pt_iscsi_app_header.skip3);
    modify_field(p4pt_scratch_app.cmd_lun, p4pt_iscsi_app_header.cmd_lun);
    modify_field(p4pt_scratch_app.tag_id, p4pt_iscsi_app_header.tag_id);
    modify_field(p4pt_scratch_app.cmd_data_length, p4pt_iscsi_app_header.cmd_data_length);
    modify_field(p4pt_scratch_app.skip4, p4pt_iscsi_app_header.skip4);
    modify_field(p4pt_scratch_app.cmd_scsi_cdb_op, p4pt_iscsi_app_header.cmd_scsi_cdb_op);
}

// params = fields from p4pt_tcb_t
action p4pt_update_tcb(next_idx, base_addr, rec_size, reserved0,
                       rec_key0, rec_key1, rec_key2, rec_key3, rec_key4,
                       rec_key5, rec_key6, rec_key7, rec_key8, rec_key9) {
    // populate k+i
    COPY_GLOBAL_KEY
    COPY_S2S_KEY

    // populate d vector
    modify_field(p4pt_tcb.next_idx, next_idx);
    modify_field(p4pt_tcb.base_addr, base_addr);
    modify_field(p4pt_tcb.rec_size, rec_size);
    modify_field(p4pt_tcb.reserved0, reserved0);
    modify_field(p4pt_tcb.rec_key0, rec_key0);
    modify_field(p4pt_tcb.rec_key0, rec_key1);
    modify_field(p4pt_tcb.rec_key2, rec_key2);
    modify_field(p4pt_tcb.rec_key3, rec_key3);
    modify_field(p4pt_tcb.rec_key4, rec_key4);
    modify_field(p4pt_tcb.rec_key5, rec_key5);
    modify_field(p4pt_tcb.rec_key6, rec_key6);
    modify_field(p4pt_tcb.rec_key7, rec_key7);
    modify_field(p4pt_tcb.rec_key8, rec_key8);
    modify_field(p4pt_tcb.rec_key9, rec_key9);
}

// params = fields from p4pt_iscsi_rec_t
action p4pt_update_tcb_rec(reserved, read, write, status, tag_id,
                           lun, req_timestamp, data_length, reserved1) {
    // populate k+i
    COPY_GLOBAL_KEY
    COPY_S2S_KEY

    // populate d vector
    modify_field(p4pt_tcb_iscsi_rec.reserved, reserved);
    modify_field(p4pt_tcb_iscsi_rec.read, read);
    modify_field(p4pt_tcb_iscsi_rec.write, write);
    modify_field(p4pt_tcb_iscsi_rec.status, status);
    modify_field(p4pt_tcb_iscsi_rec.tag_id, tag_id);
    modify_field(p4pt_tcb_iscsi_rec.lun, lun);
    modify_field(p4pt_tcb_iscsi_rec.req_timestamp, req_timestamp);
    modify_field(p4pt_tcb_iscsi_rec.data_length, data_length);
    modify_field(p4pt_tcb_iscsi_rec.reserved1, reserved1);
}

// params = fields from p4pt_tcb_iscsi_stats_t
action p4pt_update_tcb_stats(read_reqs, write_reqs, read_bytes, write_bytes,
                             read_errors, write_errors, total_read_latency,
                             total_read_resps, total_write_latency,
                             total_write_resps, reserved) {
    // populate k+i
    COPY_GLOBAL_KEY
    COPY_S2S_KEY

    // populate d vector
    modify_field(p4pt_tcb_iscsi_stats.read_reqs, read_reqs);
    modify_field(p4pt_tcb_iscsi_stats.write_reqs, write_reqs);
    modify_field(p4pt_tcb_iscsi_stats.read_bytes, read_bytes);
    modify_field(p4pt_tcb_iscsi_stats.write_bytes, write_bytes);
    modify_field(p4pt_tcb_iscsi_stats.read_errors, read_errors);
    modify_field(p4pt_tcb_iscsi_stats.write_errors, write_errors);
    modify_field(p4pt_tcb_iscsi_stats.total_read_latency, total_read_latency);
    modify_field(p4pt_tcb_iscsi_stats.total_read_resps, total_read_resps);
    modify_field(p4pt_tcb_iscsi_stats.total_write_latency, total_write_latency);
    modify_field(p4pt_tcb_iscsi_stats.total_write_resps, total_write_resps);
}

// p4pt_tcb_iscsi_read_latency_distribution_t
action p4pt_update_read_latency_distribution(range0, range1, range2, range3,
                                             range4, range5, range6, range7,
                                             range8, range9, range10, range11,
                                             range12, range13, range14, range15) {
    // populate k+i
    COPY_GLOBAL_KEY
    COPY_S2S_KEY

    // populate d vector
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range0, range0);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range1, range1);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range2, range2);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range3, range3);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range4, range4);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range5, range5);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range6, range6);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range7, range7);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range8, range8);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range9, range9);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range10, range10);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range11, range11);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range12, range12);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range13, range13);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range14, range14);
    modify_field(p4pt_tcb_iscsi_read_latency_distribution.range15, range15);
}

action p4pt_update_write_latency_distribution(range0, range1, range2, range3,
                                             range4, range5, range6, range7,
                                             range8, range9, range10, range11,
                                             range12, range13, range14, range15) {
    // populate k+i
    COPY_GLOBAL_KEY
    COPY_S2S_KEY

    // populate d vector
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range0, range0);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range1, range1);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range2, range2);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range3, range3);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range4, range4);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range5, range5);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range6, range6);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range7, range7);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range8, range8);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range9, range9);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range10, range10);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range11, range11);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range12, range12);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range13, range13);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range14, range14);
    modify_field(p4pt_tcb_iscsi_write_latency_distribution.range15, range15);
}
