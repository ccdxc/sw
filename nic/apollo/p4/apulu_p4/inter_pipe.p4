/*****************************************************************************/
/* Inter pipe : ingress pipeline                                             */
/*****************************************************************************/
action tunnel_decap() {
    remove_header(ethernet_1);
    remove_header(ctag_1);
    remove_header(ipv4_1);
    remove_header(ipv6_1);
    remove_header(udp_1);
    remove_header(vxlan_1);
    subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.frame_size,
             offset_metadata.l2_2);
}

action ingress_to_egress() {
    add_header(capri_p4_intrinsic);
    add_header(txdma_to_p4e);
    add_header(p4i_i2e);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    remove_header(ingress_recirc);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
}

action ingress_to_rxdma() {
    add_header(capri_p4_intrinsic);
    add_header(capri_rxdma_intrinsic);
    add_header(p4i_to_rxdma);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    remove_header(ingress_recirc);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, APULU_SERVICE_LIF);
    modify_field(capri_rxdma_intrinsic.rx_splitter_offset,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ +
                  CAPRI_RXDMA_INTRINSIC_HDR_SZ + APULU_P4I_TO_RXDMA_HDR_SZ));
}

action ingress_recirc() {
    add_header(capri_p4_intrinsic);
    add_header(ingress_recirc);
    remove_header(capri_txdma_intrinsic);
    remove_header(p4plus_to_p4);
    remove_header(p4plus_to_p4_vlan);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_INGRESS);
}

action p4i_inter_pipe() {
    if ((ingress_recirc.flow_done == FALSE) or
        (ingress_recirc.local_mapping_done == FALSE)) {
        ingress_recirc();
        // return
    }

    if (control_metadata.tunneled_packet == TRUE) {
        tunnel_decap();
    }

    if (control_metadata.flow_miss == TRUE) {
        ingress_to_rxdma();
    } else {
        ingress_to_egress();
    }
}

@pragma stage 5
table p4i_inter_pipe {
    actions {
        p4i_inter_pipe;
    }
}

control ingress_inter_pipe {
    if (capri_intrinsic.drop == 0) {
        apply(p4i_inter_pipe);
    }
}
