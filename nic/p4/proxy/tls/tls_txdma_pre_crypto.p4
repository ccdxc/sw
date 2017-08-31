/* TLS P4 definitions */

#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       read_tls_stg1_7

#define tx_table_s2_t0_action       tls_rx_serq 
#define tx_table_s2_t1_action       read_tnmdr
#define tx_table_s2_t2_action       read_tnmpr
#define tx_table_s2_t3_action       tls_rx_serq

#define tx_table_s3_t0_action       tls_serq_consume
#define tx_table_s3_t1_action       tdesc_alloc
#define tx_table_s3_t2_action       tpage_alloc

#define tx_table_s4_t0_action       tls_bld_brq4

#define tx_table_s5_t0_action       tls_queue_brq5

#define tx_table_s6_t0_action       tls_bld_brq6

#define tx_table_s7_t0_action       tls_queue_brq7

#include "../../common-p4+/common_txdma.p4"
#include "tls_txdma_common.p4"

/* Per stage D-vector Definitions */

// d for stage 2 table 1
header_type read_tnmdr_d_t {
    fields {
        tnmdr_pidx              : 16;
    }
}

// d for stage 2 table 2
header_type read_tnmpr_d_t {
    fields {
        tnmpr_pidx              : 16;
    }
}

// d for stage 3 table 1
header_type tdesc_alloc_d_t {
    fields {
        desc                    : 64;
        pad                     : 448;
    }
}

// d for stage 3 table 2
header_type tpage_alloc_d_t {
    fields {
        page                    : 64;
        pad                     : 448;
    }
}

header_type tls_stage_bld_barco_req_d_t {
    fields {
        key_addr                        : HBM_ADDRESS_WIDTH;
        iv_addr                         : HBM_ADDRESS_WIDTH;
        command_core                    : 4;
        command_mode                    : 4;
        command_op                      : 4;
        command_param                   : 20;
        idesc                           : ADDRESS_WIDTH;
        odesc                           : ADDRESS_WIDTH;
        // TBD: Total used   : 224 bits, pending: 288
        pad                             : 288;
    }
}
#define STG_BLD_BARCO_REQ_ACTION_PARAMS                                                                 \
key_addr, iv_addr, command_core, command_mode, command_op, command_param,idesc, odesc
#

#define GENERATE_STG_BLD_BARCO_REQ_D                                                                    \
    modify_field(tls_bld_barco_req_d.key_addr, key_addr);                                               \
    modify_field(tls_bld_barco_req_d.iv_addr, iv_addr);                                                 \
    modify_field(tls_bld_barco_req_d.command_core, command_core);                                       \
    modify_field(tls_bld_barco_req_d.command_mode, command_mode);                                       \
    modify_field(tls_bld_barco_req_d.command_op, command_op);                                           \
    modify_field(tls_bld_barco_req_d.command_param, command_param);                                     \
    modify_field(tls_bld_barco_req_d.idesc, idesc);                                                     \
    modify_field(tls_bld_barco_req_d.odesc, odesc);

header_type tls_stage_queue_brq_d_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes BRQ ring
        CAPRI_QSTATE_HEADER_RING(0)

        brq_base                       : ADDRESS_WIDTH;

        // TBD: Total used   : 128 bits, pending: 384
        pad                             : 384;
    }
}

#define STG_QUEUE_BRQ_ACTION_PARAMS                                                                 \
rsvd,cosA,cosB,cos_sel,eval_last,host,total,pid, pi_0,ci_0
#

#define GENERATE_STG_QUEUE_BRQ_D                                                                    \
    modify_field(tls_queue_brq_d.rsvd, rsvd);                                                       \
    modify_field(tls_queue_brq_d.cosA, cosA);                                                       \
    modify_field(tls_queue_brq_d.cosB, cosB);                                                       \
    modify_field(tls_queue_brq_d.cos_sel, cos_sel);                                                 \
    modify_field(tls_queue_brq_d.eval_last, eval_last);                                             \
    modify_field(tls_queue_brq_d.host, host);                                                       \
    modify_field(tls_queue_brq_d.total, total);                                                     \
    modify_field(tls_queue_brq_d.pid, pid);                                                         \
    modify_field(tls_queue_brq_d.pi_0, pi_0);                                                       \
    modify_field(tls_queue_brq_d.ci_0, ci_0);



