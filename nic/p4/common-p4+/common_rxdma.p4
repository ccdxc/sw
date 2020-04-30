#include "./common_headers.p4"
#include "nic/p4/common/defines.h"

// Please do not modify the order of the below PHV definitions lines - rameshp
@pragma dont_trim
metadata cap_phv_intr_global_t p4_intr_global;
@pragma dont_trim
metadata cap_phv_intr_p4_t     p4_intr;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t p4_rxdma_intr;

// may have to put a pad to round of  the above 2 headers + something to 512 bits
@pragma dont_trim
metadata p4_2_p4plus_app_header_t app_header;

@pragma dont_trim
metadata p4_2_p4plus_ext_app_header_t ext_app_header;

// to_stage PHV
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_0;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_1;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_2;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_3;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_4;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_5;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_6;
@pragma dont_trim
metadata p4plus_common_to_stage_t to_stage_7;

@pragma dont_trim
metadata p4plus_common_global_t common_global;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te0_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t0_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te1_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t1_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te2_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t2_s2s;
@pragma dont_trim
metadata p4plus_common_raw_table_engine_phv_t common_te3_phv;
@pragma dont_trim
metadata p4plus_common_s2s_t common_t3_s2s;

header_type rxdma_common_pad_t {
    fields {
        rxdma_common_pad : 96;
    }
}

@pragma dont_trim
metadata rxdma_common_pad_t rxdma_common_pad;

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata0;

@pragma scratch_metadata
metadata p4plus_common_raw_table_engine_phv_t scratch_te0;

@pragma scratch_metadata
metadata p4plus_common_global_t global_scratch;

@pragma scratch_metadata
metadata p4plus_common_s2s_t s2s_scratch0;

@pragma scratch_metadata
metadata cap_phv_intr_global_t p4_intr_global_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_p4_t     p4_intr_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_rxdma_t  p4_rxdma_intr_scratch;

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata2;

@pragma dont_trim
@pragma scratch_metadata
metadata rx_rdma_scratch_metadata_t scratch_rdma;

@pragma dont_trim
@pragma scratch_metadata
metadata p4_2_p4plus_app_header_t scratch_app;

@pragma dont_trim
@pragma scratch_metadata
metadata p4_2_p4plus_ext_app_header_t scratch_ext_app;

@pragma scratch_metadata
metadata p4plus_common_raw_table_engine_phv_t te0_scratch;
@pragma scratch_metadata
metadata p4plus_common_raw_table_engine_phv_t te1_scratch;
@pragma scratch_metadata
metadata p4plus_common_raw_table_engine_phv_t te2_scratch;
@pragma scratch_metadata
metadata p4plus_common_raw_table_engine_phv_t te3_scratch;

@pragma scratch_metadata
metadata p4plus_common_to_stage_t te_scratch;

#define SET_RAW_TABLE0_SCRATCH \
    modify_field(te0_scratch.table_pc, common_te0_phv.table_pc); \
    modify_field(te0_scratch.table_raw_table_size, common_te0_phv.table_raw_table_size); \
    modify_field(te0_scratch.table_lock_en, common_te0_phv.table_lock_en); \
    modify_field(te0_scratch.table_addr, common_te0_phv.table_addr);

#define SET_RAW_TABLE1_SCRATCH \
    modify_field(te1_scratch.table_pc, common_te1_phv.table_pc); \
    modify_field(te1_scratch.table_raw_table_size, common_te1_phv.table_raw_table_size); \
    modify_field(te1_scratch.table_lock_en, common_te1_phv.table_lock_en); \
    modify_field(te1_scratch.table_addr, common_te1_phv.table_addr);

#define SET_RAW_TABLE2_SCRATCH \
    modify_field(te2_scratch.table_pc, common_te2_phv.table_pc); \
    modify_field(te2_scratch.table_raw_table_size, common_te2_phv.table_raw_table_size); \
    modify_field(te2_scratch.table_lock_en, common_te2_phv.table_lock_en); \
    modify_field(te2_scratch.table_addr, common_te2_phv.table_addr);

