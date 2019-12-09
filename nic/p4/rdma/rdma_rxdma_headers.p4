

//256
header_type p4_to_p4plus_roce_bth_header_t {
    fields {
        //these fields should match with that of 
        //intrinsic.p4's p4_to_p4plus_roce_header_t
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        //Parsed UDP options valid flags
        roce_opt_ts_valid   : 1;
        roce_opt_mss_valid  : 1;
        //currently, this field rdma_hdr_len is not used in p4+ code
        //however, 6 bits is the minimum needed and can not go down further
        rdma_hdr_len        : 6;
        raw_flags           : 16;
        ecn                 : 2;
        payload_len         : 14;
        //Parsed UDP options
        roce_opt_ts_value   : 32;
        roce_opt_ts_echo    : 32;
        roce_opt_mss        : 16;
        roce_opt_pad        : 16;
        roce_int_recirc_hdr : 16;

        // BTH header
        bth_opcode          : 8;
        bth_se              : 1;
        bth_m               : 1;
        bth_pad             : 2;
        bth_tver            : 4;
        bth_pkey            : 16;
        bth_f               : 1;
        bth_b               : 1;
        bth_rsvd1           : 6;
        bth_dst_qp          : 24;
        bth_a               : 1;
        bth_rsvd2           : 7;
        bth_psn             : 24;
    }
}

//416
header_type roce_recirc_header_t {
    fields {
        app_data0_1         : 144;
        token_id            : 8;
        recirc_reason       : 4;
        recirc_iter_count   : 4;
        app_data0_2         : 96;
        app_data1           : 88;
        app_data2           : 72;
    }
}

//288
header_type p4_to_p4plus_roce_bth_immeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        immeth_data         : 32;
    }
}

//384
header_type p4_to_p4plus_roce_bth_reth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        reth_va_1           : 32;
        reth_va_2           : 32;
        reth_r_key          : 32;
        reth_dma_len1       : 16;
        reth_dma_len2       : 16;
    }
}

//416
header_type p4_to_p4plus_roce_bth_reth_immeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        reth_header_bits_1  : 32;
        reth_header_bits_2  : 96;
        immeth_data         : 32;
    }
}

//288
header_type p4_to_p4plus_roce_bth_aeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        aeth_syndrome       : 8;
        aeth_msn            : 24;
    }
}

//352
header_type p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        aeth_header_bits    : 32;
        atomicaeth_orig_data: 64; 
    }
}

//416 + 64(ext)
header_type p4_to_p4plus_roce_bth_atomiceth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        atomiceth_va_1      : 32;
        atomiceth_va_2      : 32;
        atomiceth_r_key     : 32;
        atomiceth_swap_or_add_data : 64;
        //atomiceth_cmp_data : 64;
    }
}

//416 + 64(ext)
header_type p4_to_p4plus_roce_bth_atomiceth_ext_header_t {
    fields {
        atomiceth_cmp_data : 64;
    }
}

//288
header_type p4_to_p4plus_roce_bth_ieth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        ieth_r_key          : 32;
    }
}

//416 + 32(ext)
header_type p4_to_p4plus_roce_bth_deth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        deth_q_key          : 32;
        deth_rsvd           : 8;
        deth_src_qp         : 24;
        dmac                : 48;
        smac                : 48;
        //vlan_ethtype        : 16;
        //vlan                : 16;
        //ethtype             : 5;
    }
}

//416 + 32(ext)
header_type p4_to_p4plus_roce_bth_deth_ext_header_t {
    fields {
        vlan_ethtype        : 16;
        vlan                : 16;
        ethtype             : 5;
    }
}

//416 + 64(ext)
header_type p4_to_p4plus_roce_bth_deth_immeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        deth_header_bits_1  : 32;
        deth_header_bits_2  : 32;
        immeth_data         : 32;
        dmac                : 48;
        smac_1              : 16;
        //smac_2              : 32;
        //vlan_ethtype        : 16;
        //vlan                : 16;
        //ethtype             : 5;
    }
}

//416 + 64(ext)
header_type p4_to_p4plus_roce_bth_deth_immeth_ext_header_t {
    fields {
        smac_2              : 32;
        vlan_ethtype        : 16;
        vlan                : 16;
        ethtype             : 5;
    }
}

//288
header_type p4_to_p4plus_roce_bth_xrceth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_rsvd         : 8;
        xrceth_xrcsrq       : 24;
    }
}

//320
header_type p4_to_p4plus_roce_bth_xrceth_immeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        immeth_data         : 32;
    }
}

