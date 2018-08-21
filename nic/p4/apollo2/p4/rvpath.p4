/*****************************************************************************/
/* RVPATH check                                                              */
/*****************************************************************************/
action rvpath_info(tep_index, vcn_id, subnet_id, overlay_mac) {
    modify_field(scratch_metadata.vcn_id, vcn_id);
    if (vnic_metadata.vcn_id != scratch_metadata.vcn_id) {
        modify_field(control_metadata.p4i_drop_reason,
                     1 << DROP_RVPATH_VCN_MISMATCH);
        drop_packet();
    } else {
        modify_field(apollo_i2e_metadata.rvpath_subnet_id, subnet_id);
        modify_field(apollo_i2e_metadata.rvpath_overlay_mac, overlay_mac);
        modify_field(rvpath_metadata.tep_index, tep_index);
    }
}

@pragma stage 1
@pragma hbm_table
table remote_vnic_mappings_rx {
    reads {
        vnic_metadata.src_slot_id : exact;
    }
    actions {
        rvpath_info;
    }
    size : REMOTE_VNIC_MAPPINGS_RX_TABLE_SIZE;
}

action tep_rx(tep_ip) {
    if (ipv4_1.srcAddr != tep_ip) {
        modify_field(control_metadata.p4i_drop_reason,
                     1 << DROP_TEP_RX_IP_MISMATCH);
        drop_packet();
    }
    modify_field(scratch_metadata.ipv4_src, tep_ip);
}

@pragma stage 2
table tep_rx {
    reads {
        rvpath_metadata.tep_index : exact;
    }
    actions {
        tep_rx;
    }
    size : TEP_TABLE_SIZE;
}

control rvpath_check {
    if (mpls[1].valid == TRUE) {
        apply(remote_vnic_mappings_rx);
        apply(tep_rx);
    }
}
