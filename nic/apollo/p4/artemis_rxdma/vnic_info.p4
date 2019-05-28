

// VNIC Info Table Action is same for both Rx and Tx, just that diff tables looked up
action vnic_info(entry_valid, lpm_base1, lpm_base2, lpm_base3,
                 lpm_base4, lpm_base5, lpm_base6, lpm_base7,
                 lpm_base8, acl_base, meter_base) {
    if (entry_valid == TRUE) {
        modify_field(scratch_metadata.flag, entry_valid);
        modify_field(scratch_metadata.lpm_base1, lpm_base1);
        modify_field(scratch_metadata.lpm_base2, lpm_base2);
        modify_field(scratch_metadata.lpm_base3, lpm_base3);
        modify_field(scratch_metadata.lpm_base4, lpm_base4);
        modify_field(scratch_metadata.lpm_base5, lpm_base5);
        modify_field(scratch_metadata.lpm_base6, lpm_base6);
        modify_field(scratch_metadata.lpm_base7, lpm_base7);
        modify_field(scratch_metadata.lpm_base8, lpm_base8);
        modify_field(scratch_metadata.acl_base, acl_base);
        modify_field(scratch_metadata.meter_base, meter_base);
    }
}

/* miss action : drop packet */
@pragma stage 1
@pragma hbm_table
table vnic_info_tx {
    reads {
        //p4_to_rxdma.direction : exact;
        p4_to_rxdma.vnic_id : exact;
    }
    actions {
        vnic_info;
    }
    size : RXDMA_VNIC_INFO_TABLE_SIZE;
}

/* miss action : drop packet */
@pragma stage 1
@pragma hbm_table
table vnic_info_rx {
    reads {
        p4_to_rxdma.direction : exact;
        p4_to_rxdma.vnic_id : exact;
    }
    actions {
        vnic_info;
    }
    size : RXDMA_VNIC_INFO_TABLE_SIZE;
}

control vnic_info {
    // TMP: Launch this look up only in first pass (recirc == 0)
    //if (p4_to_rxdma.direction == TX_FROM_HOST) {
        apply(vnic_info_tx);
    //} else {  // RX_FROM_SWITCH
        //apply(vnic_info_rx);
    //}
}
