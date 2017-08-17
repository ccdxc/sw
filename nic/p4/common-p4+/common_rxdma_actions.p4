
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

action common_p4plus_stage0_app_header_table_action_dummy() {
}
