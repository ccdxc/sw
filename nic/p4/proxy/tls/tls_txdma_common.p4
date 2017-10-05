
// tlscb for stage 0
header_type tlscb_0_t {
    fields {
        pc                             : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // 4 Bytes SERQ ring
        CAPRI_QSTATE_HEADER_RING(0)
        // 4 Bytes BSQ ring
        CAPRI_QSTATE_HEADER_RING(1)

        fid                             : 16;
        serq_base                       : HBM_ADDRESS_WIDTH;
        sw_serq_ci                      : 16;
        serq_prod_ci_addr               : HBM_ADDRESS_WIDTH;

        sesq_base                       : HBM_ADDRESS_WIDTH;
        sw_sesq_pi                      : 16;
        sw_sesq_ci                      : 16;

        sw_bsq_ci                       : 16;
        dec_flow                        : 8;
        debug_dol                       : 32;

        barco_command                   : 32;
        barco_key_desc_index            : 32;

        salt                            : 32;
        explicit_iv                     : 64;
        // TBD: Total used   : 504 bits, pending: 8
    }
}

#define TLSCB_0_PARAMS                                                                                  \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, fid, serq_base,         \
sw_serq_ci, serq_prod_ci_addr, sesq_base, sw_sesq_pi, sw_sesq_ci, sw_bsq_ci, dec_flow, debug_dol,       \
barco_command, barco_key_desc_index, salt, explicit_iv

#define GENERATE_TLSCB_0_D                                                                               \
    modify_field(tlscb_0_d.rsvd, rsvd);                                                                  \
    modify_field(tlscb_0_d.cosA, cosA);                                                                  \
    modify_field(tlscb_0_d.cosB, cosB);                                                                  \
    modify_field(tlscb_0_d.cos_sel, cos_sel);                                                            \
    modify_field(tlscb_0_d.eval_last, eval_last);                                                        \
    modify_field(tlscb_0_d.host, host);                                                                  \
    modify_field(tlscb_0_d.total, total);                                                                \
    modify_field(tlscb_0_d.pid, pid);                                                                    \
    modify_field(tlscb_0_d.pi_0, pi_0);                                                                  \
    modify_field(tlscb_0_d.ci_0, ci_0);                                                                  \
    modify_field(tlscb_0_d.pi_1, pi_1);                                                                  \
    modify_field(tlscb_0_d.ci_1, ci_1);                                                                  \
    modify_field(tlscb_0_d.fid, fid);                                                                    \
    modify_field(tlscb_0_d.serq_base, serq_base);                                                        \
    modify_field(tlscb_0_d.sw_serq_ci, sw_serq_ci);                                                      \
    modify_field(tlscb_0_d.serq_prod_ci_addr, serq_prod_ci_addr);                                        \
    modify_field(tlscb_0_d.sesq_base, sesq_base);                                                        \
    modify_field(tlscb_0_d.sw_sesq_pi, sw_sesq_pi);                                                      \
    modify_field(tlscb_0_d.sw_sesq_ci, sw_sesq_ci);                                                      \
    modify_field(tlscb_0_d.sw_bsq_ci, sw_bsq_ci);                                                        \
    modify_field(tlscb_0_d.dec_flow, dec_flow);                                                          \
    modify_field(tlscb_0_d.debug_dol, debug_dol);                                                        \
    modify_field(tlscb_0_d.barco_command, barco_command);                                                \
    modify_field(tlscb_0_d.barco_key_desc_index, barco_key_desc_index);                                  \
    modify_field(tlscb_0_d.salt, salt);                                                                  \
    modify_field(tlscb_0_d.explicit_iv, explicit_iv);

/* The defintion for access in stages other than 0 */
#define TLSCB_0_PARAMS_NON_STG0                                                                         \
    pc, TLSCB_0_PARAMS

#define GENERATE_TLSCB_0_D_NON_STG0                                                                     \
    modify_field(tlscb_0_d.pc, pc);                                                                     \
    GENERATE_TLSCB_0_D


