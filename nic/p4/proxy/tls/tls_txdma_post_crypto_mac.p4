/* TLS P4 definitions */


#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       tls_rx_bsq

#define tx_table_s2_t0_action       tls_bsq_consume
#define tx_table_s2_t1_action       tls_read_tnmdr
#define tx_table_s2_t2_action       tls_read_tnmpr
#define tx_table_s2_t3_action       tls_read_idesc


#define tx_table_s3_t0_action       tls_bld_brq3
#define tx_table_s3_t1_action       tls_tdesc_alloc
#define tx_table_s3_t2_action       tls_tpage_alloc
#define tx_table_s3_t3_action       tls_read_aad

#define tx_table_s4_t0_action       tls_queue_brq4
#define tx_table_s4_t1_action       tls_read_random_iv

#define tx_table_s5_t0_action       tls_post_crypto_stats5

#include "../../common-p4+/common_txdma.p4"
#include "tls_txdma_common.p4"

/* Per stage D-vector Definitions */

// d for stage 2 table 1
header_type tdesc_alloc_d_t {
    fields {
        desc                    : 64;
        pad                     : 448;
    }
}

// d for stage 2 table 2
header_type tpage_alloc_d_t {
    fields {
        page                    : 64;
        pad                     : 448;
    }
}

#define GENERATE_GLOBAL_K                                                                               \
        modify_field(tls_global_phv_scratch.fid, tls_global_phv.fid);                                   \
        modify_field(tls_global_phv_scratch.dec_flow, tls_global_phv.dec_flow);                         \
        modify_field(tls_global_phv_scratch.barco_op_failed, tls_global_phv.barco_op_failed);           \
        modify_field(tls_global_phv_scratch.pending_rx_bsq, tls_global_phv.pending_rx_bsq);             \
        modify_field(tls_global_phv_scratch.pad, tls_global_phv.pad);                                   \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);


/* Global PHV definition */
header_type tls_global_phv_t {
    fields {
        fid                             : 16;
        dec_flow                        : 8;
        barco_op_failed                 : 1;
        pending_rx_bsq                  : 1;
        pad                             : 6;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
    }
}


header_type to_stage_3_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        ipage                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        tls_hdr_len                     : 16;
	debug_dol                       : 8;
    }
}

#define GENERATE_TO_S3_K                                                                                \
    modify_field(to_s3_scratch.idesc, to_s3.idesc);                                                     \
    modify_field(to_s3_scratch.odesc, to_s3.odesc);                                                     \
    modify_field(to_s3_scratch.ipage, to_s3.ipage);                                                     \
    modify_field(to_s3_scratch.tls_hdr_len, to_s3.tls_hdr_len);                                         \
    modify_field(to_s3_scratch.debug_dol, to_s3.debug_dol);                                             

header_type to_stage_4_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        opage                           : HBM_ADDRESS_WIDTH;
        debug_dol                       : 8;
    }
}

#define GENERATE_TO_S4_K                                                                                \
    modify_field(to_s4_scratch.idesc, to_s4.idesc);                                                     \
    modify_field(to_s4_scratch.odesc, to_s4.odesc);                                                     \
    modify_field(to_s4_scratch.opage, to_s4.opage);                                                     \
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);                                             


header_type to_stage_5_phv_t {
    fields {
        tnmdr_alloc                     : 8;
        tnmpr_alloc                     : 8;
        mac_completions                 : 8;
        enc_requests                    : 8;
        debug_stage0_3_thread           : 16;
        debug_stage4_7_thread           : 16;
    }
}

/* PHV PI storage */
header_type barco_dbell_t {
    fields {
        pi                                  : 32;
    } 
}

header_type doorbell_data_pad_t {
    fields {
        pad                             : 120;
    }
}

header_type barco_desc_pad_t {
    fields {
        pad                     : 64;
    }
}

header_type crypto_random_iv_t {
    fields {
        explicit_iv             : 128;
    }
}

header_type tls_pkt_header_t {
    fields {
        tls_hdr_type                        : 8;
        tls_hdr_version_major               : 8;
        tls_hdr_version_minor               : 8;
        tls_hdr_len                         : 16;
    }
}

@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_1_t tlscb_1_d;

@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;

@pragma scratch_metadata
metadata pkt_descr_aol_t PKT_DESCR_AOL_SCRATCH;

@pragma scratch_metadata
metadata tls_stage_post_crypto_stats_d_t tls_post_crypto_stats_d;

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;

@pragma scratch_metadata
metadata read_tnmdr_d_t read_tnmdr_d;
@pragma scratch_metadata
metadata read_tnmpr_d_t read_tnmpr_d;
@pragma scratch_metadata
metadata tdesc_alloc_d_t tdesc_alloc_d;
@pragma scratch_metadata
metadata tpage_alloc_d_t tpage_alloc_d;