#define SET_RAW_TABLE3_SCRATCH \
    modify_field(te3_scratch.table_pc, common_te3_phv.table_pc); \
    modify_field(te3_scratch.table_raw_table_size, common_te3_phv.table_raw_table_size); \
    modify_field(te3_scratch.table_lock_en, common_te3_phv.table_lock_en); \
    modify_field(te3_scratch.table_addr, common_te3_phv.table_addr);

#define SET_GLOBAL_SCRATCH \
    modify_field(global_scratch.global_data, common_global.global_data);

#define SET_S2S_SCRATCH0 \
    modify_field(s2s_scratch0.s2s_data, common_t0_s2s.s2s_data);

#define SET_S2S_SCRATCH1 \
    modify_field(s2s_scratch0.s2s_data, common_t1_s2s.s2s_data);

#define SET_S2S_SCRATCH2 \
    modify_field(s2s_scratch0.s2s_data, common_t2_s2s.s2s_data);

#define SET_S2S_SCRATCH3 \
    modify_field(s2s_scratch0.s2s_data, common_t3_s2s.s2s_data);

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata1;

// dummy action function
action rx_table_dummy_action(data0, data1, data2, data3, data4, data5,
                             data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata1)
}

action common_p4plus_stage0_app_header_table_action_dummy_default() {
}

//stage 7 action functions
action rx_table_s7_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

//stage 6 action functions
action rx_table_s6_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

//stage 5 action functions
action rx_table_s5_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

//stage 4 action functions
action rx_table_s4_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

//stage 3 action functions
action rx_table_s3_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}


//stage 2 action functions
action rx_table_s2_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

//stage 1 action functions
action rx_table_s1_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

//stage 0 action functions
// add d-arguments here if required
// the below action function code is just to generate a fixed 512 bit d-vector.
// each program may have its own definition of k, d vectors.
// referencing them makes sure they are not "removed" by NCC
action rx_table_s0_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te3
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);
}

action rx_table_s0_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te2
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);
}

action rx_table_s0_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te1
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);
}

action rx_table_s0_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te0
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);
}

// stage 7
@pragma stage 7
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
@pragma memory_only
table rx_table_s7_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s7_t3_action;
        rx_table_s7_t3_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s7_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s7_t2_action;
        rx_table_s7_t2_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s7_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s7_t1_action;
        rx_table_s7_t1_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s7_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s7_t0_action;
        rx_table_s7_t0_cfg_action;
        rx_table_s7_t0_action1;
    }
}

// stage 6
@pragma stage 6
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s6_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s6_t3_action;
        rx_table_s6_t3_cfg_action;
    }
}

@pragma stage 6
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s6_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s6_t2_action;
        rx_table_s6_t2_cfg_action;
    }
}

@pragma stage 6
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s6_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s6_t1_action;
        rx_table_s6_t1_cfg_action;
        rx_table_s6_t1_action1;
        rx_table_s6_t1_action2;
    }
}

@pragma stage 6
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s6_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s6_t0_action;
        rx_table_s6_t0_cfg_action;
    }
}

// stage 5
@pragma stage 5
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s5_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s5_t3_action;
        rx_table_s5_t3_cfg_action;
    }
}

@pragma stage 5
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s5_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s5_t2_action;
        rx_table_s5_t2_cfg_action;
        rx_table_s5_t2_action1;
    }
}

@pragma stage 5
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s5_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s5_t1_action;
        rx_table_s5_t1_cfg_action;
        rx_table_s5_t1_action1;
        rx_table_s5_t1_action2;
    }
}

@pragma stage 5
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s5_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s5_t0_action;
        rx_table_s5_t0_action1;
        rx_table_s5_t0_action2;
        rx_table_s5_t0_action3;
        rx_table_s5_t0_cfg_action;
    }
}

