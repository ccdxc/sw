/* TLS P4 definitions */

#include "../../common-p4+/common_txdma_dummy.p4"

/******************************************************************************
 * Table names
 * 
 * Table names have to alphabetically be in chronological order (to match with
 * common program table names), so they need * to be prefixed by stage# and
 * table#
 *****************************************************************************/
//#define tx_table_s0_t0 tx_tls_macgen_s0_t0

//#define tx_table_s1_t0 tx_tls_macgen_s1_t0
//#define tx_table_s1_t1 tx_tls_macgen_s1_t1
//#define tx_table_s1_t2 tx_tls_macgen_s1_t2

//#define tx_table_s2_t0 tx_tls_macgen_s2_t0
//#define tx_table_s2_t1 tx_tls_macgen_s2_t1

//#define tx_table_s3_t0 tx_tls_macgen_s3_t0

//#define tx_table_s4_t0 tx_tls_macgen_s4_t0

//#define tx_table_s5_t0 tx_tls_macgen_s5_t0

//#define tx_table_s6_t0 tx_tls_macgen_s6_t0

//#define tx_table_s7_t0 tx_tls_macgen_s7_t0


/******************************************************************************
 * Action names
 *****************************************************************************/

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       read_tls_stg1_7

#define tx_table_s2_t0_action       tls_rx_serq 
#define tx_table_s2_t3_action       tls_read_pkt_descr_aol

#define tx_table_s3_t0_action       tls_serq_consume

#define tx_table_s4_t0_action       tls_bld_brq4

#define tx_table_s5_t0_action       tls_queue_brq5

#define tx_table_s6_t0_action       tls_pre_crypto_stats6

#include "../../common-p4+/common_txdma.p4"
#include "tls_txdma_common.p4"

/* Per stage D-vector Definitions */

// d for stage 2 table 1

// d for stage 2 table 2

// d for stage 3 table 1

// d for stage 3 table 2

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
        serq_ci                         : 16;
        debug_dol                       : 8;
    }
}

header_type to_stage_4_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        barco_hmac_key_desc_index       : 32;
        debug_dol                       : 8;
    }
}

header_type to_stage_5_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        ipage                           : HBM_ADDRESS_WIDTH;
        cur_tls_data_len                : 16;
        debug_dol                       : 8;
    }
}

header_type to_stage_6_phv_t {
    fields {
        mac_requests                    : 8;
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

#define s2_s5_t0_phv_t  additional_data_t
#define GENERATE_S2_S5_T0  GENERATE_AAD_FIELDS(s2_s5_t0_scratch, s2_s5_t0_phv)

header_type barco_desc_pad_t {
    fields {
        pad                     : 96;
    }
}


@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_1_t tlscb_1_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t PKT_DESCR_AOL_SCRATCH;

@pragma scratch_metadata
metadata tls_stage_pre_crypto_stats_d_t tls_pre_crypto_stats_d;

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

@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;

@pragma pa_header_union ingress  common_t0_s2s 
metadata s2_s5_t0_phv_t s2_s5_t0_phv;


@pragma dont_trim
metadata pkt_descr_aol_t idesc; 
@pragma dont_trim
metadata barco_desc_t barco_desc;
@pragma dont_trim
metadata barco_desc_pad_t   barco_desc_pad;
@pragma dont_trim
metadata barco_dbell_t barco_dbell;
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
metadata s2_s5_t0_phv_t    s2_s5_t0_scratch;

@pragma scratch_metadata
metadata barco_channel_pi_ci_t tls_enc_queue_brq_d;

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
 * Stage 2 table 3 action
 */
action tls_read_pkt_descr_aol(PKT_DESCR_AOL_ACTION_PARAMS) {
    // d for stage 2 table 3
    GENERATE_PKT_DESCR_AOL_D
}

/* Stage 3 table 0 action */
action tls_serq_consume(TLSCB_0_PARAMS_NON_STG0) {


    GENERATE_GLOBAL_K

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.serq_ci, to_s3.serq_ci);

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0

}

/* Stage 3 table 3 action */
action tls_stage3(TLSCB_1_PARAMS) {

    GENERATE_GLOBAL_K

    /* To Stage 2 fields */
    modify_field(to_s2_scratch.serq_ci, to_s2.serq_ci);
    modify_field(to_s2_scratch.idesc, to_s2.idesc);

    GENERATE_TLSCB_1_D
}

/* Stage 4 table 0 action */
action tls_bld_brq4(TLSCB_0_7_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* To Stage 4 table 0 fields */
    modify_field(to_s4_scratch.idesc, to_s4.idesc);
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);
    modify_field(to_s4_scratch.barco_hmac_key_desc_index, to_s4.barco_hmac_key_desc_index);

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0
}

/* Stage 4 table 1 action */
action tls_read_random_iv(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* To Stage 4 table 1 fields */
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0
}

/* Stage 5 table 0 action */
action tls_queue_brq5(BARCO_CHANNEL_PARAMS) {

    GENERATE_GLOBAL_K



    /* To Stage 5 fields */
    modify_field(to_s5_scratch.idesc, to_s5.idesc);
    modify_field(to_s5_scratch.cur_tls_data_len, to_s5.cur_tls_data_len);
    modify_field(to_s5_scratch.debug_dol, to_s5.debug_dol);
    modify_field(to_s5_scratch.ipage, to_s5.ipage);

    GENERATE_S2_S5_T0

    GENERATE_BARCO_CHANNEL_D
}


/* Stage 6 action */
action tls_pre_crypto_stats6(STG_PRE_CRYPTO_STATS_ACTION_PARAMS) {

    GENERATE_GLOBAL_K


    /* To Stage 6 fields */
    modify_field(to_s6_scratch.mac_requests, to_s6.mac_requests);
    modify_field(to_s6_scratch.debug_stage0_3_thread, to_s6.debug_stage0_3_thread);
    modify_field(to_s6_scratch.debug_stage4_7_thread, to_s6.debug_stage4_7_thread);




    GENERATE_STG_PRE_CRYPTO_STATS_D
}