#define GENERATE_GLOBAL_K                                                                               \
        modify_field(tls_global_phv_scratch.fid, tls_global_phv.fid);                                   \
        modify_field(tls_global_phv_scratch.dec_flow, tls_global_phv.dec_flow);                         \
        modify_field(tls_global_phv_scratch.split, tls_global_phv.split);                               \
        modify_field(tls_global_phv_scratch.pending_rx_serq, tls_global_phv.pending_rx_serq);           \
        modify_field(tls_global_phv_scratch.pending_rx_brq, tls_global_phv.pending_rx_brq);             \
        modify_field(tls_global_phv_scratch.pending_queue_brq, tls_global_phv.pending_queue_brq);       \
        modify_field(tls_global_phv_scratch.tls_global_pad0, tls_global_phv.tls_global_pad0);           \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);                   \
        modify_field(tls_global_phv_scratch.tls_hdr_type, tls_global_phv.tls_hdr_type);                 \
        modify_field(tls_global_phv_scratch.tls_hdr_version_major, tls_global_phv.tls_hdr_version_major);\
        modify_field(tls_global_phv_scratch.tls_hdr_version_minor, tls_global_phv.tls_hdr_version_minor);\
        modify_field(tls_global_phv_scratch.tls_hdr_len, tls_global_phv.tls_hdr_len);                   \
        modify_field(tls_global_phv_scratch.next_tls_hdr_offset, tls_global_phv.next_tls_hdr_offset);

/* Global PHV definition */
header_type tls_global_phv_t {
    fields {
        fid                             : 16;
        dec_flow                        : 8;
        split                           : 1;
        pending_rx_serq                 : 1;
        pending_rx_brq                  : 1;
        pending_queue_brq               : 1;
        tls_global_pad0                 : 4;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
        tls_hdr_type                    : 8;
        tls_hdr_version_major           : 8;
        tls_hdr_version_minor           : 8;
        tls_hdr_len                     : 16;
        next_tls_hdr_offset             : 16;
    }
}

header_type to_stage_2_phv_t {
    fields {
        serq_ci                         : 16;
        idesc                           : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_3_phv_t {
    fields {
        tnmdr_pi                        : 16;
        tnmpr_pi                        : 16;
        serq_ci                         : 16;
    }
}

header_type to_stage_4_phv_t {
    fields {
        idesc                           : ADDRESS_WIDTH;
        odesc                           : ADDRESS_WIDTH;
    }
}

header_type to_stage_5_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        opage                           : HBM_ADDRESS_WIDTH;
        cur_tls_data_len                : 16;
        debug_dol                       : 8;
    }
}

header_type to_stage_6_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_7_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        opage                           : HBM_ADDRESS_WIDTH;
        cur_tls_data_len                : 16;
        next_tls_hdr_offset             : 16;
    }
}


#define GENERATE_S2_S4_T0                                                               \
        modify_field(s2_s4_t0_scratch.idesc_aol0_addr, s2_s4_t0_phv.idesc_aol0_addr);   \
        modify_field(s2_s4_t0_scratch.idesc_aol0_offset, s2_s4_t0_phv.idesc_aol0_offset);   \
        modify_field(s2_s4_t0_scratch.idesc_aol0_len, s2_s4_t0_phv.idesc_aol0_len);
header_type s2_s4_t0_phv_t {
    fields {
        idesc_aol0_addr                 : HBM_ADDRESS_WIDTH;
        idesc_aol0_offset               : HBM_ADDRESS_WIDTH;
        idesc_aol0_len                  : 16;
    }
}

#define GENERATE_S4_S6_T0                                                               \
        modify_field(s4_s6_t0_scratch.idesc_aol0_addr, s4_s6_t0_phv.idesc_aol0_addr);   \
        modify_field(s4_s6_t0_scratch.idesc_aol0_offset, s4_s6_t0_phv.idesc_aol0_offset);   \
        modify_field(s4_s6_t0_scratch.idesc_aol0_len, s4_s6_t0_phv.idesc_aol0_len);
header_type s4_s6_t0_phv_t {
    fields {
        idesc_aol0_addr                 : HBM_ADDRESS_WIDTH;
        idesc_aol0_offset               : HBM_ADDRESS_WIDTH;
        idesc_aol0_len                  : 16;
    }
}


header_type s3_t1_s2s_phv_t {
    fields {
        tnmdr_pidx              : 16;
    }
}

header_type s3_t2_s2s_phv_t {
    fields {
        tnmpr_pidx              : 16;
    }
}


@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_1_t tlscb_1_d;

@pragma scratch_metadata
metadata tls_stage_bld_barco_req_d_t tls_bld_barco_req_d;

@pragma scratch_metadata
metadata tls_stage_queue_brq_d_t tls_queue_brq_d;

@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;


@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;

@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;

@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;

@pragma pa_header_union ingress  common_t0_s2s s4_s6_t0_phv
metadata s2_s4_t0_phv_t s2_s4_t0_phv;
metadata s4_s6_t0_phv_t s4_s6_t0_phv;

@pragma pa_header_union ingress common_t1_s2s
metadata s3_t1_s2s_phv_t s3_t1_s2s;

@pragma pa_header_union ingress common_t2_s2s
metadata s3_t2_s2s_phv_t s3_t2_s2s;


@pragma dont_trim
metadata barco_desc_t barco_desc;
@pragma dont_trim
metadata pkt_descr_t aol; 
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd4;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd5;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd6;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd7;