action read_tls_stg0(TLSCB_0_PARAMS) {

    GENERATE_TLSCB_0_D
}

header_type tlscb_1_t {
    fields {
        qhead                           : ADDRESS_WIDTH;
        qtail                           : ADDRESS_WIDTH;
        una_desc                        : ADDRESS_WIDTH;
        una_desc_idx                    : 8;
        una_data_offset                 : 16;
        una_data_len                    : 16;
        nxt_desc                        : ADDRESS_WIDTH;
        nxt_desc_idx                    : 8;
        nxt_data_offset                 : 16;
        nxt_data_len                    : 16;
        next_tls_hdr_offset             : 16;
        cur_tls_data_len                : 16;
        other_fid                       : 16;
        // Total used   : 384 bits, pending: 128
        pad                             : 128;
    }
}

#define TLSCB_1_PARAMS                                                                                  \
qhead, qtail, una_desc, una_desc_idx, una_data_offset, una_data_len, nxt_desc, nxt_desc_idx, nxt_data_offset, nxt_data_len, next_tls_hdr_offset, cur_tls_data_len, other_fid
#

#define GENERATE_TLSCB_1_D                                                                              \
    modify_field(tlscb_1_d.qhead, qhead);                                                               \
    modify_field(tlscb_1_d.qtail, qtail);                                                               \
    modify_field(tlscb_1_d.una_desc, una_desc);                                                         \
    modify_field(tlscb_1_d.una_desc_idx, una_desc_idx);                                                 \
    modify_field(tlscb_1_d.una_data_offset, una_data_offset);                                           \
    modify_field(tlscb_1_d.una_data_len, una_data_len);                                                 \
    modify_field(tlscb_1_d.nxt_desc, nxt_desc);                                                         \
    modify_field(tlscb_1_d.nxt_desc_idx, nxt_desc_idx);                                                 \
    modify_field(tlscb_1_d.nxt_data_offset, nxt_data_offset);                                           \
    modify_field(tlscb_1_d.nxt_data_len, nxt_data_len);                                                 \
    modify_field(tlscb_1_d.next_tls_hdr_offset, next_tls_hdr_offset);                                   \
    modify_field(tlscb_1_d.cur_tls_data_len, cur_tls_data_len);                                         \
    modify_field(tlscb_1_d.other_fid, other_fid);


/* BARCO Descriptor definition */
header_type barco_desc_t {
    fields {
        input_list_address                  : 64;
        output_list_address                 : 64;
        command                             : 32;
        key_desc_index                      : 32;
        iv_address                          : 64;
        auth_tag_addr                       : 64;
        header_size                         : 32;
        status_address                      : 64;
        opaque_tag_value                    : 32;
        opaque_tag_write_en                 : 1;
        rsvd1                               : 31;
        sector_size                         : 16;
        application_tag                     : 16;
        sector_num                          : 32;
        doorbell_address                    : 64;
        doorbell_data                       : 64;
    }
}

 
/* BARCO DMA Channel PI/CI */
header_type barco_channel_pi_ci_t {
    fields {
        pi                                  : 32;
        ci                                  : 32;
    }
}

#define BARCO_CHANNEL_PARAMS        pi, ci

#define GENERATE_BARCO_CHANNEL_D                                                                    \
    modify_field(tls_enc_queue_brq_d.pi, pi);                                                           \
    modify_field(tls_enc_queue_brq_d.ci, ci);


/* BARCO cached Descriptor definition */
header_type barco_result_t {
    fields {
        status                              : 32;
        output_list_address                 : 64;
        pad                                 : 416;
    }
}

