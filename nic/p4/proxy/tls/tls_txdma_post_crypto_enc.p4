/* TLS P4 definitions */


#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       tls_rx_bsq

#define tx_table_s2_t0_action       tls_read_desc
#define tx_table_s2_t3_action       tls_read_idesc


#define tx_table_s3_t0_action       tls_bsq_consume
#define tx_table_s3_t1_action       read_rnmdr_free_pi
#define tx_table_s3_t2_action       read_rnmpr_free_pi

#define tx_table_s4_t0_action       tls_read_odesc

#define tx_table_s5_t0_action       tls_read_aad

#define tx_table_s6_t0_action       tls_queue_sesq

#define tx_table_s7_t0_action       tls_post_crypto_stats5

#include "../../common-p4+/common_txdma.p4"
#include "tls_txdma_common.p4"

/* Per stage D-vector Definitions */

// d for stage 2 table 1
header_type read_rnmdr_free_pi_d_t {
    fields {
        rnmdr_free_pi              : 16;
    }
}

// d for stage 2 table 2
header_type read_rnmpr_free_pi_d_t {
    fields {
        rnmpr_free_pi              : 16;
    }
}


#define GENERATE_GLOBAL_K                                                                               \
        modify_field(tls_global_phv_scratch.fid, tls_global_phv.fid);                                   \
        modify_field(tls_global_phv_scratch.dec_flow, tls_global_phv.dec_flow);                         \
        modify_field(tls_global_phv_scratch.barco_op_failed, tls_global_phv.barco_op_failed);           \
        modify_field(tls_global_phv_scratch.pad, tls_global_phv.pad);                                   \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);


/* Global PHV definition */
header_type tls_global_phv_t {
    fields {
        fid                             : 16;
        dec_flow                        : 8;
        barco_op_failed                 : 1;
        pad                             : 7;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
    }
}


header_type to_stage_3_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        ipage                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
    }
}

#define GENERATE_TO_S3_K                                                                                \
    modify_field(to_s3_scratch.idesc, to_s3.idesc);                                                     \
    modify_field(to_s3_scratch.odesc, to_s3.odesc);                                                     \
    modify_field(to_s3_scratch.ipage, to_s3.ipage);

header_type to_stage_4_phv_t {
    fields {
        do_post_cbc_enc                 : 1;
        do_post_ccm_enc                 : 1;
    }
}
header_type to_stage_6_phv_t {
    fields {
        odesc                           : ADDRESS_WIDTH;
        debug_dol                       : 8;
        other_fid                       : 16;
    }
}
#define GENERATE_TO_S6_K                                                                                \
    modify_field(to_s6_scratch.odesc, to_s6.odesc);                                                     \
    modify_field(to_s6_scratch.debug_dol, to_s6.debug_dol);                                             \
    modify_field(to_s6_scratch.other_fid, to_s6.other_fid);

header_type to_stage_7_phv_t {
    fields {
        rnmdr_free                      : 8;
        rnmpr_free                      : 8;
        enc_completions                 : 8;
        dec_completions                 : 8;
        debug_stage0_3_thread           : 16;
        debug_stage4_7_thread           : 16;
    }
}

header_type doorbell_data_pad_t {
    fields {
        pad                             : 24;
    }
}

@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_1_t tlscb_1_d;

@pragma scratch_metadata
metadata barco_result_t read_desc_d;

@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;

@pragma scratch_metadata
metadata pkt_descr_aol_t PKT_DESCR_AOL_SCRATCH;

@pragma scratch_metadata
metadata read_rnmdr_free_pi_d_t read_rnmdr_free_d;

@pragma scratch_metadata
metadata read_rnmpr_free_pi_d_t read_rnmpr_free_d;

@pragma scratch_metadata
metadata tls_stage_post_crypto_stats_d_t tls_post_crypto_stats_d;

@pragma scratch_metadata
metadata additional_data_t tls_post_enc_aad_d;

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;

@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;


@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;

@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;


@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;


@pragma dont_trim
metadata pkt_descr_aol_t odesc; 
@pragma dont_trim
metadata barco_result_t read_desc;
@pragma dont_trim
metadata ring_entry_t ring_entry;
@pragma dont_trim
metadata doorbell_data_raw_t db_data;
@pragma dont_trim
metadata tls_header_t tls_hdr;
@pragma dont_trim
metadata doorbell_data_pad_t db_pad;
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



/* Stage 1 table 0 action */
action tls_rx_bsq(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_TLSCB_1_D
}

/*
 * Stage 2 table 0 action
 */
action tls_read_desc(status, output_list_address, pad) {
    GENERATE_GLOBAL_K

    // d for stage 2 table 0 read_desc
    modify_field(read_desc_d.status, status);
    modify_field(read_desc_d.output_list_address, output_list_address);
    modify_field(read_desc_d.pad, pad);

}

/* Stage 2 table 3 action */
action tls_read_idesc(PKT_DESCR_AOL_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_PKT_DESCR_AOL_D
}

/* Stage 3 table 0 action */
action tls_bsq_consume(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    GENERATE_TO_S3_K


    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0

}

/*
 * Stage 3 table 1 action
 */
action read_rnmdr_free_pi(rnmdr_free_pi) {
    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    GENERATE_TO_S3_K

    // d for stage 3 table 1 
    modify_field(read_rnmdr_free_d.rnmdr_free_pi, rnmdr_free_pi);
}

/*
 * Stage 3 table 2 action
 */
action read_rnmpr_free_pi(rnmpr_free_pi) {
    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    GENERATE_TO_S3_K

    // d for stage 2 table 2 
    modify_field(read_rnmpr_free_d.rnmpr_free_pi, rnmpr_free_pi);
}

/* Stage 4 Table 0 action */
action tls_read_odesc(PKT_DESCR_AOL_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    modify_field(to_s4_scratch.do_post_cbc_enc, to_s4.do_post_cbc_enc);
    modify_field(to_s4_scratch.do_post_ccm_enc, to_s4.do_post_ccm_enc);

    GENERATE_PKT_DESCR_AOL_D
}

/* Stage 5 Table 0 action */
action tls_read_aad(AAD_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_AAD_FIELDS_D(tls_post_enc_aad_d)
}


/* Stage 6 action */
action tls_queue_sesq(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K


    /* To Stage 6 fields */
    GENERATE_TO_S6_K


    GENERATE_TLSCB_0_D_NON_STG0
}

/* Stage 7 action */
action tls_post_crypto_stats5(STG_POST_CRYPTO_STATS_ACTION_PARAMS) {

    GENERATE_GLOBAL_K


    /* To Stage 7 fields */
    modify_field(to_s7_scratch.rnmdr_free, to_s7.rnmdr_free);
    modify_field(to_s7_scratch.rnmpr_free, to_s7.rnmpr_free);
    modify_field(to_s7_scratch.enc_completions, to_s7.enc_completions);
    modify_field(to_s7_scratch.dec_completions, to_s7.dec_completions);
    modify_field(to_s7_scratch.debug_stage0_3_thread, to_s7.debug_stage0_3_thread);
    modify_field(to_s7_scratch.debug_stage4_7_thread, to_s7.debug_stage4_7_thread);


    GENERATE_STG_POST_CRYPTO_STATS_D
}

