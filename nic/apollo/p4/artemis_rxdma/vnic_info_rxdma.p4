// VNIC Info Table Action: Get all LPM roots for a VNET - different roots in Tx & Rx direction
// Pass the roots to later stages thru PHV
action vnic_info_rxdma(lpm_base1, lpm_base2, lpm_base3, lpm_base4,
                 lpm_base5, lpm_base6, lpm_base7, lpm_base8) {

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

    // Copy the vnic id and vcn id so it goes to txdma
    modify_field(rx_to_tx_hdr.vnic_id, p4_to_rxdma.vnic_id);
    modify_field(rx_to_tx_hdr.vcn_id, p4_to_rxdma.vcn_id);

    // Always fill the remote_ip from p4 keys based on the direction
    if (p4_to_rxdma.direction == TX_FROM_HOST) {
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_dst);
    } else {
        modify_field(rx_to_tx_hdr.remote_ip, p4_to_rxdma.flow_src);
    }

    // Pick the correct xlate_idx out of 3 idx's passed by P4, only
    // needed for traffic from Switch to Host direction
    // If all 3 idx's are not valid, then xlate_idx is untouched as zero
    if (p4_to_rxdma.direction == RX_FROM_SWITCH) {
        if (p4_to_rxdma2.service_xlate_idx != 0) {
            modify_field(rxdma_control.xlate_idx, p4_to_rxdma2.service_xlate_idx);
        } else {
            if (p4_to_rxdma2.pa_or_ca_xlate_idx != 0) {
                modify_field(rxdma_control.xlate_idx, p4_to_rxdma2.pa_or_ca_xlate_idx);
            } else {
                if (p4_to_rxdma2.public_xlate_idx != 0) {
                    modify_field(rxdma_control.xlate_idx, p4_to_rxdma2.public_xlate_idx);
                }
            }
        }
    }

    // Setup key for DPORT lookup
    modify_field(lpm_metadata.lpm2_key,
        (p4_to_rxdma.flow_dport | (p4_to_rxdma.flow_proto << 16)));
    // Enable LPM2
    modify_field(p4_to_rxdma.lpm2_enable, TRUE);
}


// VNIC Info table is 256 deep (accomodates 64 VMs with 4 VNETs/VM)
// We will need separate table for Rx and Tx, but this is defined
// as single table with direction bit, so a total of 1+8 = 9 bits
// used as key to index to 512 entries.
@pragma stage 0
@pragma hbm_table
table vnic_info_rxdma {
    reads {
        p4_to_rxdma.direction : exact;
        p4_to_rxdma.vnic_id : exact;
    }
    actions {
        vnic_info_rxdma;
    }
    size : VNIC_INFO_RXDMA_TABLE_SIZE;
}

control vnic_info_rxdma {
    // TMP: Launch this look up only in first pass (recirc == 0)
    apply(vnic_info_rxdma);
}
