
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
        una_desc_idx                    : 2;
        una_data_offset                 : 16;
        una_data_len                    : 16;
        nxt_desc                        : ADDRESS_WIDTH;
        nxt_desc_idx                    : 2;
        nxt_data_offset                 : 16;
        nxt_data_len                    : 16;
        next_tls_hdr_offset             : 16;
        cur_tls_data_len                : 16;

        // Total used   : 356 bits, pending: 156
        pad                             : 156;
    }
}

#define TLSCB_1_PARAMS                                                                                  \
cipher_type, ver_major, qhead, qtail, una_desc, una_desc_idx, una_data_offset, una_data_len, nxt_desc, nxt_desc_idx, nxt_data_offset, nxt_data_len, next_tls_hdr_offset, cur_tls_data_len, ofid
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
    modify_field(tlscb_1_d.cur_tls_data_len, cur_tls_data_len);


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
        opaque_tage_write_en                : 1;
        sector_size                         : 16;
        sector_num                          : 32;
        application_tag                     : 16;
        doorbell_address                    : 64;
        doorbell_data                       : 64;
        gcm_key_buf_index                   : 8;
        gcm_new_key_flag                    : 1;
        rsvd                                : 374;
    }
}

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

