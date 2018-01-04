/* TLS P4 definitions */

#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       read_serq_entry

#define tx_table_s2_t0_action       tls_read_pkt_descr_aol
#define tx_table_s2_t1_action       read_tnmdr_pidx
#define tx_table_s2_t2_action       read_tnmpr_pidx
#define tx_table_s2_t3_action       tls_serq_consume

#define tx_table_s3_t0_action       tls_serq_process
#define tx_table_s3_t1_action       tdesc_alloc
#define tx_table_s3_t2_action       tpage_alloc

#define tx_table_s4_t0_action       tls_read_tls_header

#define tx_table_s5_t0_action       tls_bld_brq5

#define tx_table_s6_t0_action       tls_queue_brq6
#define tx_table_s6_t1_action       tls_write_arq

#define tx_table_s7_t0_action       tls_pre_crypto_stats7

#include "../../common-p4+/common_txdma.p4"
#include "../../cpu-p4+/cpu_rx_common.p4"
#include "tls_txdma_common.p4"


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
        key_desc_index                  : HBM_ADDRESS_WIDTH;
        command_core                    : 4;
        command_mode                    : 4;
        command_op                      : 4;
        command_param                   : 20;
        // TBD: Total used   : 64 bits, pending: 448
        pad                             : 448;
    }
}
#define STG_BLD_BARCO_REQ_ACTION_PARAMS                                                                 \
key_desc_index, command_core, command_mode, command_op, command_param,idesc, odesc
#

#define GENERATE_STG_BLD_BARCO_REQ_D                                                                    \
    modify_field(tls_bld_barco_req_d.key_desc_index, key_desc_index);                                   \
    modify_field(tls_bld_barco_req_d.command_core, command_core);                                       \
    modify_field(tls_bld_barco_req_d.command_mode, command_mode);                                       \
    modify_field(tls_bld_barco_req_d.command_op, command_op);                                           \
    modify_field(tls_bld_barco_req_d.command_param, command_param);

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
        modify_field(tls_global_phv_scratch.write_arq, tls_global_phv.write_arq);                       \
        modify_field(tls_global_phv_scratch.do_pre_ccm_dec, tls_global_phv.do_pre_ccm_dec);             \
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
        write_arq                       : 1;
        do_pre_ccm_dec                  : 1;
        tls_global_pad0                 : 2;
        qstate_addr                     : HBM_ADDRESS_WIDTH;
        tls_hdr_type                    : 8;
        tls_hdr_version_major           : 8;
        tls_hdr_version_minor           : 8;
        tls_hdr_len                     : 16;
        next_tls_hdr_offset             : 16;
    }
}

header_type to_stage_3_phv_t {
    fields {
        idesc                           : ADDRESS_WIDTH;
    }
}

header_type to_stage_4_phv_t {
    fields {
        pad                             : 96;
        debug_dol                       : 8;
    }
}
#define GENERATE_TO_S4_K                                                                \
    modify_field(to_s4_scratch.debug_dol, to_s4.debug_dol);

header_type to_stage_5_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        opage                           : HBM_ADDRESS_WIDTH;
        cur_tls_data_len                : 16;
        debug_dol                       : 8;
        write_arq                       : 1;
        pad                             : 7;
    }
}

#define s4_s6_t0_phv_t  additional_data_t
#define GENERATE_S4_S6_T0  GENERATE_AAD_FIELDS(s4_s6_t0_scratch, s4_s6_t0_phv)


header_type to_stage_6_phv_t {
    fields {
        idesc                           : HBM_ADDRESS_WIDTH;
        odesc                           : HBM_ADDRESS_WIDTH;
        opage                           : HBM_ADDRESS_WIDTH;
        cur_tls_data_len                : 16;
        next_tls_hdr_offset             : 16;
    }
}
 header_type to_stage_7_phv_t {
     fields {
        tnmdr_alloc                     : 8;
        tnmpr_alloc                     : 8;
        enc_requests                    : 8;
        dec_requests                    : 8;
        debug_stage0_3_thread           : 16;
        debug_stage4_7_thread           : 16;
     }
 }

#define GENERATE_TO_S6(TO, FROM)                                                        \
        modify_field(TO.idesc, FROM.idesc);                                             \
        modify_field(TO.odesc, FROM.odesc);                                             \
        modify_field(TO.opage, FROM.opage);                                             \
        modify_field(TO.cur_tls_data_len, FROM.cur_tls_data_len);                       \
        modify_field(TO.next_tls_hdr_offset, FROM.next_tls_hdr_offset);


#define GENERATE_S2_S3_T0                                                                   \
        modify_field(s2_s3_t0_scratch.idesc_aol0_addr, s2_s3_t0_phv.idesc_aol0_addr);       \
        modify_field(s2_s3_t0_scratch.idesc_aol0_offset, s2_s3_t0_phv.idesc_aol0_offset);   \
        modify_field(s2_s3_t0_scratch.idesc_aol0_len, s2_s3_t0_phv.idesc_aol0_len);

