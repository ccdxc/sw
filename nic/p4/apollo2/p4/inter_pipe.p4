/*****************************************************************************/
/* Ingress pipeline to RxDMA pipeline                                        */
/*****************************************************************************/
action ingress_to_rxdma() {
    if ((service_header.local_ip_mapping_done == FALSE) or
        (service_header.flow_done == FALSE)) {
        add_header(service_header);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
    } else {
        modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
        modify_field(capri_intrinsic.lif, APOLLO_SERVICE_LIF);
        add_header(capri_p4_intrinsic);
        add_header(capri_rxdma_intrinsic);
        add_header(p4_to_arm_header);
        add_header(p4_to_rxdma_header);
        // Splitter offset should point to here
        modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                     (CAPRI_GLOBAL_INTRINSIC_HDR_SZ +
                      CAPRI_RXDMA_INTRINSIC_HDR_SZ +
                      APOLLO_P4_TO_ARM_HDR_SZ +
                      APOLLO_P4_TO_RXDMA_HDR_SZ));
        add_header(predicate_header);
        add_header(p4_to_txdma_header);
        add_header(apollo_i2e_metadata);
        remove_header(service_header);

        modify_field(p4_to_txdma_header.payload_len,
                     capri_p4_intrinsic.packet_len + APOLLO_I2E_HDR_SZ);
        modify_field(predicate_header.direction, control_metadata.direction);
        if (control_metadata.direction == RX_FROM_SWITCH) {
            modify_field(predicate_header.lpm_bypass, TRUE);
        }
    }
}

@pragma stage 5
table ingress_to_rxdma {
    actions {
        ingress_to_rxdma;
    }
}

control ingress_to_rxdma {
    apply(ingress_to_rxdma);
}

/*****************************************************************************/
/* Egress pipeline to uplink                                                 */
/*****************************************************************************/
action egress_to_uplink() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_UPLINK_1);
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    remove_header(capri_txdma_intrinsic);
    remove_header(predicate_header);
    remove_header(txdma_to_p4e_header);
    remove_header(apollo_i2e_metadata);
}

@pragma stage 5
table egress_to_uplink {
    actions {
        egress_to_uplink;
    }
}

control egress_to_uplink {
    apply(egress_to_uplink);
}
