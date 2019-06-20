/*****************************************************************************/
/* Remote 4to6 mapping                                                       */
/*****************************************************************************/
action remote_46_info(ipv6_tx_da, nh_id) {
    if (txdma_control.st_enable == TRUE) {
        modify_field(session_info_hint.nexthop_idx, nh_id);
        modify_field(session_info_hint.tx_dst_ip, ipv6_tx_da);
        modify_field(txdma_control.st_enable, FALSE);
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
