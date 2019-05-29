

// VNIC Info Table Action: Get all LPM roots for a VNET - different roots in Tx & Rx direction
// Pass the roots to later stages thru PHV
action vnic_info(entry_valid, lpm_base1, lpm_base2, lpm_base3,
                 lpm_base4, lpm_base5, lpm_base6, lpm_base7,
                 lpm_base8) {
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
    }
}


// VNIC Info table is 256 deep (accomodates 64 VMs with 4 VNETs/VM)
// We will need separate table for Rx and Tx, but this is defined
// as single table with direction bit, so a total of 1+8 = 9 bits
// used as key to index to 512 entries. 
@pragma stage 0
@pragma hbm_table
table vnic_info {
    reads {
        // TMP: Right now these 2 fields are treated as Union by P4C, Mahesh
        // will fix this to generate struct for these 2 fields instead of Union
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
    apply(vnic_info);
}