// stage 4
@pragma stage 4
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s4_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s4_t3_action;
        rx_table_s4_t3_action1;
        rx_table_s4_t3_action2;
        rx_table_s4_t3_cfg_action;
    }
}

@pragma stage 4
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s4_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s4_t2_action;
        rx_table_s4_t2_action1;
        rx_table_s4_t2_action2;
        rx_table_s4_t2_action3;
        rx_table_s4_t2_cfg_action;
    }
}

@pragma stage 4
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s4_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s4_t1_action;
        rx_table_s4_t1_cfg_action;
        rx_table_s4_t1_action1;
        rx_table_s4_t1_action2;
    }
}

@pragma stage 4
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s4_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s4_t0_action;
        rx_table_s4_t0_cfg_action;
        rx_table_s4_t0_action1;
        rx_table_s4_t0_action2;
    }
}

// stage 3
@pragma stage 3
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s3_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s3_t3_action;
        rx_table_s3_t3_action1;
        rx_table_s3_t3_action2;
        rx_table_s3_t3_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s3_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s3_t2_action;
        rx_table_s3_t2_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s3_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s3_t1_action;
        rx_table_s3_t1_action1;
        rx_table_s3_t1_action2;
        rx_table_s3_t1_action3;
        rx_table_s3_t1_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s3_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s3_t0_action;
        rx_table_s3_t0_action1;
        rx_table_s3_t0_action2;
        rx_table_s3_t0_cfg_action;
    }
}

// stage 2
@pragma stage 2
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s2_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s2_t3_action;
        rx_table_s2_t3_cfg_action;
    }
}

@pragma stage 2
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s2_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s2_t2_action;
        rx_table_s2_t2_cfg_action;
    }
}
@pragma stage 2
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s2_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s2_t1_action;
        rx_table_s2_t1_cfg_action;
    }
}

@pragma stage 2
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s2_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s2_t0_action;
        rx_table_s2_t0_action1;
        rx_table_s2_t0_action2;
        rx_table_s2_t0_action3;
        rx_table_s2_t0_action4;
        rx_table_s2_t0_cfg_action;
    }
}

// stage 1
@pragma stage 1
@pragma raw_table common_te3_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s1_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s1_t3_action;
        rx_table_s1_t3_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te2_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s1_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s1_t2_action;
        rx_table_s1_t2_action1;
        rx_table_s1_t2_action2;
        rx_table_s1_t2_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te1_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s1_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s1_t1_action;
        rx_table_s1_t1_action1;
        rx_table_s1_t1_action2;
        rx_table_s1_t1_action3;
        rx_table_s1_t1_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te0_phv.table_pc
@pragma table_write // lock_en_raw=1
table rx_table_s1_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s1_t0_action;
        rx_table_s1_t0_action1;
        rx_table_s1_t0_action2;
        rx_table_s1_t0_action3;
        rx_table_s1_t0_action4;
        rx_table_s1_t0_cfg_action;
    }
}

// stage 0
@pragma stage 0
@pragma raw_table common_te3_phv.table_pc
table rx_table_s0_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        rx_table_s0_t3_action;
        rx_table_s0_t3_cfg_action;
    }
}

@pragma stage 0
@pragma raw_table common_te2_phv.table_pc
table rx_table_s0_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        rx_table_s0_t2_action;
        rx_table_s0_t2_cfg_action;
    }
}

@pragma stage 0
@pragma raw_table common_te1_phv.table_pc
table rx_table_s0_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        rx_table_s0_t1_action;
        rx_table_s0_t1_cfg_action;
    }
}

@pragma stage 0
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table rx_table_s0_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        rx_table_s0_t0_action;
        rx_table_s0_t0_cfg_action;
    }
}

parser start {
    return ingress;
}

