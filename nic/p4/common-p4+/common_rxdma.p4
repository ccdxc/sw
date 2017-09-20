#include "./common_headers.p4"


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


header_type classic_scratch_metadata_t {
    fields {
        hash_seed_320 : 320;
    }
}

@pragma dont_trim
@pragma scratch_metadata
metadata classic_scratch_metadata_t scratch_classic;

@pragma dont_trim
@pragma scratch_metadata
metadata rdma_scratch_metadata_t scratch_rdma;

@pragma dont_trim
@pragma scratch_metadata
metadata p4_2_p4plus_app_header_t scratch_app;

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
action rx_table_dummy_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata1)
}

action common_p4plus_stage0_app_header_table_action_dummy_default() {
}

//stage 7 action functions 
action rx_table_s7_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action rx_table_s7_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

//stage 6 action functions 
action rx_table_s6_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action rx_table_s6_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

//stage 5 action functions 
action rx_table_s5_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action rx_table_s5_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

//stage 4 action functions 
action rx_table_s4_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action rx_table_s4_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

//stage 3 action functions 
action rx_table_s3_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action rx_table_s3_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}


//stage 2 action functions 
action rx_table_s2_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action rx_table_s2_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

//stage 1 action functions
action rx_table_s1_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t2_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t1_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action rx_table_s1_t0_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, 
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
action rx_table_s0_t3_cfg_action(data0, data1,
                             data2, data3,
                             data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te3
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action rx_table_s0_t2_cfg_action(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te2
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action rx_table_s0_t1_cfg_action(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te1
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action rx_table_s0_t0_cfg_action(data0, data1, data2, data3, data4, data5, data6, data7)
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
                                  cqcb_base_addr_page_id, 
                                  log_num_cq_entries,
                                  prefetch_pool_base_addr_page_id,
                                  log_num_prefetch_pool_entries, 
                                  reserved) {
    if (((1 << p4_rxdma_intr.qtype) & rdma_en_qtype_mask)  != 0) {

        modify_field(scratch_rdma.rdma_en_qtype_mask, rdma_en_qtype_mask);
        modify_field(scratch_rdma.pt_base_addr_page_id, pt_base_addr_page_id);
        modify_field(scratch_rdma.log_num_pt_entries, log_num_pt_entries);
        modify_field(scratch_rdma.cqcb_base_addr_page_id, cqcb_base_addr_page_id);
        modify_field(scratch_rdma.log_num_cq_entries, log_num_cq_entries);
        modify_field(scratch_rdma.prefetch_pool_base_addr_page_id, 
                     prefetch_pool_base_addr_page_id);
        modify_field(scratch_rdma.log_num_prefetch_pool_entries, 
                     log_num_prefetch_pool_entries);
        modify_field(scratch_rdma.reserved, reserved);
    }
}

@pragma stage 0
table rx_stage0_load_rdma_params {
    reads {
        p4_intr_global.lif : exact;
    }
    actions {
        rx_stage0_load_rdma_params;
    }
    size : LIF_TABLE_SIZE;
}


action common_p4plus_stage0_lif_table0(hash_seed_320) {
    modify_field(scratch_classic.hash_seed_320, hash_seed_320);
}

@pragma stage 0
table common_p4plus_stage0_lif_table0 {
    reads {
        p4_intr_global.lif : exact;
    }
    actions {
        common_p4plus_stage0_lif_table0;
    }
    size : LIF_TABLE_SIZE;
}

action common_p4plus_stage0_app_header_table_action(data0, data1,
                             data2, data3,
                             data4, data5,
                             data6) {
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
        common_p4plus_stage0_app_header_table_action_dummy15;

@pragma stage 0
@pragma raw_index_table
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
table common_p4plus_stage0_app_header_table_offset_64 {
    reads { 
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_KEY
    }
    actions {
        COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_ACTIONS
    }
}
control common_p4plus_stage0 {
    if (app_header.table0_valid == 1) {
        apply(common_p4plus_stage0_app_header_table_offset_64);
    } else {
        apply(common_p4plus_stage0_app_header_table);
    }
    if (app_header.app_type == P4PLUS_APPTYPE_CLASSIC_NIC) {
        apply(common_p4plus_stage0_lif_table0);
    }
    if (app_header.app_type == P4PLUS_APPTYPE_RDMA) {
        if (p4_intr.recirc == 0) {
            apply(rx_stage0_load_rdma_params);
        } else {
            // apply(rx_table_s0_t0);
            // apply(rx_table_s0_t1); 
            apply(rx_table_s0_t2); 
            apply(rx_table_s0_t3);
       } 
    }
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
