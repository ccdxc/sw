/*****************************************************************************/
/* ROCEv2 processing                                                         */
/*****************************************************************************/
action decode_roce_opcode(raw_flags, len, qtype) {
    if ((capri_intrinsic.tm_oport == TM_PORT_DMA) and
        (control_metadata.rdma_enabled == TRUE)) {
        modify_field(control_metadata.p4plus_app_id, P4PLUS_APPTYPE_RDMA);
        add_header(p4_to_p4plus_roce);
        modify_field(p4_to_p4plus_roce.raw_flags, raw_flags);
        modify_field(p4_to_p4plus_roce.rdma_hdr_len, len);
        modify_field(p4_to_p4plus_roce.payload_len, (udp.len - 8 - len));

        add_header(capri_rxdma_intrinsic);
        add_header(capri_rxdma_p4_intrinsic);
        modify_field(capri_rxdma_intrinsic.qid, roce_bth.destQP);
        modify_field(capri_rxdma_intrinsic.qtype, qtype);
        add(capri_rxdma_intrinsic.rx_splitter_offset, len,
            (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
             P4PLUS_ROCE_HDR_SZ));
        if ((roce_bth.opCode & 0xE0) == 0x60) {
            add_header(p4_to_p4plus_roce_ip);
            add_header(p4_to_p4plus_roce_eth);
        }
    }
}

@pragma stage 4
table decode_roce_opcode {
    reads {
        roce_bth.opCode : exact;
    }
    actions {
        nop;
        decode_roce_opcode;
    }
    default_action : nop;
    size : DECODE_ROCE_OPCODE_TABLE_SIZE;
}

control process_roce {
    if (valid(roce_bth)) {
        apply(decode_roce_opcode);
    }
}
