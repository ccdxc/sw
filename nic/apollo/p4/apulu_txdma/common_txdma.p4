#include "nic/p4/include/intrinsic.p4"
#include "nic/p4/common/defines.h"
#include "../include/common_headers.p4"
#include "common_metadata.p4"

@pragma scratch_metadata
metadata common_scratch_metadata_t common_scratch_metadata0;

@pragma scratch_metadata
metadata common_scratch_metadata_t common_scratch_metadata1;

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
metadata cap_phv_intr_global_t capri_intr_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_p4_t     capri_p4_intr_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_txdma_t  capri_txdma_intr_scratch;

@pragma scratch_metadata
metadata cap_phv_intr_global_t p4_intr_global_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_p4_t     p4_intr_scratch;
@pragma scratch_metadata
metadata cap_phv_intr_txdma_t  p4_txdma_intr_scratch;

@pragma scratch_metadata
metadata p4plus_2_p4_app_header_t app_header_scratch;

@pragma scratch_metadata
metadata policer_scratch_metadata_t scratch_policer;

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
action tx_table_dummy_action(data0, data1, data2, data3, data4, data5,
                             data6, data7)
{
    SCRATCH_METADATA_INIT_7(common_scratch_metadata1)
}

//stage 7 action functions
action tx_table_s7_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action tx_table_s7_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action tx_table_s7_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

action tx_table_s7_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_7.to_stage_data);
}

//stage 6 action functions
action tx_table_s6_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action tx_table_s6_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action tx_table_s6_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

action tx_table_s6_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_6.to_stage_data);
}

//stage 5 action functions
action tx_stage5_lif_egress_rl_params(entry_valid, pkt_rate, rlimit_en,
                                      rlimit_prof, color_aware,
                                      rsvd, axi_wr_pend, burst, rate, tbkt) {
    modify_field(scratch_policer.policer_valid, entry_valid);
    modify_field(scratch_policer.policer_pkt_rate, pkt_rate);
    modify_field(scratch_policer.policer_rlimit_en, rlimit_en);
    modify_field(scratch_policer.policer_rlimit_prof, rlimit_prof);
    modify_field(scratch_policer.policer_color_aware, color_aware);
    modify_field(scratch_policer.policer_rsvd, rsvd);
    modify_field(scratch_policer.policer_axi_wr_pend, axi_wr_pend);
    modify_field(scratch_policer.policer_burst, burst);
    modify_field(scratch_policer.policer_rate, rate);
    modify_field(scratch_policer.policer_tbkt, tbkt);
}

action tx_table_s5_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action tx_table_s5_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action tx_table_s5_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

action tx_table_s5_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_5.to_stage_data);
}

//stage 4 action functions
action tx_table_s4_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action tx_table_s4_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action tx_table_s4_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

action tx_table_s4_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_4.to_stage_data);
}

//stage 3 action functions
action tx_table_s3_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action tx_table_s3_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action tx_table_s3_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}

action tx_table_s3_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_3.to_stage_data);
}


//stage 2 action functions
action tx_table_s2_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action tx_table_s2_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action tx_table_s2_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

action tx_table_s2_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE0_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH0
    modify_field(te_scratch.to_stage_data, to_stage_2.to_stage_data);
}

//stage 1 action functions
action tx_table_s1_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action tx_table_s1_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action tx_table_s1_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_1.to_stage_data);
}

action tx_table_s1_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
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
action tx_table_s0_t3_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    //te3
    SET_RAW_TABLE3_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH3
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action tx_table_s0_t2_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    //te2
    SET_RAW_TABLE2_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH2
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action tx_table_s0_t1_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT(common_scratch_metadata0)
    //te1
    SET_RAW_TABLE1_SCRATCH
    SET_GLOBAL_SCRATCH
    SET_S2S_SCRATCH1
    modify_field(te_scratch.to_stage_data, to_stage_0.to_stage_data);

}

