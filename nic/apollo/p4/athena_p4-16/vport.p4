control RxVport(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {
    @name(".nop") action nop() {
    }

    @name(".drop_packet") action drop_packet() {
        intr_global.drop = 1;
    }

    @name(".rx_vport_action") action rx_vport_action() {
        intr_p4.setValid();
        intr_p4.packet_len = intr_p4.frame_size - CAPRI_GLOBAL_INTRINSIC_HDR_SZ;
	hdr.capri_txdma_intrinsic.setInvalid();
	hdr.p4plus_to_p4.setInvalid();
        intr_global.tm_iq      = intr_global.tm_oq;
        intr_global.tm_oport   = TM_PORT_EGRESS;
        
        if (intr_p4.parser_err == TRUE) {
            if (intr_p4.len_err != 0) {
                    drop_packet();
            }
        }
    }

    @name(".rx_vport") table rx_vport {
        actions = {
            rx_vport_action;
        }
        default_action = rx_vport_action;
        stage = 0;
    }

    apply {
        rx_vport.apply();
    }

}

control TxVport(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

    @name(".tx_vport") action tx_vport_action() {
        intr_global.tm_iq = intr_global.tm_oq;
        intr_global.tm_oport = TM_PORT_DMA;
	metadata.cntrl.flow_miss = hdr.i2e.flow_miss;
    }

    @name(".tx_vport") table tx_vport {
        actions = {
            tx_vport_action;
        }
        default_action = tx_vport_action;
        stage = 4;
    }

    apply {
        tx_vport.apply();
    }

}
