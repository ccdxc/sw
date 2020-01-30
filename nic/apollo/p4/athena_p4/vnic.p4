action mpls_label_to_vnic(vnic_type, vnic_id) {
    if (vnic_id == 0) {
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
table mpls_label_to_vnic {
    reads {
        mpls_dst.label : exact;
    }
    actions {
        mpls_label_to_vnic;
    }
    size : MPLS_LABEL_VNIC_MAP_TABLE_SIZE;
}

action vlan_to_vnic(vnic_type, vnic_id) {
    if (vnic_id == 0) {
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