action rx_stage0_load_rdma_params(rdma_en_qtype_mask,
                                  pt_base_addr_page_id,
                                  log_num_pt_entries,
                                  log_num_kt_entries,
                                  log_num_dcqcn_profiles,
                                  log_num_ah_entries,
                                  cqcb_base_addr_hi,
                                  sqcb_base_addr_hi,
                                  rqcb_base_addr_hi,
                                  log_num_cq_entries,
                                  log_num_sq_entries,
                                  log_num_rq_entries,
                                  prefetch_base_addr_page_id,
                                  log_prefetch_buf_size,
                                  sq_qtype,
                                  rq_qtype,
                                  aq_qtype,
                                  reserved) {
    if (p4_rxdma_intr.qid > 0) {
        modify_field(scratch_rdma.reserved, reserved);
    }

    if (p4_intr.recirc == 0) {

        if (((1 << p4_rxdma_intr.qtype) & rdma_en_qtype_mask)  != 0) {

            modify_field(scratch_ext_app.app_data3, ext_app_header.app_data3);

            modify_field(scratch_rdma.rdma_en_qtype_mask, rdma_en_qtype_mask);
            modify_field(scratch_rdma.pt_base_addr_page_id, pt_base_addr_page_id);
            modify_field(scratch_rdma.log_num_pt_entries, log_num_pt_entries);
            modify_field(scratch_rdma.log_num_kt_entries, log_num_kt_entries);
            modify_field(scratch_rdma.log_num_dcqcn_profiles, log_num_dcqcn_profiles);
            modify_field(scratch_rdma.log_num_ah_entries, log_num_ah_entries);
            modify_field(scratch_rdma.cqcb_base_addr_hi, cqcb_base_addr_hi);
            modify_field(scratch_rdma.sqcb_base_addr_hi, sqcb_base_addr_hi);
            modify_field(scratch_rdma.rqcb_base_addr_hi, rqcb_base_addr_hi);
            modify_field(scratch_rdma.log_num_cq_entries, log_num_cq_entries);
            modify_field(scratch_rdma.log_num_sq_entries, log_num_sq_entries);
            modify_field(scratch_rdma.log_num_rq_entries, log_num_rq_entries);
            modify_field(scratch_rdma.prefetch_base_addr_page_id,
                         prefetch_base_addr_page_id);
            modify_field(scratch_rdma.log_prefetch_buf_size,
                         log_prefetch_buf_size);
            modify_field(scratch_rdma.sq_qtype, sq_qtype);
            modify_field(scratch_rdma.rq_qtype, rq_qtype);
            modify_field(scratch_rdma.aq_qtype, aq_qtype);
            modify_field(scratch_rdma.reserved, reserved);
        }
    }
}

@pragma stage 0
table rx_stage0_load_rdma_params {
    reads {
        p4_intr_global.lif : exact;
    }
    actions {
        rx_stage0_load_rdma_params;
        rx_stage0_load_rdma_params_dummy1;
        rx_stage0_load_rdma_params_dummy2;
        rx_stage0_load_rdma_params_dummy3;
        rx_stage0_load_rdma_params_dummy4;
        rx_stage0_load_rdma_params_dummy5;
    }
    size : LIF_TABLE_SIZE;
}

action common_p4plus_stage0_app_header_table_action(data0, data1, data2, data3,
                                                    data4, data5, data6) {
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_intr_scratch.recirc_count, p4_intr.recirc_count);
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
    modify_field(scratch_app.app_data2, app_header.app_data2);
    SCRATCH_METADATA_INIT_7(scratch_metadata0)
}

#define COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY \
        p4_rxdma_intr.qstate_addr : exact;

