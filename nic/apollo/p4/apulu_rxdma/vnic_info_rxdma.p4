/******************************************************************************/
/* VNIC info : Get all lpm roots for a vnic. Different roots for tx and rx    */
/* packet. Pass the roots to later stats through PHV                          */
/******************************************************************************/
action vnic_info_rxdma(lpm_base1, lpm_base2, lpm_base3, lpm_base4,
                       lpm_base5, lpm_base6, lpm_base7, lpm_base8) {

    // Disable this lookup for further passes
    modify_field(p4_to_rxdma.vnic_info_en, FALSE);

    // Unused for now.
    modify_field(scratch_metadata.field40, lpm_base8);

    // Fill the remote_ip based on the direction
    if (p4_to_rxdma.rx_packet == 0) {
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_dst);
    } else {
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_src);
    }

    // If route root != NULL
    if (lpm_base1 != 0) {
        // Copy route root to PHV
        modify_field(rx_to_tx_hdr.route_base_addr, lpm_base1);
    }

    if (lpm_base2 != 0) {
        // Copy the sacl roots to lpm metadata
        modify_field(lpm_metadata.sacl_base_addr,  lpm_base2);
        modify_field(rx_to_tx_hdr.sacl_base_addr0, lpm_base2);
        modify_field(rx_to_tx_hdr.sacl_base_addr1, lpm_base3);
        modify_field(rx_to_tx_hdr.sacl_base_addr2, lpm_base4);
        modify_field(rx_to_tx_hdr.sacl_base_addr3, lpm_base5);
        modify_field(rx_to_tx_hdr.sacl_base_addr4, lpm_base6);
        modify_field(rx_to_tx_hdr.sacl_base_addr5, lpm_base7);

        // Setup and Enable LPM1 for SPORT lookup
        modify_field(lpm_metadata.lpm1_base_addr, lpm_base2 +
                     SACL_SPORT_TABLE_OFFSET);
        modify_field(lpm_metadata.lpm1_key, p4_to_rxdma.flow_sport);
        modify_field(p4_to_rxdma.lpm1_enable, TRUE);

        // Setup and Enable LPM2 for DPORT lookup
        modify_field(lpm_metadata.lpm2_base_addr, lpm_base2 +
                     SACL_PROTO_DPORT_TABLE_OFFSET);
        modify_field(lpm_metadata.lpm2_key, (p4_to_rxdma.flow_dport |
                                            (p4_to_rxdma.flow_proto << 24)));
        modify_field(p4_to_rxdma.lpm2_enable, TRUE);
    }
}


// VNIC Info table is 2048 deep, with bit 12 indicating rx or tx pakcet
@pragma stage 0
@pragma hbm_table
@pragma index_table
table vnic_info_rxdma {
    reads {
        p4_to_rxdma.vnic_info_key : exact;
    }
    actions {
        vnic_info_rxdma;
    }
    size : VNIC_INFO_RXDMA_TABLE_SIZE;
}

control vnic_info_rxdma {
    apply(vnic_info_rxdma);
}