@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;
@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;
@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;
@pragma scratch_metadata
metadata s2_s4_t0_phv_t s2_s4_t0_scratch;
@pragma scratch_metadata
metadata s4_s6_t0_phv_t s4_s6_t0_scratch;

@pragma scratch_metadata
metadata s3_t1_s2s_phv_t s3_t1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t2_s2s_phv_t s3_t2_s2s_scratch;


@pragma scratch_metadata
metadata read_tnmdr_d_t read_tnmdr_d;
@pragma scratch_metadata
metadata read_tnmpr_d_t read_tnmpr_d;
@pragma scratch_metadata
metadata tdesc_alloc_d_t tdesc_alloc_d;
@pragma scratch_metadata
metadata tpage_alloc_d_t tpage_alloc_d;



action read_tls_stg1_7(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_TLSCB_1_D
}



/* Stage 2 table 0 action */
action tls_rx_serq(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 2 fields */
    modify_field(to_s2_scratch.serq_ci, to_s2.serq_ci);
    modify_field(to_s2_scratch.idesc, to_s2.idesc);

    GENERATE_TLSCB_1_D
}

/*
 * Stage 2 table 1 action
 */
action read_tnmdr(tnmdr_pidx) {
    // d for stage 2 table 1 read-tnmdr-idx
    modify_field(read_tnmdr_d.tnmdr_pidx, tnmdr_pidx);
}

/*
 * Stage 2 table 2 action
 */
action read_tnmpr(tnmpr_pidx) {
    // d for stage 2 table 2 read-tnmpr-idx
    modify_field(read_tnmpr_d.tnmpr_pidx, tnmpr_pidx);
}

/* Stage 3 table 0 action */
action tls_serq_consume(TLSCB_0_PARAMS) {


    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.serq_ci, to_s3.serq_ci);

    /* D vector */
    GENERATE_TLSCB_0_D

}

/*
 * Stage 3 table 1 action
 */
action tdesc_alloc(desc, pad) {
    // k + i for stage 3 table 1

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t1_s2s_scratch.tnmdr_pidx, s3_t1_s2s.tnmdr_pidx);

    // d for stage 3 table 1
    modify_field(tdesc_alloc_d.desc, desc);
    modify_field(tdesc_alloc_d.pad, pad);
}

/*
 * Stage 3 table 2 action
 */
action tpage_alloc(page, pad) {
    // k + i for stage 3 table 2

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t2_s2s_scratch.tnmpr_pidx, s3_t2_s2s.tnmpr_pidx);

    // d for stage 3 table 2
    modify_field(tpage_alloc_d.page, page);
    modify_field(tpage_alloc_d.pad, pad);
}


/* Stage 3 table 3 action */
action tls_stage3(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 2 fields */
    modify_field(to_s2_scratch.serq_ci, to_s2.serq_ci);
    modify_field(to_s2_scratch.idesc, to_s2.idesc);

    GENERATE_TLSCB_1_D
}

/* Stage 4 action */
action tls_bld_brq4(STG_BLD_BARCO_REQ_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_S2_S4_T0

    /* To Stage 4 fields */
    modify_field(to_s4_scratch.idesc, to_s4.idesc);
    modify_field(to_s4_scratch.odesc, to_s4.odesc);


    GENERATE_STG_BLD_BARCO_REQ_D
}


/* Stage 5 action */
action tls_queue_brq5(STG_QUEUE_BRQ_ACTION_PARAMS) {

    GENERATE_GLOBAL_K



    /* To Stage 5 fields */
    modify_field(to_s5_scratch.idesc, to_s5.idesc);
    modify_field(to_s5_scratch.odesc, to_s5.odesc);
    modify_field(to_s5_scratch.opage, to_s5.opage);
    modify_field(to_s5_scratch.cur_tls_data_len, to_s5.cur_tls_data_len);
    modify_field(to_s5_scratch.debug_dol, to_s5.debug_dol);


    GENERATE_STG_QUEUE_BRQ_D
}

/* Stage 6 action */
action tls_bld_brq6(STG_BLD_BARCO_REQ_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_S4_S6_T0

    /* To Stage 6 fields */
    modify_field(to_s6_scratch.idesc, to_s6.idesc);
    modify_field(to_s6_scratch.odesc, to_s6.odesc);


    GENERATE_STG_BLD_BARCO_REQ_D
}

/* Stage 7 action */
action tls_queue_brq7(STG_QUEUE_BRQ_ACTION_PARAMS) {

    GENERATE_GLOBAL_K



    /* To Stage 7 fields */
    modify_field(to_s7_scratch.idesc, to_s7.idesc);
    modify_field(to_s7_scratch.odesc, to_s7.odesc);
    modify_field(to_s7_scratch.opage, to_s7.opage);
    modify_field(to_s7_scratch.cur_tls_data_len, to_s7.cur_tls_data_len);
    modify_field(to_s7_scratch.next_tls_hdr_offset, to_s7.next_tls_hdr_offset);

    GENERATE_STG_QUEUE_BRQ_D
}