//416
header_type p4_to_p4plus_roce_bth_xrceth_reth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        reth_va             : 64;
        reth_r_key          : 32;
        reth_dma_len        : 32;
    }
}

//416 + 32 (ext)
header_type p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        reth_header_bits    : 128;
        //immeth_data         : 32;
    }
}

//416 + 32 (ext)
header_type p4_to_p4plus_roce_bth_xrceth_reth_immeth_ext_header_t {
    fields {
        immeth_data        : 32;
    }
}

//416 + 96(ext)
header_type p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        atomiceth_va        : 64;
        atomiceth_r_key     : 32;
        atomiceth_swap_or_add_data_1 : 32;
        //atomiceth_swap_or_add_data_2 : 32;
        //atomiceth_cmp_data  : 64;
    }
}

//416 + 96(ext)
header_type p4_to_p4plus_roce_bth_xrceth_atomiceth_ext_header_t {
    fields {
        atomiceth_swap_or_add_data_2 : 32;
        atomiceth_cmp_data  : 64;
    }
}

//320
header_type p4_to_p4plus_roce_bth_xrceth_ieth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        ieth_r_key          : 32;
    }
}

//248 + 52
header_type rdma_completion_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        wrid                : 64;
        status              : 8;
        error               : 1;
        err_qp_instantly    : 1;
        lif_cqe_error_id_vld: 1;
        lif_error_id_vld    : 1;
        lif_error_id        : 4;
        rsvd                : 8;
        ssn                 : 24;
        tx_psn              : 24;
    }
}

//248
header_type rdma_cq_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        cindex              : 16;
        color               : 1;
        arm                 : 1;
        sarm                : 1;
    }
}

//248
header_type rdma_timer_expiry_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        rexmit_psn          : 24;
        ssn                 : 24;
        tx_psn              : 24;
    }
}

//248
header_type rdma_aq_completion_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        wqe_id              : 16;
        status              : 8;
        op                  : 8;
        error               : 1;
    }
}

//248
header_type rdma_sq_drain_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        tx_psn              : 24;
        ssn                 : 24;
    }
}

//248
header_type rdma_aq_completion_feedback_header_create_qp_t {
    fields {
        common_roce_bits    :  48;
        rq_dma_addr         :  64;
        rq_id               :  24;
        rq_cmb              :   1;
        rq_spec             :   1;
        qp_privileged       :   1;
        log_pmtu            :   5;
        access_flags        :   3;
        rsvd                :  13;
        aq_comp_common_bits :  56;
        rq_cq_id            :  24;
        rq_depth_log2       :   8;
        rq_stride_log2      :   8;
        rq_page_size_log2   :   8;
        rq_tbl_index        :  32;
        rq_map_count        :  32;
        pd                  :  32;
        rq_type_state       :   8;
        pid                 :  16;
    }
}

//248
header_type rdma_aq_completion_feedback_header_modify_qp_t {
    fields {
        common_roce_bits      :  48;
        rq_psn                :  24;
        rq_psn_valid          :   1;
        rq_id                 :  24;
        rsvd                  :  63;
        aq_comp_common_bits   :  56;
        ah_len                :   8;
        ah_addr               :  32;
        av_valid              :   1;            
        state                 :   3;
        state_valid           :   1;
        pmtu_log2             :   5;
        pmtu_valid            :   1;            
        rrq_base_addr         :  32;
        rrq_depth_log2        :   5;
        rrq_valid             :   1;
        err_retry_count       :   3;
        err_retry_count_valid :   1;
        tx_psn_valid          :   1;
        tx_psn                :  24;
        rnr_retry_count       :   3;
        rnr_retry_valid       :   1;
        q_key_rsq_base_addr   :  32;
        q_key_valid           :   1;
        rsq_depth_log2        :   5;
        rsq_valid             :   1;
        sqd_async_notify_en   :   1;
        access_flags_valid    :   1;
        access_flags          :   3;
        cur_state             :   3;
        cur_state_valid       :   1;
        congestion_mgmt_enable :  1;
        dcqcn_cfg_id          :   4;
        rsvd2                 :   1;
    }
}

//248
header_type rdma_aq_completion_feedback_header_query_destroy_qp_t {
    fields {
        common_roce_bits      :  48;
        rsvd                  :  112;
        aq_comp_common_bits   :  56;
        rq_id                 :  24;
        dma_addr              :  64;
        rsvd2                 :  88;
    }
}