header_type s2_s3_t0_phv_t {
    fields {
        idesc_aol0_addr                 : ADDRESS_WIDTH;
        idesc_aol0_offset               : 32;
        pad1                            : 24;
        idesc_aol0_len                  : 32;
    }
}
#define GENERATE_S3_S4_T0                                                               \
        modify_field(s3_s4_t0_scratch.idesc_aol0_addr, s3_s4_t0_phv.idesc_aol0_addr);   \
        modify_field(s3_s4_t0_scratch.idesc_aol0_offset, s3_s4_t0_phv.idesc_aol0_offset);   \
        modify_field(s3_s4_t0_scratch.idesc_aol0_len, s3_s4_t0_phv.idesc_aol0_len);
header_type s3_s4_t0_phv_t {
    fields {
        idesc_aol0_addr                 : ADDRESS_WIDTH;
        idesc_aol0_offset               : 32;
        pad1                            : 24;
        idesc_aol0_len                  : 32;
    }
}

#define GENERATE_S5_S6_T1                                                               \
        modify_field(s5_s6_t1_s2s_scratch.arq_base, s5_s6_t1_s2s.arq_base);             \
        modify_field(s5_s6_t1_s2s_scratch.debug_dol, s5_s6_t1_s2s.debug_dol);

header_type s5_s6_t1_s2s_phv_t {
    fields {
        debug_dol                       : 8;
        pad                             : 8;
        arq_base                        : 32;

    }
}


/* PHV PI storage */
header_type barco_dbell_t {
    fields {
        pi                                  : 32;
    } 
}

