/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
action rx_vport_stats(permit_packets, permit_bytes,
                      drop_packets, drop_bytes) {
#ifdef SUPPORT_GFT_GTEST
    if (capri_intrinsic.lif == EXCEPTION_VPORT) {
        add_header(capri_txdma_intrinsic);
        add_header(p4plus_to_p4);
        modify_field(capri_intrinsic.tm_oport, TM_PORT_EGRESS);
        if ((roce_bth_1.valid == TRUE) or (roce_bth_2.valid == TRUE)) {
            modify_field(p4plus_to_p4.p4plus_app_id, P4PLUS_APPTYPE_RDMA);
        }
    }
#endif
    add_header(capri_p4_intrinsic);
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
@pragma table_write
table rx_vport_stats {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        rx_vport_stats;
    }
    size : VPORT_STATS_TABLE_SIZE;
}

control rx_vport_stats {
    apply(rx_vport_stats);
}

/******************************************************************************/
/* Tx pipeline                                                                */
/******************************************************************************/
action tx_vport_stats(permit_packets, permit_bytes,
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

@pragma stage 4
@pragma table_write
table tx_vport_stats {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        tx_vport_stats;
    }
    size : VPORT_STATS_TABLE_SIZE;
}

control tx_vport_stats {
    apply(tx_vport_stats);
}

