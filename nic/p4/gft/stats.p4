action ingress_vport_stats(permit_packets, permit_bytes,
                           drop_packets, drop_bytes) {
    if (capri_intrinsic.drop == TRUE) {
        modify_field(scratch_metadata.num_packets, drop_packets);
        modify_field(scratch_metadata.num_bytes,
                     drop_bytes + capri_p4_intrinsic.packet_len);
    }
    else {
        modify_field(scratch_metadata.num_packets, permit_packets);
        modify_field(scratch_metadata.num_bytes,
                     permit_bytes + capri_p4_intrinsic.packet_len);
    }
}

@pragma stage 5
table ingress_vport_stats {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        ingress_vport_stats;
    }
    size : VPORT_STATS_TABLE_SIZE;
}

control ingress_vport_stats {
    apply(ingress_vport_stats);
}
