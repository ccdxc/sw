/*****************************************************************************/
/* Remote 4to6 mapping                                                       */
/*****************************************************************************/
action remote_46_info(ipv6_tx_da, nh_id) {
    modify_field(txdma_control.nexthop_group_index, nh_id);
    modify_field(txdma_control.ipv6_tx_da, ipv6_tx_da);
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
