/* TLS TXDMA Pre-Crypto Decryption RX Reassembly - P4 definitions */

#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action       read_tls_stg0

#define tx_table_s1_t0_action       read_serq_entry

#define tx_table_s2_t0_action       check_tls_hdr

#define tx_table_s3_t0_action       read_tls_hdr_or_auth_tag

#define tx_table_s4_t0_action       update_tlscb_records_state
#define tx_table_s4_t1_action       read_tnmdr_pidx_odesc
#define tx_table_s4_t2_action       read_tnmpr_pidx_opage
#define tx_table_s4_t3_action       read_auth_tag


#define tx_table_s5_t0_action       process_auth_tag
#define tx_table_s5_t1_action       odesc_alloc
#define tx_table_s5_t2_action       opage_alloc
#define tx_table_s5_t3_action       read_tnmdr_pidx_idesc

#define tx_table_s6_t0_action       read_barco_semaphore
#define tx_table_s6_t1_action       read_cached_odesc_opage
#define tx_table_s6_t3_action       idesc_alloc

#define tx_table_s7_t0_action       queue_barco

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

#define STG_QUEUE_BRQ_ACTION_PARAMS                                                                 \
rsvd,cosA,cosB,cos_sel,eval_last,host,total,pid, pi_0,ci_0
#

#define GENERATE_GLOBAL_K                                                                       \
        modify_field(tls_global_phv_scratch.qstate_addr, tls_global_phv.qstate_addr);           \
        modify_field(tls_global_phv_scratch.A0, tls_global_phv.A0);                             \
        modify_field(tls_global_phv_scratch.O0, tls_global_phv.O0);                             \
        modify_field(tls_global_phv_scratch.L0, tls_global_phv.L0);                             \
        modify_field(tls_global_phv_scratch.tls_rec_len, tls_global_phv.tls_rec_len);           \
        modify_field(tls_global_phv_scratch.dec_flow, tls_global_phv.dec_flow);                 \
        modify_field(tls_global_phv_scratch.write_arq, tls_global_phv.write_arq);               \
        modify_field(tls_global_phv_scratch.first_segment, tls_global_phv.first_segment);       \
        modify_field(tls_global_phv_scratch.no_payload, tls_global_phv.no_payload);             \
        modify_field(tls_global_phv_scratch.enqueue_barco, tls_global_phv.enqueue_barco);       \
        modify_field(tls_global_phv_scratch.skip_dma, tls_global_phv.skip_dma);                 \
        modify_field(tls_global_phv_scratch.segment_pending, tls_global_phv.segment_pending);


/* Global PHV definition */
/* Max size: 128 bits */
header_type tls_global_phv_t {
    fields {
        qstate_addr                     : HBM_ADDRESS_WIDTH;
        A0                              : HBM_ADDRESS_WIDTH;
        O0                              : 16;
        L0                              : 16;
        tls_rec_len                     : 15;
        dec_flow                        : 1;
        write_arq                       : 1;
        first_segment                   : 1; /* First segment of the TLS record */
        no_payload                      : 1; /* No payload in the current pass */
        enqueue_barco                   : 1; /* Enqueue to Barco, indicates TLS
                                                record completion */
        skip_dma                        : 1; /* No DMA in this pass */
        segment_pending                 : 1; /* Bytes pending in the current segment */
        /* Used: 118b, Pending: 10b */
    }
}



/* PHV PI storage */
header_type barco_dbell_t {
    fields {
        pi                                  : 32;
    } 
}

/* Custom definition to unionize with p4_2_p4plus_app_header_t */
header_type pkt_descr_aol_padded_t {
    fields {
        pad: 8;
        A0 : 64;
        O0 : 32;
        L0 : 32;
        A1 : 64;
        O1 : 32;
        L1 : 32;
        A2 : 64;
        O2 : 32;
        L2 : 32;
        next_addr : 64;
        next_pkt : 64;

    }
}

/* PHV scratch used use SRC in PHV2MEM commands */
header_type phv_scratch_t {
    fields {
/* TLS CB record linked-list */
        new_tail_addr       : ADDRESS_WIDTH;
/* ipage ref-counting */
        ipage_ref_count     : 16;
/* TLS doorbell for pending bytes in segment DB */
        pend_dbell_data     : 64;
    /* Total : 160, Used : 144, Available : 16 */
    }
}

