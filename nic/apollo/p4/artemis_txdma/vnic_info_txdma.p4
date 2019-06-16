// VNIC Info Table in TXDMA direction
action vnic_info_txdma(lpm_base1, lpm_base2) {
    // Copy the LPM roots to PHV based on AF
    if (rx_to_tx_hdr.iptype == IPTYPE_IPV4) {
        modify_field(txdma_control.lpm1_base_addr, lpm_base1);
    } else {
        modify_field(txdma_control.lpm1_base_addr, lpm_base2);
    }
}

@pragma stage 0
@pragma index_table
table vnic_info_txdma {
    reads {
        rx_to_tx_hdr.vpc_id   : exact;
    }
    actions {
        vnic_info_txdma;
    }
    size : VNIC_INFO_TABLE_SIZE;
}

control vnic_info_txdma {
    apply(vnic_info_txdma);
}