header_type tls_stage_pre_crypto_stats_d_t {
    fields {
        tnmdr_alloc                     : 16;
        tnmpr_alloc                     : 16;
        enc_requests                    : 16;
        dec_requests                    : 16;
        debug_stage0_3_thread           : 16;
        debug_stage4_7_thread           : 16;
        // TBD: Total used   : 96 bits, pending: 416
        pad                             : 416;
    }
}
#define STG_PRE_CRYPTO_STATS_ACTION_PARAMS                                                          \
tnmdr_alloc,tnmpr_alloc, enc_requests, dec_requests, debug_stage0_3_thread, debug_stage4_7_thread, pad
#

#define GENERATE_STG_PRE_CRYPTO_STATS_D                                                             \
    modify_field(tls_pre_crypto_stats_d.tnmdr_alloc, tnmdr_alloc);                                  \
    modify_field(tls_pre_crypto_stats_d.tnmpr_alloc, tnmpr_alloc);                                  \
    modify_field(tls_pre_crypto_stats_d.enc_requests, enc_requests);                                \
    modify_field(tls_pre_crypto_stats_d.dec_requests, dec_requests);                                \
    modify_field(tls_pre_crypto_stats_d.debug_stage0_3_thread, debug_stage0_3_thread);              \
    modify_field(tls_pre_crypto_stats_d.debug_stage4_7_thread, debug_stage4_7_thread);              \
    modify_field(tls_pre_crypto_stats_d.pad, pad);


header_type tls_stage_post_crypto_stats_d_t {
    fields {
        rnmdr_free                      : 16;
        rnmpr_free                      : 16;
        enc_completions                 : 16;
        dec_completions                 : 16;
        debug_stage0_3_thread           : 16;
        debug_stage4_7_thread           : 16;
        // TBD: Total used   : 96 bits, pending: 416
        pad                             : 416;
    }
}

#define STG_POST_CRYPTO_STATS_ACTION_PARAMS                                                             \
rnmdr_free,rnmpr_free, enc_completions, dec_completions, debug_stage0_3_thread, debug_stage4_7_thread, pad
#

#define GENERATE_STG_POST_CRYPTO_STATS_D                                                                \
    modify_field(tls_post_crypto_stats_d.rnmdr_free, rnmdr_free);                                       \
    modify_field(tls_post_crypto_stats_d.rnmpr_free, rnmpr_free);                                       \
    modify_field(tls_post_crypto_stats_d.enc_completions, enc_completions);                             \
    modify_field(tls_post_crypto_stats_d.dec_completions, dec_completions);                             \
    modify_field(tls_post_crypto_stats_d.debug_stage0_3_thread, debug_stage0_3_thread);                 \
    modify_field(tls_post_crypto_stats_d.debug_stage4_7_thread, debug_stage4_7_thread);                 \
    modify_field(tls_post_crypto_stats_d.pad, pad);


header_type serq_entry_d_t {
    fields {
        idesc                           : 64;
        pad                             : 448;
    }
}
#define SERQ_ENTRY_ACTION_PARAMS idesc, pad
#define SERQ_ENTRY_SCRATCH serq_entry_d
#define GENERATE_SERQ_ENTRY_D                                                                           \
    modify_field(SERQ_ENTRY_SCRATCH.idesc, idesc);                                                      \
    modify_field(SERQ_ENTRY_SCRATCH.pad, pad);


header_type tnmdr_pidx_t {
    fields {
        tnmdr_pidx                      : 32;
        tnmdr_pidx_full                 : 1;
        pad                             : 479;
    }
}
#define TNMDR_PIDX_ACTION_PARAMS tnmdr_pidx, pad
#define TNMDR_PIDX_SCRATCH tnmdr_pidx_d
#define GENERATE_TNMDR_PIDX_D                                                                           \
    modify_field(TNMDR_PIDX_SCRATCH.tnmdr_pidx, tnmdr_pidx);                                            \
    modify_field(TNMDR_PIDX_SCRATCH.pad, pad);


