/*****************************************************************************/
/* Local 4to6 mapping in TxDMA                                                  */
/*****************************************************************************/
action local_46_info_txdma(prefix) {
     modify_field(txdma_control.tx_local_46_ip, prefix);
}

@pragma stage 1
@pragma hbm_table
@pragma index_table
table local_46_mapping_txdma {
    reads {
        rx_to_tx_hdr.ca6_xlate_idx   : exact;
    }
    actions {
        local_46_info_txdma;
    }
    size : LOCAL_46_MAPPING_TABLE_SIZE;
}

control local_46_mapping_txdma {
    apply(local_46_mapping_txdma);
}
