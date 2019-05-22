/******************************************************************************/
/* Local IP mappings                                                          */
/******************************************************************************/
action local_ip_mapping_info(vnic_id, vpc_id, service_tag, xlate_idx1,
                             xlate_idx2) {
    // if table lookup is a miss, return

    if (vnic_id != 0) {
        modify_field(vnic_metadata.vnic_id, vnic_id);
        modify_field(vnic_metadata.vpc_id, vpc_id);
    }
    modify_field(p4_to_rxdma.service_tag, service_tag);
    modify_field(p4i_i2e.xlate_idx1, xlate_idx1);
    modify_field(p4i_i2e.xlate_idx2, xlate_idx2);
}

@pragma stage 2
table local_ip_mapping {
    reads {
        vnic_metadata.vpc_id    : exact;
        key_metadata.mapping_ip : exact;
    }
    actions {
        local_ip_mapping_info;
    }
    size : LOCAL_IP_MAPPING_HASH_TABLE_SIZE;
}

@pragma stage 2
@pragma overflow_table local_ip_mapping
table local_ip_mapping_otcam {
    reads {
        vnic_metadata.vpc_id    : ternary;
        key_metadata.mapping_ip : ternary;
    }
    actions {
        local_ip_mapping_info;
    }
    size : LOCAL_IP_MAPPING_OTCAM_TABLE_SIZE;
}

/******************************************************************************/
/* Service mappings                                                           */
/******************************************************************************/
action service_mapping_info(xlate_idx3, xlate_port) {
    // if table lookup is a miss, return

    modify_field(p4i_i2e.xlate_idx3, xlate_idx3);
    modify_field(p4i_i2e.xlate_port, xlate_port);
}

@pragma stage 2
table service_mapping {
    reads {
        vnic_metadata.vpc_id        : exact;
        key_metadata.mapping_ip     : exact;
        key_metadata.mapping_ip2    : exact;
        key_metadata.mapping_port   : exact;
    }
    actions {
        service_mapping_info;
    }
    size : SERVICE_MAPPING_HASH_TABLE_SIZE;
}

@pragma stage 2
@pragma overflow_table service_mapping
table service_mapping_otcam {
    reads {
        vnic_metadata.vpc_id        : ternary;
        key_metadata.mapping_ip     : ternary;
        key_metadata.mapping_ip2    : ternary;
        key_metadata.mapping_port   : ternary;
    }
    actions {
        service_mapping_info;
    }
    size : SERVICE_MAPPING_OTCAM_TABLE_SIZE;
}

control ingress_mappings {
    apply(local_ip_mapping);
    apply(local_ip_mapping_otcam);
    apply(service_mapping);
    apply(service_mapping_otcam);
}
