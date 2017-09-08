#include "./common_headers.p4"


@pragma dont_trim
metadata cap_phv_intr_global_t p4_intr_global;
@pragma dont_trim
metadata cap_phv_intr_p4_t     p4_intr;
@pragma dont_trim
metadata cap_phv_intr_txdma_t p4_txdma_intr;

// may have to put a pad to round of  the above 2 headers + something to 512 bits
@pragma dont_trim
metadata p4plus_2_p4_app_header_t app_header;

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

header_type txdma_common_pad_t {
    fields {
        txdma_common_pad : 96;
    }
}

@pragma dont_trim
@pragma scratch_metadata
metadata rdma_scratch_metadata_t scratch_rdma;

@pragma dont_trim
metadata txdma_common_pad_t txdma_common_pad;

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata0;

@pragma scratch_metadata
metadata scratch_metadata_t scratch_metadata1;

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
@pragma scratch_metadata
metadata p4plus_common_global_t global_scratch;

@pragma scratch_metadata
metadata p4plus_common_s2s_t s2s_scratch0;

@pragma scratch_metadata
metadata cap_phv_intr_global_t p4_intr_global_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_p4_t     p4_intr_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_txdma_t  p4_txdma_intr_scratch;

@pragma scratch_metadata
metadata p4plus_2_p4_app_header_t app_header_scratch;


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

// dummy action function 
action tx_table_dummy_action(data0, data1,
                             data2, data3,
                             data4, data5, 
                             data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata1)
}