header_type odesc_opage_ref_t {
    fields {
        odesc           : ADDRESS_WIDTH;
        opage           : ADDRESS_WIDTH;
    /* Total : 160, Used : 128 , Available : 32 */
    }
}

header_type pad_to_dma_cmds_t {
    fields {
        pad                     : 96;
    }
}

@pragma scratch_metadata
metadata tlscb_0_t tlscb_0_d;

@pragma scratch_metadata
metadata tlscb_records_state_t  TLSCB_RECORDS_STATE_SCRATCH;

@pragma scratch_metadata
metadata pkt_descr_aol_t PKT_DESCR_AOL_SCRATCH;

/* to Stage PHV area */
@pragma pa_header_union ingress to_stage_0 aad
@pragma dont_trim
metadata additional_data_t aad;

@pragma pa_header_union ingress to_stage_1 iv
@pragma dont_trim
metadata crypto_iv_t iv;


@pragma pa_header_union ingress to_stage_2 auth_tag
@pragma dont_trim
metadata aead_auth_tag_t auth_tag;


/* to stage 3 */
header_type to_stage_3_phv_t {
    fields {
        idesc           : HBM_ADDRESS_WIDTH;
        cur_tls_record_pend_len     : 16;
        bytes_avail     : 4;
        bytes_to_read   : 4;
        tls_hdr_read    : 1;
        auth_tag_read   : 1;
        /* Used : Pending: 54 */
    }
}
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

#define GENERATE_TO_S3                                                      \
    modify_field(to_s3_scratch.idesc, to_s3.idesc);                         \
    modify_field(to_s3_scratch.cur_tls_record_pend_len, to_s3.cur_tls_record_pend_len);             \
    modify_field(to_s3_scratch.bytes_avail, to_s3.bytes_avail);             \
    modify_field(to_s3_scratch.bytes_to_read, to_s3.bytes_to_read);         \
    modify_field(to_s3_scratch.tls_hdr_read, to_s3.tls_hdr_read);           \
    modify_field(to_s3_scratch.auth_tag_read, to_s3.auth_tag_read);

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

/* to stage 4 */
header_type to_stage_4_phv_t {
    fields {
        idesc                   : HBM_ADDRESS_WIDTH;
        segment_bytes_consumed  : 16;
        auth_tag_bytes_to_read  : 4;
        bytes_read      : 4;
        auth_tag_to_read        : 1;
        tls_hdr_read    : 1;
        auth_tag_read   : 1;
        alloc_idesc     : 1;
        /* Used : 12 Pending: 116 */
    }
}
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

#define GENERATE_TO_S4                                                      \
    modify_field(to_s4_scratch.idesc, to_s4.idesc);                         \
    modify_field(to_s4_scratch.segment_bytes_consumed, to_s4.segment_bytes_consumed);\
    modify_field(to_s4_scratch.auth_tag_bytes_to_read, to_s4.auth_tag_bytes_to_read);\
    modify_field(to_s4_scratch.bytes_read, to_s4.bytes_read);               \
    modify_field(to_s4_scratch.auth_tag_to_read, to_s4.auth_tag_to_read);   \
    modify_field(to_s4_scratch.tls_hdr_read, to_s4.tls_hdr_read);           \
    modify_field(to_s4_scratch.auth_tag_read, to_s4.auth_tag_read);         \
    modify_field(to_s4_scratch.alloc_idesc, to_s4.alloc_idesc);

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

/* Stage 4 Table 1 D scratch */
@pragma scratch_metadata
metadata tnmdr_pidx_t TNMDR_PIDX_SCRATCH;

/* Stage 4 Table 2 D scratch */
@pragma scratch_metadata
metadata tnmpr_pidx_t TNMPR_PIDX_SCRATCH;

/* to stage 5 */
header_type to_stage_5_phv_t {
    fields {
        /* From Stage 4 Table 3 */
        auth_tag_bytes_read     : 4;
        auth_tag_read           : 1;
        /* Used : 5, Pending : 123 */
    }
}
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;

#define GENERATE_TO_S5                                                          \
    modify_field(to_s5_scratch.auth_tag_bytes_read, to_s5.auth_tag_bytes_read); \
    modify_field(to_s5_scratch.auth_tag_read, to_s5.auth_tag_read);


@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

/* to stage 6 */
header_type to_stage_6_phv_t {
    fields {
        fid                     : 16;
        queue_to_barco          : 1;
        /* Used : 17 , Pending : 111 */
    }
}
@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;