header_type odesc_dma_src_t {
    fields {
        odesc                               : 64;
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

header_type pad_to_dma_cmds_t {
    fields {
        pad                     : 0;
    }
}

header_type ring_entry_pad_t {
    fields {
        pad                     : 192;
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

@pragma scratch_metadata
metadata tls_stage_pre_crypto_stats_d_t tls_pre_crypto_stats_d;

@pragma scratch_metadata
metadata serq_entry_d_t SERQ_ENTRY_SCRATCH;

@pragma scratch_metadata
metadata tnmdr_pidx_t TNMDR_PIDX_SCRATCH;

@pragma scratch_metadata
metadata tnmpr_pidx_t TNMPR_PIDX_SCRATCH;

@pragma scratch_metadata
metadata pkt_descr_aol_t PKT_DESCR_AOL_SCRATCH;

@pragma scratch_metadata
metadata tls_header_t TLS_HDR_SCRATCH;

@pragma scratch_metadata
metadata barco_channel_pi_ci_t tls_enc_queue_brq_d;

@pragma pa_header_union ingress to_stage_1 cpu_hdr1
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_1_t cpu_hdr1;


@pragma pa_header_union ingress to_stage_2 cpu_hdr2
@pragma dont_trim
metadata barco_dbell_t barco_dbell;
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_2_t cpu_hdr2;


@pragma pa_header_union ingress to_stage_3 odesc_dma_src
metadata to_stage_3_phv_t to_s3;
@pragma dont_trim
metadata odesc_dma_src_t odesc_dma_src;


@pragma pa_header_union ingress to_stage_4 crypto_iv to_s4
@pragma dont_trim
metadata crypto_iv_t crypto_iv;
metadata to_stage_4_phv_t to_s4;


@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;

@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;

@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;

@pragma pa_header_union ingress  common_t0_s2s s3_s4_t0_phv s4_s6_t0_phv
metadata s2_s3_t0_phv_t s2_s3_t0_phv;
metadata s3_s4_t0_phv_t s3_s4_t0_phv;
metadata s4_s6_t0_phv_t s4_s6_t0_phv;



@pragma pa_header_union ingress common_t1_s2s s5_s6_t1_s2s
metadata s3_t1_s2s_phv_t s3_t1_s2s;
metadata s5_s6_t1_s2s_phv_t s5_s6_t1_s2s;

@pragma pa_header_union ingress common_t2_s2s
metadata s3_t2_s2s_phv_t s3_t2_s2s;


@pragma dont_trim
metadata pkt_descr_aol_t idesc; 
@pragma dont_trim
metadata pkt_descr_aol_t odesc; 
@pragma dont_trim
metadata barco_desc_t barco_desc;
@pragma dont_trim
metadata ring_entry_t ring_entry;
@pragma dont_trim
metadata ccm_header_t ccm_header_with_aad;
@pragma dont_trim
metadata pad_to_dma_cmds_t pad_to_dma_cmds;

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
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata barco_dbell_t barco_db_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;
@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;
@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;

@pragma scratch_metadata
metadata s2_s3_t0_phv_t s2_s3_t0_scratch;
@pragma scratch_metadata
metadata s3_s4_t0_phv_t s3_s4_t0_scratch;
@pragma scratch_metadata
metadata s4_s6_t0_phv_t s4_s6_t0_scratch;

@pragma scratch_metadata
metadata s3_t1_s2s_phv_t s3_t1_s2s_scratch;
@pragma scratch_metadata
metadata s5_s6_t1_s2s_phv_t s5_s6_t1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t2_s2s_phv_t s3_t2_s2s_scratch;


@pragma scratch_metadata
metadata tdesc_alloc_d_t tdesc_alloc_d;
@pragma scratch_metadata
metadata tpage_alloc_d_t tpage_alloc_d;
@pragma scratch_metadata
metadata arq_pi_d_t arq_tx_pi_d;



/* Stage 1 Table 0 action */
action read_serq_entry(SERQ_ENTRY_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_SERQ_ENTRY_D
}



/* Stage 2 table 0 action */
action tls_read_pkt_descr_aol(PKT_DESCR_AOL_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_PKT_DESCR_AOL_D
}

/*
 * Stage 2 table 1 action
 */
action read_tnmdr_pidx(TNMDR_PIDX_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMDR_PIDX_D
}

/*
 * Stage 2 table 2 action
 */
action read_tnmpr_pidx(TNMPR_PIDX_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMPR_PIDX_D
}

/* Stage 2 table 3 action */
action tls_serq_consume(TLSCB_0_PARAMS_NON_STG0) {


    GENERATE_GLOBAL_K

    /* D vector */
    GENERATE_TLSCB_0_D_NON_STG0

}

/* Stage 3 table 0 action */
action tls_serq_process(TLSCB_1_PARAMS) {


    GENERATE_GLOBAL_K

    GENERATE_S2_S3_T0

    /* To Stage 3 fields */
    modify_field(to_s3_scratch.idesc, to_s3.idesc);

    /* Unionized PHV mem with to_s3 used in later stage as scratch */
    /*modify_field(barco_db_scratch.pi, barco_dbell.pi); */

    /* D vector */
    GENERATE_TLSCB_1_D

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

    GENERATE_TLSCB_1_D
}

/* Stage 4 action */
action tls_read_tls_header(TLS_HDR_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_S3_S4_T0

    GENERATE_TO_S4_K

    GENERATE_TLS_HDR_D
}


/* Stage 5 action */
action tls_bld_brq5(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    /* To Stage 5 fields */
    modify_field(to_s5_scratch.idesc, to_s5.idesc);
    modify_field(to_s5_scratch.odesc, to_s5.odesc);
    modify_field(to_s5_scratch.debug_dol, to_s5.debug_dol);

    GENERATE_TLSCB_0_D_NON_STG0
}


/* Stage 6 action */
action tls_queue_brq6(BARCO_CHANNEL_PARAMS) {

    GENERATE_GLOBAL_K

    /* Stage 4 to 6 */
    GENERATE_S4_S6_T0

    /* To Stage 6 fields */
    GENERATE_TO_S6(to_s6_scratch, to_s6)


    GENERATE_BARCO_CHANNEL_D
}

/*
 * Stage 6 table 1 action
 */
action tls_write_arq(ARQ_PI_PARAMS) {

    // k + i for stage 6
    GENERATE_GLOBAL_K

    GENERATE_S5_S6_T1

    // from to_stage 6
    modify_field(to_s6_scratch.idesc, to_s6.idesc);
    modify_field(to_s6_scratch.odesc, to_s6.odesc);
    modify_field(to_s6_scratch.opage, to_s6.opage);
    modify_field(to_s6_scratch.cur_tls_data_len, to_s6.cur_tls_data_len);
    modify_field(to_s6_scratch.next_tls_hdr_offset, to_s6.next_tls_hdr_offset);


    // from stage to stage

    // d for stage 6 table 1
    GENERATE_ARQ_PI_D(arq_tx_pi_d)
}


/* Stage 7 action */
action tls_pre_crypto_stats7(STG_PRE_CRYPTO_STATS_ACTION_PARAMS) {

    GENERATE_GLOBAL_K


    /* To Stage 7 fields */
    modify_field(to_s7_scratch.tnmdr_alloc, to_s7.tnmdr_alloc);
    modify_field(to_s7_scratch.tnmpr_alloc, to_s7.tnmpr_alloc);
    modify_field(to_s7_scratch.enc_requests, to_s7.enc_requests);
    modify_field(to_s7_scratch.dec_requests, to_s7.dec_requests);
    modify_field(to_s7_scratch.debug_stage0_3_thread, to_s7.debug_stage0_3_thread);
    modify_field(to_s7_scratch.debug_stage4_7_thread, to_s7.debug_stage4_7_thread);




    GENERATE_STG_PRE_CRYPTO_STATS_D
}
