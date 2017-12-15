/*****************************************************************************/
/* ROCEv2 processing                                                         */
/*****************************************************************************/
action decode_roce_opcode(raw_flags, len, qtype, tm_oq_overwrite, tm_oq) {
    if ((capri_intrinsic.tm_oport == TM_PORT_DMA) and
        (control_metadata.rdma_enabled == TRUE)) {

        modify_field(control_metadata.p4plus_app_id, P4PLUS_APPTYPE_RDMA);
        add_header(p4_to_p4plus_roce);
        modify_field(p4_to_p4plus_roce.raw_flags, raw_flags);
        modify_field(p4_to_p4plus_roce.rdma_hdr_len, len);
        modify_field(p4_to_p4plus_roce.payload_len, (udp.len - 8 - len));

        if (tm_oq_overwrite == TRUE) {
            modify_field(capri_intrinsic.tm_oq, tm_oq);
        }

        add_header(capri_rxdma_intrinsic);
        if (capri_intrinsic.tm_instance_type == TM_INSTANCE_TYPE_MULTICAST) {
            modify_field(capri_rxdma_intrinsic.qid, control_metadata.qid);
        } else {
            modify_field(capri_rxdma_intrinsic.qid, roce_bth.destQP);
        }
        modify_field(capri_rxdma_intrinsic.qtype, qtype);
        add(capri_rxdma_intrinsic.rx_splitter_offset, len,
            (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_RXDMA_INTRINSIC_HDR_SZ +
             P4PLUS_ROCE_HDR_SZ));
        if ((roce_bth.opCode & 0xE0) == 0x60) {
            add_header(p4_to_p4plus_roce_ip);
            add_header(p4_to_p4plus_roce_eth);
            /* increment splitter offset to conditionally skip eth header for UD RDMA */
            add_to_field(capri_rxdma_intrinsic.rx_splitter_offset, 14);
            /* Add conditionally 40B for ip header length for UD RDMA
               For IPv6, store 40B of IPv6 header
               For IPv4, save 20B of hdr into first 20Bytes and rest 20Bytes are zero */
            add_to_field(p4_to_p4plus_roce.payload_len, 40);
        }
    }
    modify_field(scratch_metadata.tm_oq_overwrite, tm_oq_overwrite);
}

@pragma stage 2
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