action tx_table_s0_t0_cfg_action(data0, data1, data2, data3, data4, data5,
                                 data6, data7)
{
    SCRATCH_METADATA_INIT_7(common_scratch_metadata1)
    modify_field(capri_intr_scratch.lif, capri_intr.lif);
    modify_field(capri_intr_scratch.tm_iq, capri_intr.tm_iq);
    modify_field(capri_p4_intr_scratch.recirc_count, capri_p4_intr.recirc_count);
    modify_field(capri_txdma_intr_scratch.qid, capri_txdma_intr.qid);
    modify_field(capri_txdma_intr_scratch.qtype, capri_txdma_intr.qtype);
    modify_field(capri_txdma_intr_scratch.qstate_addr, capri_txdma_intr.qstate_addr);
    modify_field(app_header_scratch.table0_valid, app_header.table0_valid);
    modify_field(app_header_scratch.table1_valid, app_header.table1_valid);
    modify_field(app_header_scratch.table2_valid, app_header.table2_valid);
    modify_field(app_header_scratch.table3_valid, app_header.table3_valid);

    modify_field(app_header_scratch.app_data1, app_header.app_data1);
}


// stage 7
@pragma stage 7
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
@pragma memory_only
table tx_table_s7_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t3_action;
        tx_table_s7_t3_cfg_action;
        tx_table_s7_t3_action1;
        tx_table_s7_t3_action2;
        tx_table_s7_t3_action3;
        tx_table_s7_t3_action4;
    }
}

@pragma stage 7
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s7_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t2_action;
        tx_table_s7_t2_cfg_action;
        tx_table_s7_t2_action1;
        tx_table_s7_t2_action2;
        tx_table_s7_t2_action3;
        tx_table_s7_t2_action4;
    }
}

@pragma stage 7
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s7_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t1_action;
        tx_table_s7_t1_cfg_action;
        tx_table_s7_t1_action1;
        tx_table_s7_t1_action2;
        tx_table_s7_t1_action3;
        tx_table_s7_t1_action4;
    }
}

@pragma stage 7
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table tx_table_s7_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s7_t0_action;
        tx_table_s7_t0_cfg_action;
        tx_table_s7_t0_action1;
        tx_table_s7_t0_action2;
        tx_table_s7_t0_action3;
        tx_table_s7_t0_action4;
    }
}

// stage 6
@pragma stage 6
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s6_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t3_action;
        tx_table_s6_t3_cfg_action;
        tx_table_s6_t3_action1;
        tx_table_s6_t3_action2;
        tx_table_s6_t3_action3;
        tx_table_s6_t3_action4;
    }
}

@pragma stage 6
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s6_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t2_action;
        tx_table_s6_t2_cfg_action;
        tx_table_s6_t2_action1;
        tx_table_s6_t2_action2;
        tx_table_s6_t2_action3;
        tx_table_s6_t2_action4;
    }
}

@pragma stage 6
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s6_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t1_action;
        tx_table_s6_t1_cfg_action;
        tx_table_s6_t1_action1;
        tx_table_s6_t1_action2;
        tx_table_s6_t1_action3;
        tx_table_s6_t1_action4;
    }
}

@pragma stage 6
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table tx_table_s6_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s6_t0_action;
        tx_table_s6_t0_cfg_action;
        tx_table_s6_t0_action1;
        tx_table_s6_t0_action2;
        tx_table_s6_t0_action3;
        tx_table_s6_t0_action4;
    }
}

// stage 5
@pragma stage 5
@pragma policer_table two_color
@pragma enable_rate_limit
table tx_table_s5_t4_lif_rate_limiter_table {
    reads {
        capri_intr.lif : exact;
    }
    actions {
        tx_stage5_lif_egress_rl_params;
    }
    size : EGRESS_RATE_LIMITER_TABLE_SIZE;
}

@pragma stage 5
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s5_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t3_action;
        tx_table_s5_t3_cfg_action;
        tx_table_s5_t3_action1;
        tx_table_s5_t3_action2;
        tx_table_s5_t3_action3;
        tx_table_s5_t3_action4;
    }
}

@pragma stage 5
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s5_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t2_action;
        tx_table_s5_t2_cfg_action;
        tx_table_s5_t2_action1;
        tx_table_s5_t2_action2;
        tx_table_s5_t2_action3;
        tx_table_s5_t2_action4;
        tx_table_s5_t2_action5;
    }
}

