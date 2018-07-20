/*****************************************************************************/
/* RVPATH check                                                              */
/*****************************************************************************/
action rvpath_info(tep_idx, vcn_id, subnet_id, overlay_mac) {
    modify_field(scratch_metadata.vcn_id, vcn_id);
    if (vnic_metadata.vcn_id != scratch_metadata.vcn_id) {
        drop_packet();
    } else {
        modify_field(rvpath_metadata.subnet_id, subnet_id);
        modify_field(rvpath_metadata.overlay_mac, overlay_mac);
        modify_field(rvpath_metadata.tep_idx, tep_idx);
    }
}

@pragma stage 1
@pragma hbm_table
table remote_vnic_mappings_rx {
    reads {
        rvpath_metadata.src_slot_id : exact;
    }
    actions {
        rvpath_info;
    }
    size : REMOTE_VNIC_MAPPINGS_RX_TABLE_SIZE;
}

control rvpath_check {
    if (mpls[1].valid == TRUE) {
        apply(remote_vnic_mappings_rx);
    }
}