header_type tnmpr_pidx_t {
    fields {
        tnmpr_pidx                      : 32;
        tnmpr_pidx_full                 : 1;
        pad                             : 479;
    }
}
#define TNMPR_PIDX_ACTION_PARAMS tnmpr_pidx, pad
#define TNMPR_PIDX_SCRATCH tnmpr_pidx_d
#define GENERATE_TNMPR_PIDX_D                                                                           \
    modify_field(TNMPR_PIDX_SCRATCH.tnmpr_pidx, tnmpr_pidx);                                            \
    modify_field(TNMPR_PIDX_SCRATCH.pad, pad);


#define PKT_DESCR_AOL_ACTION_PARAMS                                                                     \
A0, O0, L0, A1, O1, L1, A2, O2, L2, next_addr, next_pkt 
#define PKT_DESCR_AOL_SCRATCH  pkt_descr_aol_d 
#define GENERATE_PKT_DESCR_AOL_D                                                                        \
    modify_field(PKT_DESCR_AOL_SCRATCH.A0, A0);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.O0, O0);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.L0, L0);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.A1, A1);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.O1, O1);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.L1, L1);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.A2, A2);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.O2, O2);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.L2, L2);                                                         \
    modify_field(PKT_DESCR_AOL_SCRATCH.next_addr, next_addr);                                           \
    modify_field(PKT_DESCR_AOL_SCRATCH.next_pkt, next_pkt);
    

header_type tls_header_t {
    fields {
        tls_hdr_type                        : 8;
        tls_hdr_version_major               : 8;
        tls_hdr_version_minor               : 8;
        tls_hdr_len                         : 16;
        tls_iv                              : 64;   /* Not truly part of the TLS header */
    }
}

#define TLS_HDR_ACTION_PARAMS   tls_hdr_type, tls_hdr_version_major, tls_hdr_version_minor, tls_hdr_len, tls_iv
#define TLS_HDR_SCRATCH tls_header_d
#define GENERATE_TLS_HDR_D                                                                              \
    modify_field(TLS_HDR_SCRATCH.tls_hdr_type, tls_hdr_type);                                           \
    modify_field(TLS_HDR_SCRATCH.tls_hdr_version_major, tls_hdr_version_major);                         \
    modify_field(TLS_HDR_SCRATCH.tls_hdr_version_minor, tls_hdr_version_minor);                         \
    modify_field(TLS_HDR_SCRATCH.tls_hdr_len, tls_hdr_len);                                             \
    modify_field(TLS_HDR_SCRATCH.tls_iv, tls_iv);


header_type read_tnmdr_d_t {
    fields {
        tnmdr_pidx              : 32;
        tnmdr_pidx_full         : 1;
    }
}

header_type read_tnmpr_d_t {
    fields {
        tnmpr_pidx              : 32;
        tnmpr_pidx_full         : 1;
    }
}



/* AAD Definition for AES-GCM */
header_type additional_data_t {
    fields {
        aad_seq_num                 : 64;
        aad_type                    : 8;
        aad_version_major           : 8;
        aad_version_minor           : 8;
        aad_length                  : 16;
    }
}

#define AAD_ACTION_PARAMS aad_seq_num, aad_type, aad_version_major, aad_version_minor, aad_length

#define GENERATE_AAD_FIELDS(TO, FROM)                                                               \
    modify_field(TO.aad_seq_num, FROM.aad_seq_num);                                                 \
    modify_field(TO.aad_type, FROM.aad_type);                                                       \
    modify_field(TO.aad_version_major, FROM.aad_version_major);                                     \
    modify_field(TO.aad_version_minor, FROM.aad_version_minor);                                     \
    modify_field(TO.aad_length, FROM.aad_length);

#define GENERATE_AAD_FIELDS_D(TO)                                                               \
    modify_field(TO.aad_seq_num, aad_seq_num);                                                 \
    modify_field(TO.aad_type, aad_type);                                                       \
    modify_field(TO.aad_version_major, aad_version_major);                                     \
    modify_field(TO.aad_version_minor, aad_version_minor);                                     \
    modify_field(TO.aad_length, aad_length);