@pragma stage 5
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s5_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t1_action;
        tx_table_s5_t1_cfg_action;
        tx_table_s5_t1_action1;
        tx_table_s5_t1_action2;
        tx_table_s5_t1_action3;
        tx_table_s5_t1_action4;
    }
}

@pragma stage 5
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table tx_table_s5_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s5_t0_action;
        tx_table_s5_t0_cfg_action;
        tx_table_s5_t0_action1;
        tx_table_s5_t0_action2;
        tx_table_s5_t0_action3;
        tx_table_s5_t0_action4;
    }
}

// stage 4
@pragma stage 4
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s4_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t3_action;
        tx_table_s4_t3_cfg_action;
        tx_table_s4_t3_action1;
        tx_table_s4_t3_action2;
        tx_table_s4_t3_action3;
        tx_table_s4_t3_action4;
    }
}

@pragma stage 4
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s4_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t2_action;
        tx_table_s4_t2_cfg_action;
        tx_table_s4_t2_action1;
        tx_table_s4_t2_action2;
        tx_table_s4_t2_action3;
        tx_table_s4_t2_action4;
    }
}

@pragma stage 4
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s4_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t1_action;
        tx_table_s4_t1_cfg_action;
        tx_table_s4_t1_action1;
        tx_table_s4_t1_action2;
        tx_table_s4_t1_action3;
        tx_table_s4_t1_action4;
    }
}

@pragma stage 4
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table tx_table_s4_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s4_t0_action;
        tx_table_s4_t0_cfg_action;
        tx_table_s4_t0_action1;
        tx_table_s4_t0_action2;
        tx_table_s4_t0_action3;
        tx_table_s4_t0_action4;
        tx_table_s4_t0_action5;
        tx_table_s4_t0_action6;
        tx_table_s4_t0_action7;
    }
}

// stage 3
@pragma stage 3
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s3_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t3_action;
        tx_table_s3_t3_cfg_action;
        tx_table_s3_t3_action1;
        tx_table_s3_t3_action2;
        tx_table_s3_t3_action3;
        tx_table_s3_t3_action4;
    }
}

@pragma stage 3
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s3_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t2_action;
        tx_table_s3_t2_cfg_action;
        tx_table_s3_t2_action1;
        tx_table_s3_t2_action2;
        tx_table_s3_t2_action3;
        tx_table_s3_t2_action4;
    }
}

@pragma stage 3
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s3_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t1_action;
        tx_table_s3_t1_cfg_action;
        tx_table_s3_t1_action1;
        tx_table_s3_t1_action2;
        tx_table_s3_t1_action3;
        tx_table_s3_t1_action4;
    }
}

@pragma stage 3
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
table tx_table_s3_t0 {
    reads {
       common_te0_phv.table_addr : exact;
    }
    actions {
        tx_table_s3_t0_action;
        tx_table_s3_t0_cfg_action;
        tx_table_s3_t0_action1;
        tx_table_s3_t0_action2;
        tx_table_s3_t0_action3;
        tx_table_s3_t0_action4;
        tx_table_s3_t0_action5;
        tx_table_s3_t0_action6;
    }
}

// stage 2
@pragma stage 2
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s2_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t3_action;
        tx_table_s2_t3_cfg_action;
        tx_table_s2_t3_action1;
        tx_table_s2_t3_action2;
        tx_table_s2_t3_action3;
        tx_table_s2_t3_action4;
    }
}

@pragma stage 2
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s2_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t2_action;
        tx_table_s2_t2_cfg_action;
        tx_table_s2_t2_action1;
        tx_table_s2_t2_action2;
        tx_table_s2_t2_action3;
        tx_table_s2_t2_action4;
    }
}
@pragma stage 2
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s2_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s2_t1_action;
        tx_table_s2_t1_cfg_action;
        tx_table_s2_t1_action1;
        tx_table_s2_t1_action2;
        tx_table_s2_t1_action3;
        tx_table_s2_t1_action4;
    }
}

@pragma stage 2
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
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
        tx_table_s2_t0_action5;
        tx_table_s2_t0_action6;
        tx_table_s2_t0_action7;
        tx_table_s2_t0_action8;
        tx_table_s2_t0_action9;
        tx_table_s2_t0_action10;
        tx_table_s2_t0_action11;
        tx_table_s2_t0_action12;
    }
}

