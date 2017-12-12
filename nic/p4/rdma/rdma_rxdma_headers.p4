

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
        rsvd                : 4;
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

//416
header_type p4_to_p4plus_roce_bth_deth_header_t {
    fields {
        common_header_bits  : 160;
        bth_header_bits     : 96;
        deth_q_key          : 32;
        deth_rsvd           : 8;
        deth_src_qp         : 24;
        dmac                : 48;
        smac                : 48;
    }
}

//416 + 16(ext)
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
    }
}

//416 + 16(ext)
header_type p4_to_p4plus_roce_bth_deth_immeth_ext_header_t {
    fields {
        smac_2             : 32;
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

//248
header_type rdma_ud_feedback_header_t {
    fields {
        common_header_bits  : 160;
        feedback_type       : 8;
        wrid                : 64;
        optype              : 8;
        status              : 8;
    }
}