//stage 7 action functions 
action tx_table_s7_t3_cfg_action(data0, data1,
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

action tx_table_s7_t2_cfg_action(data0, data1,
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

action tx_table_s7_t1_cfg_action(data0, data1,
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

action tx_table_s7_t0_cfg_action(data0, data1,
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
action tx_table_s6_t3_cfg_action(data0, data1,
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

action tx_table_s6_t2_cfg_action(data0, data1,
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

action tx_table_s6_t1_cfg_action(data0, data1,
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

action tx_table_s6_t0_cfg_action(data0, data1,
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
action tx_table_s5_t3_cfg_action(data0, data1,
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

action tx_table_s5_t2_cfg_action(data0, data1,
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

action tx_table_s5_t1_cfg_action(data0, data1,
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

action tx_table_s5_t0_cfg_action(data0, data1,
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
action tx_table_s4_t3_cfg_action(data0, data1,
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

action tx_table_s4_t2_cfg_action(data0, data1,
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

action tx_table_s4_t1_cfg_action(data0, data1,
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

action tx_table_s4_t0_cfg_action(data0, data1,
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
action tx_table_s3_t3_cfg_action(data0, data1,
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

action tx_table_s3_t2_cfg_action(data0, data1,
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

action tx_table_s3_t1_cfg_action(data0, data1,
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

action tx_table_s3_t0_cfg_action(data0, data1,
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
action tx_table_s2_t3_cfg_action(data0, data1,
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

action tx_table_s2_t2_cfg_action(data0, data1,
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

action tx_table_s2_t1_cfg_action(data0, data1,
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

action tx_table_s2_t0_cfg_action(data0, data1,
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
action tx_table_s1_t3_cfg_action(data0, data1,
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

action tx_table_s1_t2_cfg_action(data0, data1,
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

action tx_table_s1_t1_cfg_action(data0, data1,
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

action tx_table_s1_t0_cfg_action(data0, data1,
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
action tx_table_s0_t3_cfg_action(data0, data1,
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

action tx_table_s0_t2_cfg_action(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te2
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action tx_table_s0_t1_cfg_action(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT(scratch_metadata0)
    //te1
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action tx_table_s0_t0_cfg_action(data0, data1,
                                 data2, data3,
                                 data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata1)
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);
    modify_field(app_header_scratch.table0_valid, app_header.table0_valid);
    modify_field(app_header_scratch.table1_valid, app_header.table1_valid);
    modify_field(app_header_scratch.table2_valid, app_header.table2_valid);
    modify_field(app_header_scratch.table3_valid, app_header.table3_valid);
}


// stage 7
@pragma stage 7
@pragma raw_table common_te3_phv.table_pc
table tx_table_s7_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t3_action;
        tx_table_s7_t3_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te2_phv.table_pc
table tx_table_s7_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t2_action;
        tx_table_s7_t2_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te1_phv.table_pc
table tx_table_s7_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t1_action;
        tx_table_s7_t1_cfg_action;
    }
}

@pragma stage 7
@pragma raw_table common_te0_phv.table_pc
table tx_table_s7_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t0_action;
        tx_table_s7_t0_cfg_action;
    }
}

// stage 6
@pragma stage 6
@pragma raw_table common_te3_phv.table_pc
table tx_table_s6_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t3_action;
        tx_table_s6_t3_cfg_action;
    }
}

@pragma stage 6
@pragma raw_table common_te2_phv.table_pc
table tx_table_s6_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t2_action;
        tx_table_s6_t2_cfg_action;
    }
}

@pragma stage 6
@pragma raw_table common_te1_phv.table_pc
table tx_table_s6_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t1_action;
        tx_table_s6_t1_cfg_action;
    }
}

@pragma stage 6
@pragma raw_table common_te0_phv.table_pc
table tx_table_s6_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t0_action;
        tx_table_s6_t0_cfg_action;
    }
}

// stage 5
@pragma stage 5
@pragma raw_table common_te3_phv.table_pc
table tx_table_s5_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t3_action;
        tx_table_s5_t3_cfg_action;
    }
}

@pragma stage 5
@pragma raw_table common_te2_phv.table_pc
table tx_table_s5_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t2_action;
        tx_table_s5_t2_cfg_action;
    }
}

@pragma stage 5
@pragma raw_table common_te1_phv.table_pc
table tx_table_s5_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t1_action;
        tx_table_s5_t1_cfg_action;
    }
}

@pragma stage 5
@pragma raw_table common_te0_phv.table_pc
table tx_table_s5_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t0_action;
        tx_table_s5_t0_cfg_action;
    }
}

// stage 4
@pragma stage 4
@pragma raw_table common_te3_phv.table_pc
table tx_table_s4_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t3_action;
        tx_table_s4_t3_cfg_action;
    }
}

@pragma stage 4
@pragma raw_table common_te2_phv.table_pc
table tx_table_s4_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t2_action;
        tx_table_s4_t2_cfg_action;
    }
}

@pragma stage 4
@pragma raw_table common_te1_phv.table_pc
table tx_table_s4_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t1_action;
        tx_table_s4_t1_cfg_action;
    }
}

@pragma stage 4
@pragma raw_table common_te0_phv.table_pc
table tx_table_s4_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t0_action;
        tx_table_s4_t0_cfg_action;
        tx_table_s4_t0_action1;
    }
}

// stage 3
@pragma stage 3
@pragma raw_table common_te3_phv.table_pc
table tx_table_s3_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t3_action;
        tx_table_s3_t3_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te2_phv.table_pc
table tx_table_s3_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t2_action;
        tx_table_s3_t2_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te1_phv.table_pc
table tx_table_s3_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t1_action;
        tx_table_s3_t1_cfg_action;
    }
}

@pragma stage 3
@pragma raw_table common_te0_phv.table_pc
table tx_table_s3_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t0_action;
        tx_table_s3_t0_cfg_action;
        tx_table_s3_t0_action1;
        tx_table_s3_t0_action2;
    }
}

// stage 2
@pragma stage 2
@pragma raw_table common_te3_phv.table_pc
table tx_table_s2_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t3_action;
        tx_table_s2_t3_cfg_action;
    }
}

@pragma stage 2
@pragma raw_table common_te2_phv.table_pc
table tx_table_s2_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t2_action;
        tx_table_s2_t2_cfg_action;
    }
}
@pragma stage 2
@pragma raw_table common_te1_phv.table_pc
table tx_table_s2_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t1_action;
        tx_table_s2_t1_cfg_action;
    }
}

@pragma stage 2
@pragma raw_table common_te0_phv.table_pc
table tx_table_s2_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t0_action;
        tx_table_s2_t0_cfg_action;
        tx_table_s2_t0_action1;
        tx_table_s2_t0_action2;
        tx_table_s2_t0_action3;
        tx_table_s2_t0_action4;
    }
}