#define COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS \
        common_p4plus_stage0_app_header_table_action; \
        common_p4plus_stage0_app_header_table_action_dummy; \
        common_p4plus_stage0_app_header_table_action_dummy1; \
        common_p4plus_stage0_app_header_table_action_dummy2; \
        common_p4plus_stage0_app_header_table_action_dummy3; \
        common_p4plus_stage0_app_header_table_action_dummy4; \
        common_p4plus_stage0_app_header_table_action_dummy5; \
        common_p4plus_stage0_app_header_table_action_dummy5; \
        common_p4plus_stage0_app_header_table_action_dummy6; \
        common_p4plus_stage0_app_header_table_action_dummy7; \
        common_p4plus_stage0_app_header_table_action_dummy8; \
        common_p4plus_stage0_app_header_table_action_dummy9; \
        common_p4plus_stage0_app_header_table_action_dummy10; \
        common_p4plus_stage0_app_header_table_action_dummy11; \
        common_p4plus_stage0_app_header_table_action_dummy12; \
        common_p4plus_stage0_app_header_table_action_dummy13; \
        common_p4plus_stage0_app_header_table_action_dummy14; \
        common_p4plus_stage0_app_header_table_action_dummy15; \
        common_p4plus_stage0_app_header_table_action_dummy16; \
        common_p4plus_stage0_app_header_table_action_dummy17; \
        common_p4plus_stage0_app_header_table_action_dummy18; \
        common_p4plus_stage0_app_header_table_action_dummy19;

@pragma stage 0
@pragma raw_index_table
@pragma table_write
table common_p4plus_stage0_app_header_table {
    reads {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY
    }
    actions {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS
    }
}

@pragma stage 0
@pragma raw_index_table
//@pragma table_write
table common_p4plus_stage0_app_header_ext_table {
    reads {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY
    }
    actions {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS
    }
}

@pragma stage 0
@pragma raw_index_table
@pragma table_write
table common_p4plus_stage0_app_header_table_offset_64 {
    reads {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY
    }
    actions {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS
    }
}

@pragma stage 0
@pragma raw_index_table
//@pragma table_write
table common_p4plus_stage0_app_header_table_ext_offset_64 {
    reads {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY
    }
    actions {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS
    }
}

// d-vector for rss_params table
header_type eth_rx_rss_params {
    fields {
        rss_type : 8;
        rss_key : 320;
        debug : 1;
        rsvd : 7;
    }
}

@pragma scratch_metadata
metadata eth_rx_rss_params eth_rx_rss_params_scratch;

header_type p4_to_p4plus_classic_nic_header_ext_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        flow_id             : 24;
        vlan_pcp            : 3;
        vlan_dei            : 1;
        vlan_vid            : 12;
        encap_pkt           : 1;
        rss_override        : 1;
        packet_len          : 14;
        csum                : 16;
        csum_ip_bad         : 1;
        csum_ip_ok          : 1;
        csum_udp_bad        : 1;
        csum_udp_ok         : 1;
        csum_tcp_bad        : 1;
        csum_tcp_ok         : 1;
        vlan_valid          : 1;
        pad                 : 1;
        l2_pkt_type         : 2;
        pkt_type            : 6;
        l4_sport            : 16;
        l4_dport            : 16;
        ip_sa               : 128;
        ip_da               : 128;
    }
}

@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_classic_nic_header_ext_t p4_to_p4plus;
@pragma scratch_metadata
metadata p4_to_p4plus_classic_nic_header_ext_t p4_to_p4plus_scratch;

action eth_rx_rss_params(rss_type, rss_key, debug, rsvd)
{
    // --- For K+I generation

    // K: From Intrinsic & RXDMA headers
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // I: From APP header
    modify_field(p4_to_p4plus_scratch.p4plus_app_id, p4_to_p4plus.p4plus_app_id);
    modify_field(p4_to_p4plus_scratch.table0_valid, p4_to_p4plus.table0_valid);
    modify_field(p4_to_p4plus_scratch.table1_valid, p4_to_p4plus.table1_valid);
    modify_field(p4_to_p4plus_scratch.table2_valid, p4_to_p4plus.table2_valid);
    modify_field(p4_to_p4plus_scratch.table3_valid, p4_to_p4plus.table3_valid);

    modify_field(p4_to_p4plus_scratch.pkt_type, p4_to_p4plus.pkt_type);
    modify_field(p4_to_p4plus_scratch.l4_sport, p4_to_p4plus.l4_sport);
    modify_field(p4_to_p4plus_scratch.l4_dport, p4_to_p4plus.l4_dport);
    modify_field(p4_to_p4plus_scratch.ip_sa, p4_to_p4plus.ip_sa);
    modify_field(p4_to_p4plus_scratch.ip_da, p4_to_p4plus.ip_da);

    // --- D-struct generation
    modify_field(eth_rx_rss_params_scratch.rss_type, rss_type);
    modify_field(eth_rx_rss_params_scratch.rss_key, rss_key);
    modify_field(eth_rx_rss_params_scratch.debug, debug);
    modify_field(eth_rx_rss_params_scratch.rsvd, rsvd);
}