// stage 1
@pragma stage 1
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s1_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t3_action;
        tx_table_s1_t3_cfg_action;
        tx_table_s1_t3_action1;
        tx_table_s1_t3_action2;
        tx_table_s1_t3_action3;
        tx_table_s1_t3_action4;
    }
}

@pragma stage 1
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s1_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t2_action;
        tx_table_s1_t2_cfg_action;
        tx_table_s1_t2_action1;
        tx_table_s1_t2_action2;
        tx_table_s1_t2_action3;
        tx_table_s1_t2_action4;
    }
}

@pragma stage 1
@pragma raw_table common_te1_phv.table_pc
@pragma table_write
table tx_table_s1_t1 {
    reads {
       common_te1_phv.table_addr : exact;
    }
    actions {
        tx_table_s1_t1_action;
        tx_table_s1_t1_cfg_action;
        tx_table_s1_t1_action1;
        tx_table_s1_t1_action2;
        tx_table_s1_t1_action3;
        tx_table_s1_t1_action4;
    }
}

@pragma stage 1
@pragma raw_table common_te0_phv.table_pc
@pragma table_write
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
        tx_table_s1_t0_action7;
        tx_table_s1_t0_action8;
        tx_table_s1_t0_action9;
        tx_table_s1_t0_action10;
        tx_table_s1_t0_action11;
    }
}

// stage 0
@pragma stage 0
@pragma raw_table common_te3_phv.table_pc
@pragma table_write
table tx_table_s0_t3 {
    reads {
       common_te3_phv.table_addr : exact;
    }
    actions {
        tx_table_s0_t3_action;
        tx_table_s0_t3_cfg_action;
        tx_table_s0_t3_action1;
        tx_table_s0_t3_action2;
        tx_table_s0_t3_action3;
        tx_table_s0_t3_action4;
    }
}

@pragma stage 0
@pragma raw_table common_te2_phv.table_pc
@pragma table_write
table tx_table_s0_t2 {
    reads {
       common_te2_phv.table_addr : exact;
    }
    actions {
        tx_table_s0_t2_action;
        tx_table_s0_t2_cfg_action;
        tx_table_s0_t2_action1;
        tx_table_s0_t2_action2;
        tx_table_s0_t2_action3;
        tx_table_s0_t2_action4;
    }
}

//s0-t1 is used to do non-lockable work like
//PHV writes etc.. to reduce the load on s0-t0
@pragma stage 0
@pragma raw_index_table
table tx_table_s0_t1 {
    reads {
       capri_txdma_intr.qstate_addr : exact;
    }
    actions {
        tx_table_s0_t0_action;
        tx_table_s0_t0_cfg_action;
        tx_table_s0_t0_action1;
        tx_table_s0_t0_action2;
        tx_table_s0_t0_action3;
        tx_table_s0_t0_action4;
    }
}

@pragma stage 0
@pragma raw_index_table
@pragma table_write
table tx_table_s0_t0 {
    reads {
       capri_txdma_intr.qstate_addr : exact;
    }
    actions {
        tx_table_s0_t0_action;
        tx_table_s0_t0_cfg_action;
        tx_table_s0_t0_action1;
        tx_table_s0_t0_action2;
        tx_table_s0_t0_action3;
        tx_table_s0_t0_action4;
    }
}

control common_tx_p4plus_stage0 {
    apply(tx_table_s0_t0);
}

parser start {
    return ingress;
}

control ingress {
    common_tx_p4plus_stage0();
    if (txdma_predicate.cps_path_en == 1) {
        if (txdma_predicate.pass_two == 0) {
            read_qstate();
        }

        if (txdma_predicate.lpm1_enable == TRUE) {
            route_lookup();
        }

        if (txdma_predicate.rfc_enable == TRUE) {
            rfc();
        }

        pkt_dma();
    } else {
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
            apply(tx_table_s1_t2);
            apply(tx_table_s2_t2);
            apply(tx_table_s3_t2);
            apply(tx_table_s4_t2);
            apply(tx_table_s5_t2);
            apply(tx_table_s6_t2);
            apply(tx_table_s7_t2);
        }
        apply(tx_table_s5_t4_lif_rate_limiter_table);
    }
}
