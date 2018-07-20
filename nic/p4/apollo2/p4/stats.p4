/*****************************************************************************/
/* Ingress VNIC stats                                                        */
/*****************************************************************************/
action ingress_vnic_stats(in_packets, in_bytes) {
    modify_field(scratch_metadata.in_packets, in_packets);
    modify_field(scratch_metadata.in_bytes, in_bytes);
    modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
    add_header(capri_p4_intrinsic);

    if ((service_header.local_ip_mapping_done == FALSE) or
        (service_header.flow_done == FALSE)) {
        add_header(service_header);
        modify_field(service_header.nexthop_index,
                     rewrite_metadata.nexthop_index);
    }
}

@pragma stage 5
table ingress_vnic_stats {
    reads {
        control_metadata.ingress_vnic   : exact;
    }
    actions {
        ingress_vnic_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

control ingress_stats {
    apply(ingress_vnic_stats);
}

/*****************************************************************************/
/* Egress VNIC stats                                                         */
/*****************************************************************************/
action egress_vnic_stats(out_packets, out_bytes) {
    modify_field(scratch_metadata.in_packets, out_packets);
    modify_field(scratch_metadata.in_bytes, out_bytes);
}

@pragma stage 5
table egress_vnic_stats {
    reads {
        control_metadata.egress_vnic    : exact;
    }
    actions {
        egress_vnic_stats;
    }
    size : VNIC_STATS_TABLE_SIZE;
}

control egress_stats {
    apply(egress_vnic_stats);
}