// stage 1
@pragma stage 1
@pragma raw_table common_te3_phv.table_pc
table tx_table_s1_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t3_action;
        tx_table_s1_t3_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te2_phv.table_pc
table tx_table_s1_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t2_action;
        tx_table_s1_t2_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te1_phv.table_pc
table tx_table_s1_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t1_action;
        tx_table_s1_t1_cfg_action;
    }
}

@pragma stage 1
@pragma raw_table common_te0_phv.table_pc
table tx_table_s1_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t0_action;
        tx_table_s1_t0_cfg_action;
        tx_table_s1_t0_action1;
        tx_table_s1_t0_action2;
        tx_table_s1_t0_action3;
        tx_table_s1_t0_action4;
        tx_table_s1_t0_action5;
        tx_table_s1_t0_action6;
    }
}

// stage 0
@pragma stage 0
@pragma raw_table common_te3_phv.table_pc
table tx_table_s0_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s0_t3_action;
        tx_table_s0_t3_cfg_action;
    }
}

@pragma stage 0
@pragma raw_table common_te2_phv.table_pc
table tx_table_s0_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s0_t2_action;
        tx_table_s0_t2_cfg_action;
    }
}

@pragma stage 0
@pragma raw_table common_te1_phv.table_pc
table tx_table_s0_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s0_t1_action;
        tx_table_s0_t1_cfg_action;
    }
}

@pragma stage 0
@pragma raw_index_table
table tx_table_s0_t0 {
    reads {
       p4_txdma_intr.qstate_addr : exact;
    }
    actions {
        tx_table_s0_t0_action;
        tx_table_s0_t0_cfg_action;
        tx_table_s0_t0_action1;
    }
}

parser start {
    return ingress;
}

action tx_stage0_load_rdma_params(rdma_en_qtype_mask, 
                                  pt_base_addr_page_id, 
                                  log_num_pt_entries, 
                                  cqcb_base_addr_page_id, 
                                  log_num_cq_entries,
                                  prefetch_pool_base_addr_page_id,
                                  log_num_prefetch_pool_entries, 
                                  reserved) {


#if 0
    sllv r5, 1, k.global.qtype
    and r5, r5, d.rdma_en_qtype_mask
    sne c1, r5, r0
#endif

    if (((1 << p4_txdma_intr.qtype) & rdma_en_qtype_mask)  != 0) {

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
table tx_stage0_rdma_params_table {
    reads {
        p4_intr_global.lif : exact;
    }
    actions {
        tx_stage0_load_rdma_params;
    }
    size : LIF_TABLE_SIZE;
}


control common_tx_p4plus_stage0 {
    apply(tx_table_s0_t0);
    if (p4_intr.recirc == 0) {
        apply(tx_stage0_rdma_params_table);
    }
}   

control ingress {
    common_tx_p4plus_stage0();
    if (app_header.table0_valid == 1) {
        apply(tx_table_s1_t0);
        apply(tx_table_s2_t0);
        apply(tx_table_s3_t0);
        apply(tx_table_s4_t0);
        apply(tx_table_s5_t0);
        apply(tx_table_s6_t0);
        apply(tx_table_s7_t0);
    }
    if (app_header.table1_valid == 1) {
        apply(tx_table_s0_t1);
        apply(tx_table_s1_t1);
        apply(tx_table_s2_t1);
        apply(tx_table_s3_t1);
        apply(tx_table_s4_t1);
        apply(tx_table_s5_t1);
        apply(tx_table_s6_t1);
        apply(tx_table_s7_t1);
    }
    if (app_header.table2_valid == 1) {
        apply(tx_table_s0_t2);
        apply(tx_table_s1_t2);
        apply(tx_table_s2_t2);
        apply(tx_table_s3_t2);
        apply(tx_table_s4_t2);
        apply(tx_table_s5_t2);
        apply(tx_table_s6_t2);
        apply(tx_table_s7_t2);
    }
    if (app_header.table3_valid == 1) {
        apply(tx_table_s0_t3);
        apply(tx_table_s1_t3);
        apply(tx_table_s2_t3);
        apply(tx_table_s3_t3);
        apply(tx_table_s4_t3);
        apply(tx_table_s5_t3);
        apply(tx_table_s6_t3);
        apply(tx_table_s7_t3);
    }
}