@pragma stage 0
table eth_rx_rss_params {
    reads {
        p4_intr_global.lif : exact;
    }
    actions {
        eth_rx_rss_params;
    }
    size : LIF_TABLE_SIZE;
}

// RSS Input Parts
header_type toeplitz_input0_t {
    fields {
        data : 128;
    }
}

header_type toeplitz_input1_t {
    fields {
        data : 128;
    }
}

header_type toeplitz_input2_t {
    fields {
        data : 32;
        data2 : 8;
    }
}

// RSS Key Parts
header_type toeplitz_key0_t {
    fields {
        data : 128;
    }
}

header_type toeplitz_key1_t {
    fields {
        data : 128;
    }
}

header_type toeplitz_key2_t {
    fields {
        data : 128;          // last 34 bits are qstate addr
    }
}

// Input Parts
@pragma pa_header_union ingress to_stage_2          // Flit 2
metadata toeplitz_input0_t toeplitz_input0;

@pragma pa_header_union ingress to_stage_3          // Flit 2
metadata toeplitz_input1_t toeplitz_input1;

@pragma pa_header_union ingress to_stage_6          // Flit 3
metadata toeplitz_input2_t toeplitz_input2;

// Key Parts
@pragma pa_header_union ingress to_stage_4          // Flit 2
metadata toeplitz_key0_t toeplitz_key0;

@pragma pa_header_union ingress to_stage_5          // Flit 2
metadata toeplitz_key1_t toeplitz_key1;

@pragma pa_header_union ingress to_stage_7          // Flit 3
metadata toeplitz_key2_t toeplitz_key2;

// K-vector for rss indirection table
@pragma scratch_metadata
metadata toeplitz_key2_t toeplitz_key2_scratch;

// D-vector for rss indirection table
header_type eth_rx_rss_indir {
    fields {
        enable : 8;
        qid : 8;
    }
}

@pragma scratch_metadata
metadata eth_rx_rss_indir eth_rx_rss_indir_scratch;

action eth_rx_rss_indir(enable, qid) {
    // For D-struct generation
    modify_field(eth_rx_rss_indir_scratch.enable, enable);
    modify_field(eth_rx_rss_indir_scratch.qid, qid);
}

@pragma stage 1
@pragma hbm_table
@pragma hash_type 4
@pragma toeplitz_key toeplitz_input0.data toeplitz_input1.data toeplitz_input2.data
@pragma toeplitz_seed toeplitz_key0.data toeplitz_key1.data toeplitz_key2.data
table eth_rx_rss_indir {
    reads {
        // Flit 2
        toeplitz_input0.data      : exact;
        toeplitz_input1.data      : exact;
        toeplitz_key0.data        : exact;
        toeplitz_key1.data        : exact;
        // Flit 3
        toeplitz_input2.data      : exact;
        toeplitz_key2.data        : exact;
    }
    actions {
        eth_rx_rss_indir;
    }
    size : 128;
}

action rx_table_cpu_hash_action() {

}

