action mpls_label_to_vnic(vnic_type, vnic_id) {
    if (vnic_id == 0) {
        /* Skip DNAT and Flow lookup */
        modify_field(control_metadata.skip_dnat_lkp, TRUE);
        modify_field(control_metadata.skip_flow_lkp, TRUE);
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(ingress_recirc_header.dnat_done, TRUE);
        /* Treat it as a flow miss for now */
        modify_field(control_metadata.flow_miss, TRUE);
    }
    else {
        modify_field(control_metadata.vnic_type, vnic_type);
        modify_field(key_metadata.vnic_id, vnic_id);
    }
}

@pragma stage 0
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
table mpls_label_to_vnic {
    reads {
        control_metadata.mpls_label_b20_b4 : exact;
        control_metadata.mpls_label_b3_b0 : exact;
    }
    actions {
        mpls_label_to_vnic;
    }
    size : MPLS_LABEL_VNIC_MAP_TABLE_SIZE;
}

action vlan_to_vnic(vnic_type, vnic_id) {
    if (vnic_id == 0) {
        /* Skip DNAT and Flow lookup */
        modify_field(control_metadata.skip_dnat_lkp, TRUE);
        modify_field(control_metadata.skip_flow_lkp, TRUE);
        modify_field(ingress_recirc_header.flow_done, TRUE);
        modify_field(ingress_recirc_header.dnat_done, TRUE);
        /* Treat it as a flow miss for now */
        modify_field(control_metadata.flow_miss, TRUE);
    }
    else {
        modify_field(control_metadata.vnic_type, vnic_type);
        modify_field(key_metadata.vnic_id, vnic_id);
    }
}

@pragma stage 0
@pragma hbm_table
@pragma index_table
@pragma capi_bitfields_struct
table vlan_to_vnic {
    reads {
        ctag_1.vid      : exact;
    }
    actions {
        vlan_to_vnic;
    }
    size : VLAN_VNIC_MAP_TABLE_SIZE;
}

control vnic {
    if (control_metadata.direction == TX_FROM_HOST) {
        apply(vlan_to_vnic);
    }

    if (control_metadata.direction == RX_FROM_SWITCH) {
        apply(mpls_label_to_vnic);
    }
}
