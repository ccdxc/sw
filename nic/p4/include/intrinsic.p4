header_type cap_phv_intr_global_t {
    fields {
        tm_iport            : 4;
        tm_oport            : 4;
        tm_iq               : 5;
        lif                 : 11;
        timestamp           : 48;
        tm_span_session     : 8;
        tm_replicate_ptr    : 16;
        tm_replicate_en     : 1;
        tm_cpu              : 1;
        tm_q_depth          : 14;
        drop                : 1;
        bypass              : 1;
        hw_error            : 1;
        tm_oq               : 5;
        debug_trace         : 1;
        csum_err            : 5;
        error_bits          : 6;
        tm_instance_type    : 4;
    }
}

header_type cap_phv_intr_p4_t {
    fields {
        crc_err             : 1;
        len_err             : 4;
        recirc_count        : 3;
        parser_err          : 1;
        p4_pad              : 1;
        frame_size          : 14;
        no_data             : 1;
        recirc              : 1;
        packet_len          : 14;
    }
}

header_type cap_phv_intr_txdma_t {
    fields {
        qid                 : 24;
        dma_cmd_ptr         : 6;
        qstate_addr         : 34;
        qtype               : 3;
        txdma_rsv           : 5;
    }
}

header_type cap_phv_intr_rxdma_t {
    fields {
        qid                 : 24;
        dma_cmd_ptr         : 6;
        qstate_addr         : 34;
        qtype               : 3;
        rx_splitter_offset  : 10;
        rxdma_rsv           : 3;
    }
}
