
// Define all dummy actions before including the P4 file
#define rx_table_s7_t3_action	rx_table_dummy_action
#define rx_table_s7_t2_action	rx_table_dummy_action
#define rx_table_s7_t1_action	rx_table_dummy_action
#define rx_table_s7_t0_action	rx_table_dummy_action
#define rx_table_s6_t3_action	rx_table_dummy_action
#define rx_table_s6_t2_action	rx_table_dummy_action
#define rx_table_s6_t1_action	rx_table_dummy_action
#define rx_table_s6_t0_action	rx_table_dummy_action
#define rx_table_s5_t3_action	rx_table_dummy_action
#define rx_table_s5_t2_action	rx_table_dummy_action
#define rx_table_s5_t1_action	rx_table_dummy_action
#define rx_table_s5_t0_action	rx_table_dummy_action
#define rx_table_s4_t3_action	rx_table_dummy_action
#define rx_table_s4_t2_action	rx_table_dummy_action
#define rx_table_s4_t1_action	rx_table_dummy_action
#define rx_table_s4_t0_action	rx_table_dummy_action
#define rx_table_s3_t3_action	rx_table_dummy_action
#define rx_table_s3_t2_action	rx_table_dummy_action
#define rx_table_s3_t1_action	rx_table_dummy_action
#define rx_table_s3_t0_action	rx_table_dummy_action
#define rx_table_s2_t3_action	rx_table_dummy_action
#define rx_table_s2_t2_action	rx_table_dummy_action
#define rx_table_s2_t1_action	rx_table_dummy_action
#define rx_table_s2_t0_action	rx_table_dummy_action
#define rx_table_s1_t3_action	rx_table_dummy_action
#define rx_table_s1_t2_action	rx_table_dummy_action
#define rx_table_s1_t1_action	rx_table_dummy_action
#define rx_table_s1_t0_action	rx_table_dummy_action
#define rx_table_s0_t3_action	rx_table_dummy_action
#define rx_table_s0_t2_action	rx_table_dummy_action
#define rx_table_s0_t1_action	rx_table_dummy_action
#define rx_table_s0_t0_action	rx_table_dummy_action


#include "common_rxdma.p4"

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata1;

// dummy action function 
action rx_table_dummy_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata1)
}

action common_p4plus_stage0_app_header_table_action() {
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);
    modify_field(scratch_app.app_type, app_header.app_type);
    modify_field(scratch_app.table0_valid, app_header.table0_valid);
    modify_field(scratch_app.table1_valid, app_header.table1_valid);
    modify_field(scratch_app.table2_valid, app_header.table2_valid);
    modify_field(scratch_app.table3_valid, app_header.table3_valid);
    modify_field(scratch_app.gft_flow_id, app_header.gft_flow_id);
    modify_field(scratch_app.app_data0, app_header.app_data0);
    modify_field(scratch_app.app_data1, app_header.app_data1);
}