@pragma stage 3
@pragma hash_type 4
@pragma toeplitz_key toeplitz_input0.data toeplitz_input1.data toeplitz_input2.data toeplitz_input2.data2
@pragma toeplitz_seed toeplitz_key0.data toeplitz_key1.data toeplitz_key2.data
table rx_table_cpu_hash {
    reads {
        // Flit 2
        toeplitz_input0.data      : exact;
        toeplitz_input1.data      : exact;
        toeplitz_key0.data        : exact;
        toeplitz_key1.data        : exact;
        // Flit 3
        toeplitz_input2.data      : exact;
        toeplitz_input2.data2     : exact;
        toeplitz_key2.data        : exact;
    }
    actions {
        rx_table_cpu_hash_action;
    }
    size : 0;
}

action rx_stage0_packet_len() {
    modify_field(p4_intr_scratch.frame_size, p4_intr.frame_size);
    modify_field(p4_rxdma_intr_scratch.rx_splitter_offset, p4_rxdma_intr.rx_splitter_offset);

    modify_field(scratch_app.app_type, app_header.app_type);
}

@pragma stage 0
table rx_stage0_packet_len {
    actions {
        rx_stage0_packet_len;
    }
}

control common_p4plus_stage0 {

    if (app_header.table0_valid == 1) {
        apply(common_p4plus_stage0_app_header_table_offset_64);
        if (app_header.table1_valid == 1) {
            apply(common_p4plus_stage0_app_header_table_ext_offset_64);
        }
    } else {
        apply(common_p4plus_stage0_app_header_table);
        if (app_header.table1_valid == 1) {
            apply(common_p4plus_stage0_app_header_ext_table);
        }
    }

    if (app_header.app_type == P4PLUS_APPTYPE_CLASSIC_NIC) {
        apply(eth_rx_rss_params);
    }
    if (app_header.table0_valid == 0) {
        if (app_header.table1_valid == 0) {
            if (app_header.table2_valid == 0) {
                if (app_header.table3_valid == 0) {
                    apply(eth_rx_rss_indir);
                }
            }
        }
    }

    if (app_header.app_type == P4PLUS_APPTYPE_CPU) {
        if (app_header.table0_valid == 0) {
            if (app_header.table1_valid == 0) {
                if (app_header.table2_valid == 0) {
                    if (app_header.table3_valid == 0) {
                        apply(rx_table_cpu_hash);
                    }
                }
            }
        }
    }

    if (app_header.app_type == P4PLUS_APPTYPE_RDMA) {
        apply(rx_stage0_load_rdma_params);
    }

    apply(rx_stage0_packet_len);
}

control ingress {
    common_p4plus_stage0();

    if (app_header.table0_valid == 1) {
       //apply(rx_table_s0_t0);
       apply(rx_table_s1_t0);
       apply(rx_table_s2_t0);
       apply(rx_table_s3_t0);
       apply(rx_table_s4_t0);
       apply(rx_table_s5_t0);
       apply(rx_table_s6_t0);
       apply(rx_table_s7_t0);
    }
    if (app_header.table1_valid == 1) {
       //apply(rx_table_s0_t1);
       apply(rx_table_s1_t1);
       apply(rx_table_s2_t1);
       apply(rx_table_s3_t1);
       apply(rx_table_s4_t1);
       apply(rx_table_s5_t1);
       apply(rx_table_s6_t1);
       apply(rx_table_s7_t1);
    }
    if (app_header.table2_valid == 1) {
        //apply(rx_table_s0_t2);
        apply(rx_table_s1_t2);
        apply(rx_table_s2_t2);
        apply(rx_table_s3_t2);
        apply(rx_table_s4_t2);
        apply(rx_table_s5_t2);
        apply(rx_table_s6_t2);
        apply(rx_table_s7_t2);
    }
    if (app_header.table3_valid == 1) {
        //apply(rx_table_s0_t3);
        apply(rx_table_s1_t3);
        apply(rx_table_s2_t3);
        apply(rx_table_s3_t3);
        apply(rx_table_s4_t3);
        apply(rx_table_s5_t3);
        apply(rx_table_s6_t3);
        apply(rx_table_s7_t3);
    }
}