#define GENERATE_TO_S6                                                          \
    modify_field(to_s6_scratch.fid, to_s6.fid);                                 \
    modify_field(to_s6_scratch.queue_to_barco, to_s6.queue_to_barco);

@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;

/* to stage 7 */
header_type to_stage_7_phv_t {
    fields {
        odesc           : HBM_ADDRESS_WIDTH;
        opage           : HBM_ADDRESS_WIDTH;
        curr_idesc      : HBM_ADDRESS_WIDTH;
        idesc           : HBM_ADDRESS_WIDTH;
        /* Used : 128 , Pending : 0 */
    }
}

@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;

#define GENERATE_TO_S7                                                          \
    modify_field(to_s7_scratch.odesc, to_s7.odesc);                             \
    modify_field(to_s7_scratch.opage, to_s7.opage);                             \
    modify_field(to_s7_scratch.curr_idesc, to_s7.curr_idesc);                   \
    modify_field(to_s7_scratch.idesc, to_s7.idesc);

@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;



@pragma pa_header_union ingress common_global
metadata tls_global_phv_t tls_global_phv;



header_type s6_t0_to_s7_t0_t {
    fields {
        rec_head_idesc      : 64;
        producer_idx        : 32;
        fid                 : 16;
    } 
}
#define S6_T0_TO_S7_T0_SCRATCH  s6_t0_to_s7_t0_scratch
#define GENERATE_S6_T0_TO_S7_T0_K                   \
    modify_field(S6_T0_TO_S7_T0_SCRATCH.rec_head_idesc, s6_t0_to_s7_t0.rec_head_idesc); \
    modify_field(S6_T0_TO_S7_T0_SCRATCH.producer_idx, s6_t0_to_s7_t0.producer_idx); \
    modify_field(S6_T0_TO_S7_T0_SCRATCH.fid, s6_t0_to_s7_t0.fid);

@pragma scratch_metadata
metadata s6_t0_to_s7_t0_t S6_T0_TO_S7_T0_SCRATCH;

@pragma pa_header_union ingress common_t0_s2s s4_t3_to_s5_t0 s6_t0_to_s7_t0
metadata bytes_16_t s2_to_s3_to_s4_t0;
metadata bytes_16_t s4_t3_to_s5_t0;
metadata s6_t0_to_s7_t0_t   s6_t0_to_s7_t0;

@pragma pa_header_union ingress common_t1_s2s
@pragma dont_trim
metadata odesc_opage_ref_t odesc_opage_ref;

@pragma pa_header_union ingress common_t2_s2s
@pragma dont_trim
metadata phv_scratch_t  phv_scratch;

header_type pkt_descr_one_aol_t {
    fields {
        A0          : 64;
        O0          : 32;
        L0          : 32;
    }
}

/* Truncated AOL due to lack for PHV storage */
@pragma pa_header_union ingress common_t3_s2s
@pragma dont_trim
metadata pkt_descr_one_aol_t curr_idesc;


/* Start of scratch in PHV */
@pragma dont_trim
metadata pkt_descr_aol_t idesc; 

@pragma dont_trim
metadata pkt_descr_aol_t odesc; 

@pragma dont_trim
metadata barco_desc_t barco_desc;

@pragma dont_trim
metadata ring_entry_t ring_entry;

@pragma dont_trim
metadata barco_dbell_t barco_dbell;

@pragma dont_trim
metadata pad_to_dma_cmds_t pad_to_dma_cmds;

/* Start of DMA commands */

/* PHV Scratch: phv_scratch_new_tail_addr  */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_tls_rec_list;

/* PHV Scratch: idesc_A0 -> idesc_next_pkt */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_idesc;

/* PHV Scratch: odesc_opage_ref_odesc - odesc_opage_ref_opage */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_odesc_opage_ref;

/* PHV Scratch: curr_idesc_A0 - curr_idesc_next_pkt */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_curr_idesc;

/* PHV Scratch: phv_scratch_pend_dbell_data - phv_scratch_pend_dbell_data */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_tls_txdma_penddbell;

/* PHV Scratch: phv_scratch_ipage_ref_count */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_ipage_refcount;

/* PHV Scratch: odesc_A0 -> odesc_next_pkt */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_odesc;

/* PHV Scratch: aad_aad_seq_num - auth_tag_auth_tag */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_aad_iv_authtag;

