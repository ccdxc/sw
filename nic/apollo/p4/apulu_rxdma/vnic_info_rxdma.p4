/******************************************************************************/
/* VNIC info : Get all lpm roots for a vnic. Different roots for tx and rx    */
/* packet. Pass the roots to later stats through PHV                          */
/******************************************************************************/
action vnic_info_rxdma(lpm_base1, lpm_base2, lpm_base3, lpm_base4,
                       lpm_base5, lpm_base6, lpm_base7, lpm_base8) {

    // Pass payload_len from rxdma to txdma
    modify_field(rx_to_tx_hdr.payload_len, capri_p4_intr.packet_len);

    // Disable lookup for further passes
    modify_field(p4_to_rxdma.vnic_info_en, FALSE);

    // Copy the LPM roots to PHV based on AF
    if (p4_to_rxdma.iptype == IPTYPE_IPV4) {
        modify_field(rx_to_tx_hdr.sacl_base_addr, lpm_base1);
        modify_field(lpm_metadata.meter_base_addr, lpm_base3);
        modify_field(rx_to_tx_hdr.route_base_addr, lpm_base5);
        // Setup root address of DPORT lookup
        modify_field(lpm_metadata.lpm2_base_addr, lpm_base1 +
                     SACL_PROTO_DPORT_TABLE_OFFSET);
    } else {
        modify_field(rx_to_tx_hdr.sacl_base_addr, lpm_base2);
        modify_field(lpm_metadata.meter_base_addr, lpm_base4);
        modify_field(rx_to_tx_hdr.route_base_addr, lpm_base6);
        // Setup root address of DPORT lookup
        modify_field(lpm_metadata.lpm2_base_addr, lpm_base2 +
                     SACL_PROTO_DPORT_TABLE_OFFSET);
    }

    modify_field(scratch_metadata.field40, lpm_base7);
    modify_field(scratch_metadata.field40, lpm_base8);

    // Copy the data that need to go to txdma
    modify_field(rx_to_tx_hdr.vpc_id, p4_to_rxdma.vpc_id);
    modify_field(rx_to_tx_hdr.vnic_id, p4_to_rxdma.vnic_id);
    modify_field(rx_to_tx_hdr.rx_packet, p4_to_rxdma.rx_packet);
    // Pass iptype (address family) to txdma
    modify_field(rx_to_tx_hdr.iptype, p4_to_rxdma.iptype);

    // Fill the remote_ip and tag classid based on the txdma_to_p4e
    if (p4_to_rxdma.rx_packet == 0) {
        modify_field(rx_to_tx_hdr.stag_classid, p4_to_rxdma.service_tag);
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_dst);
    } else {
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_src);
        modify_field(rx_to_tx_hdr.dtag_classid, p4_to_rxdma.service_tag);
    }

    // Setup key for DPORT lookup
    modify_field(lpm_metadata.lpm2_key,
                 (p4_to_rxdma.flow_dport | (p4_to_rxdma.flow_proto << 16)));
    // Enable LPM2
    modify_field(p4_to_rxdma.lpm2_enable, TRUE);
}


// VNIC Info table is 2048 deep, with bit 12 indicating rx or tx pakcet
@pragma stage 0
@pragma hbm_table
@pragma index_table
table vnic_info_rxdma {
    reads {
        p4_to_rxdma.vnic_id : exact;
    }
    actions {
        vnic_info_rxdma;
    }
    size : VNIC_INFO_RXDMA_TABLE_SIZE;
}

control vnic_info_rxdma {
    apply(vnic_info_rxdma);
}
