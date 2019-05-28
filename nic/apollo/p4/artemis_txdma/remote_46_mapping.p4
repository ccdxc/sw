/*****************************************************************************/
/* Remote 4to6 mapping                                                       */
/*****************************************************************************/
action remote_46_info(entry_valid, ipv6_tx_da) {
    if (entry_valid == TRUE) {
        modify_field(scratch_metadata.flag, entry_valid);
        modify_field(scratch_metadata.ipv6_tx_da, ipv6_tx_da);
    }
}

@pragma stage 0
@pragma_index_table
table remote_46_mapping {
    reads {
        txdma_control.svc_id   : exact;
    }
    actions {
        remote_46_info;
    }
    size : REMOTE_46_MAPPING_TABLE_SIZE;
}

control remote_46_mapping {
    apply(remote_46_mapping);
}