/* PHV Scratch: barco_desc_input_list_address - barco_desc_second_key_desc_index */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_barco_desc;

/* PHV Scratch: barco_dbell_pi */
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_barco_pi_write;

@pragma scratch_metadata
metadata tls_global_phv_t tls_global_phv_scratch;


@pragma scratch_metadata
metadata serq_entry_new_t SERQ_ENTRY_NEW_SCRATCH;

/* Stage 1 Table 0 action */
action read_serq_entry(SERQ_ENTRY_NEW_ACTION_PARAMS) {

    GENERATE_GLOBAL_K

    GENERATE_SERQ_ENTRY_NEW_D
}


/* Stage 2 Table 0 action */
action check_tls_hdr(TLSCB_RECORDS_STATE_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TLSCB_RECORDS_STATE
}




@pragma scratch_metadata
metadata bytes_16_t BYTES_16_SCRATCH;

/* Stage 3 Table 0 action */
action read_tls_hdr_or_auth_tag(BYTES_16_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_BYTES_16_K(s2_to_s3_to_s4_t0)

    GENERATE_TO_S3

    GENERATE_BYTES_16_D
}


@pragma scratch_metadata
metadata tls_header_t TLS_HDR_SCRATCH;

/* Stage 4 Table 0 Action */
action update_tlscb_records_state(TLSCB_RECORDS_STATE_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_BYTES_16_K(s2_to_s3_to_s4_t0)

    GENERATE_TO_S4

    GENERATE_TLSCB_RECORDS_STATE
}

/* Stage 4 Table 1 Action */
action read_tnmdr_pidx_odesc(TNMDR_PIDX_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMDR_PIDX_D
}

/* Stage 4 Table 2 Action */
action read_tnmpr_pidx_opage(TNMPR_PIDX_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMPR_PIDX_D
}

/* Stage 4 Table 3 Action */
action read_auth_tag(BYTES_16_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TO_S4

    GENERATE_BYTES_16_D
}

/* Stage 5 Table 0 Action */
action process_auth_tag(TLSCB_RECORDS_STATE_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_BYTES_16_K(s4_t3_to_s5_t0)

    GENERATE_TO_S5

    GENERATE_TLSCB_RECORDS_STATE
}

@pragma scratch_metadata
metadata tnmdr_entry_t TNMDR_ENTRY_SCRATCH;

/* Stage 5 Table 1 Action */
action odesc_alloc(TNMDR_ENTRY_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMDR_ENTRY_D
}

@pragma scratch_metadata
metadata tnmpr_entry_t TNMPR_ENTRY_SCRATCH;

/* Stage 5 Table 2 Action */
action opage_alloc(TNMPR_ENTRY_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMPR_ENTRY_D
}

/* Stage 5 Table 3 Action */
action read_tnmdr_pidx_idesc(TNMDR_PIDX_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMDR_PIDX_D
}

@pragma scratch_metadata
metadata semaphore_t SEMAPHORE_SCRATCH;

/* Stage 6 Table 0 Action */
action read_barco_semaphore(SEMAPHORE_ACTION_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TO_S6

    GENERATE_SEMAPHORE_D
}

#define ODESC_OPAGE_CACHE_PARAMS    odesc, opage
#define ODESC_OPAGE_CACHE_SCRATCH   odesc_opage_cache_scratch
#define GENERATE_ODESC_OPAGE_CACHE_D                                \
    modify_field(ODESC_OPAGE_CACHE_SCRATCH.odesc, odesc);           \
    modify_field(ODESC_OPAGE_CACHE_SCRATCH.opage, opage);
 

@pragma scratch_metadata
metadata odesc_opage_ref_t ODESC_OPAGE_CACHE_SCRATCH;

/* Stage 6 Table 1 Action */
action read_cached_odesc_opage(ODESC_OPAGE_CACHE_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_ODESC_OPAGE_CACHE_D
}


/* Stage 6 Table 3 Action */
action idesc_alloc(TNMDR_ENTRY_PARAMS) {
    GENERATE_GLOBAL_K

    GENERATE_TNMDR_ENTRY_D
}


/* Stage 7 Table 0 Action */
action queue_barco(TLSCB_0_PARAMS_NON_STG0) {

    GENERATE_GLOBAL_K

    GENERATE_TO_S7

    GENERATE_S6_T0_TO_S7_T0_K

    GENERATE_TLSCB_0_D_NON_STG0
}
