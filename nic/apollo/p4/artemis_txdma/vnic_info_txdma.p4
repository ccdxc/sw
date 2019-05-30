

// VNIC Info Table in TXDMA direction: Get base address for vpc peer routes
action vnic_info_txdma(entry_valid, lpm_base1, lpm_base2) {
    // Fill the keys only in second pass
    if (capri_p4_intr.recirc_count == 1) {
        if (entry_valid == TRUE) {
            //TODO-KSM: Fill the base with either v4 or v6 base based on packet type
            modify_field(scratch_metadata.vpc_peer_base, lpm_base1);
            modify_field(scratch_metadata.vpc_peer_base, lpm_base2);
        }
    }
    modify_field(scratch_metadata.flag, entry_valid);
}


@pragma stage 0
@pragma hbm_table
table vnic_info_txdma {
    reads {
        txdma_control.vpc_id    : exact;
    }
    actions {
        vnic_info_txdma;
    }
    size : RXDMA_VNIC_INFO_TXDMA_TABLE_SIZE;
}

control vnic_info_txdma {
    apply(vnic_info_txdma);
}
