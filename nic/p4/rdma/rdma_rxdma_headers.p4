

header_type p4_to_p4plus_roce_bth_header_t {
    fields {
        //these fields should match with that of 
        //intrinsic.p4's p4_to_p4plus_roce_header_t
        p4plus_app_id       : 4;
        flags               : 4;
        rdma_hdr_len        : 8;
        raw_flags           : 16;
        payload_len         : 16;

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

header_type p4_to_p4plus_roce_bth_immeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        immeth_data         : 32;
    }
}

header_type p4_to_p4plus_roce_bth_reth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        reth_va             : 64;
        reth_r_key          : 32;
        reth_dma_len1       : 16;
        reth_dma_len2       : 16;
    }
}

header_type p4_to_p4plus_roce_bth_reth_immeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        reth_header_bits    : 128;
        immeth_data         : 32;
    }
}

header_type p4_to_p4plus_roce_bth_aeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        aeth_syndrome       : 8;
        aeth_msn            : 24;
    }
}

header_type p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        aeth_header_bits    : 32;
        atomicaeth_orig_data: 64; 
    }
}

header_type p4_to_p4plus_roce_bth_atomiceth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        atomiceth_va        : 64;
        atomiceth_r_key     : 32;
        atomiceth_swap_or_add_data : 64;
        atomiceth_cmp_data  : 64;
    }
}

header_type p4_to_p4plus_roce_bth_ieth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        ieth_r_key          : 32;
    }
}

header_type p4_to_p4plus_roce_bth_deth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        deth_q_key          : 32;
        deth_rsvd           : 8;
        deth_src_qp         : 24;
    }
}

header_type p4_to_p4plus_roce_bth_deth_immeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        deth_header_bits    : 64;
        immeth_data         : 32;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_rsvd         : 8;
        xrceth_xrcsrq       : 24;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_immeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        immeth_data         : 32;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_reth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        reth_va             : 64;
        reth_r_key          : 32;
        reth_dma_len        : 32;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        reth_header_bits    : 128;
        immeth_data         : 32;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        atomiceth_va        : 64;
        atomiceth_r_key     : 32;
        atomiceth_swap_or_add_data : 64;
        atomiceth_cmp_data  : 64;
    }
}

header_type p4_to_p4plus_roce_bth_xrceth_ieth_header_t {
    fields {
        common_header_bits  : 48;
        bth_header_bits     : 96;
        xrceth_header_bits  : 32;
        ieth_r_key          : 32;
    }
}