@pragma scratch_metadata
metadata additional_data_t tls_post_mac_aad_d;

@pragma scratch_metadata
metadata barco_channel_pi_ci_t tls_enc_queue_brq_d;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;


@pragma dont_trim
metadata pkt_descr_aol_t idesc;
@pragma dont_trim
metadata pkt_descr_aol_t odesc;
@pragma dont_trim
metadata barco_desc_t barco_desc;
@pragma dont_trim
metadata barco_desc_pad_t barco_desc_pad;
@pragma dont_trim
metadata barco_dbell_t barco_dbell;
@pragma dont_trim
metadata tls_pkt_header_t tls_hdr;
@pragma dont_trim
metadata crypto_random_iv_t crypto_random_iv;
@pragma dont_trim
metadata doorbell_data_raw_t db_data;
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
action tls_bsq_consume(TLSCB_0_PARAMS_NON_STG0) {
    GENERATE_GLOBAL_K

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0
}

/*
 * Stage 2 table 1 action
 */
action tls_read_tnmdr(tnmdr_pidx) {
    // d for stage 2 table 1 read-tnmdr-idx
    modify_field(read_tnmdr_d.tnmdr_pidx, tnmdr_pidx);
}

/*
 * Stage 2 table 2 action
 */
action tls_read_tnmpr(tnmpr_pidx) {
    // d for stage 2 table 2 read-tnmpr-idx
    modify_field(read_tnmpr_d.tnmpr_pidx, tnmpr_pidx);
}

/* Stage 2 table 3 action */
action tls_read_idesc(PKT_DESCR_AOL_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_PKT_DESCR_AOL_D
}

/* Stage 3 table 0 action */
action tls_bld_brq3(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* To Stage 3 table 0 fields */
    modify_field(to_s3_scratch.idesc, to_s3.idesc);
    modify_field(to_s3_scratch.ipage, to_s3.ipage);
    modify_field(to_s3_scratch.odesc, to_s3.odesc);
    modify_field(to_s3_scratch.debug_dol, to_s3.debug_dol);

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0
}

/*
 * Stage 3 table 1 action
 */
action tls_tdesc_alloc(desc, pad) {
    // k + i for stage 3 table 1

    // from to_stage 3
    modify_field(to_s3_scratch.debug_dol, to_s3.debug_dol);

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 3 table 1
    modify_field(tdesc_alloc_d.desc, desc);
    modify_field(tdesc_alloc_d.pad, pad);
}

/*
 * Stage 3 table 2 action
 */
action tls_tpage_alloc(page, pad) {
    // k + i for stage 3 table 2

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 3 table 2
    modify_field(tpage_alloc_d.page, page);
    modify_field(tpage_alloc_d.pad, pad);
}

/* Stage 3 Table 3 action */
action tls_read_aad(AAD_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    GENERATE_TO_S3_K

    /* D vector */
    GENERATE_AAD_FIELDS_D(tls_post_mac_aad_d)
}

/* Stage 4 Table 0 action */
action tls_queue_brq4(BARCO_CHANNEL_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 4 fields */
    GENERATE_TO_S4_K

#if 0
    modify_field(to_s4_scratch.idesc, to_s4.idesc);
    modify_field(to_s4_scratch.odesc, to_s4.odesc);
    modify_field(to_s4_scratch.opage, to_s4.opage);
    modify_field(to_s4_scratch.cur_tls_data_len, to_s4.cur_tls_data_len);
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);
#endif

    GENERATE_BARCO_CHANNEL_D
}

/* Stage 4 table 1 action */
action tls_read_random_iv(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0
}

/* Stage 5 table 0 action */
action tls_post_crypto_stats5(STG_POST_CRYPTO_STATS_ACTION_PARAMS) {

    GENERATE_GLOBAL_K


    /* To Stage 5 fields */
    modify_field(to_s5_scratch.tnmdr_alloc, to_s5.tnmdr_alloc);
    modify_field(to_s5_scratch.tnmpr_alloc, to_s5.tnmpr_alloc);
    modify_field(to_s5_scratch.mac_completions, to_s5.mac_completions);
    modify_field(to_s5_scratch.enc_requests, to_s5.enc_requests);
    modify_field(to_s5_scratch.debug_stage0_3_thread, to_s5.debug_stage0_3_thread);
    modify_field(to_s5_scratch.debug_stage4_7_thread, to_s5.debug_stage4_7_thread);

    GENERATE_STG_POST_CRYPTO_STATS_D
}