/*****************************************************************************/
/* Ingress VNIC stats                                                        */
/*****************************************************************************/
action vnic_tx_stats(out_packets, out_bytes) {
    add(scratch_metadata.in_packets, out_packets, 1);
    add(scratch_metadata.in_bytes, out_bytes, capri_p4_intrinsic.packet_len);
}

@pragma stage 5
@pragma index_table
@pragma table_write
table vnic_tx_stats {
    reads {
        vnic_metadata.vnic_id   : exact;
    }
    actions {
        vnic_tx_stats;
    }
    size : VNIC_TABLE_SIZE;
}

/*****************************************************************************/
/* Ingress drop stats                                                        */
/*****************************************************************************/
action p4i_drop_stats(drop_stats_pad, drop_stats_pkts) {
    add_header(capri_p4_intrinsic);
    remove_header(arm_to_p4i);
    remove_header(p4plus_to_p4_vlan);
    remove_header(p4plus_to_p4);
    remove_header(capri_txdma_intrinsic);
    modify_field(scratch_metadata.drop_stats_pad, drop_stats_pad);
    modify_field(scratch_metadata.drop_stats_pkts, drop_stats_pkts);

    if (control_metadata.rx_packet == FALSE) {
        // LIF Tx drop stats updated using memwr in ASM
        modify_field(p4i_i2e.src_lif, p4i_i2e.src_lif);
        if (ethernet_1.dstAddr == 0xFFFFFFFFFFFF) {
            // broadcast
            add_to_field(scratch_metadata.in_bytes,
                         capri_p4_intrinsic.packet_len);
            add_to_field(scratch_metadata.in_packets, 1);
        } else {
            if ((ethernet_1.dstAddr & 0x010000000000) != 0) {
                // multicast
                add_to_field(scratch_metadata.in_bytes,
                             capri_p4_intrinsic.packet_len);
                add_to_field(scratch_metadata.in_packets, 1);
            } else {
                // unicast
                add_to_field(scratch_metadata.in_bytes,
                             capri_p4_intrinsic.packet_len);
                add_to_field(scratch_metadata.in_packets, 1);
            }
        }
    }
}

@pragma stage 5
@pragma table_write
table p4i_drop_stats {
    reads {
        control_metadata.p4i_drop_reason    : ternary;
    }
    actions {
        p4i_drop_stats;
    }
    size : DROP_STATS_TABLE_SIZE;
}

control ingress_stats {
    if (control_metadata.rx_packet == FALSE) {
        apply(vnic_tx_stats);
    }
    if (capri_intrinsic.drop == TRUE) {
        apply(p4i_drop_stats);
    }
}

/*****************************************************************************/
/* Egress VNIC stats                                                         */
/*****************************************************************************/
action vnic_rx_stats(in_packets, in_bytes) {
    add(scratch_metadata.in_packets, in_packets, 1);
    add(scratch_metadata.in_bytes, in_bytes, capri_p4_intrinsic.packet_len);
}

@pragma stage 5
@pragma index_table
@pragma table_write
table vnic_rx_stats {
    reads {
        vnic_metadata.rx_vnic_id    : exact;
    }
    actions {
        vnic_rx_stats;
    }
    size : VNIC_TABLE_SIZE;
}

action p4e_drop_stats(drop_stats_pad, drop_stats_pkts) {
    modify_field(scratch_metadata.drop_stats_pad, drop_stats_pad);
    modify_field(scratch_metadata.drop_stats_pkts, drop_stats_pkts);

    if ((control_metadata.rx_packet == FALSE) and
        (control_metadata.span_copy == FALSE)) {
        // LIF Tx drop stats updated using memwr in ASM
        modify_field(p4e_i2e.src_lif, p4e_i2e.src_lif);
        if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_UNICAST) {
            add_to_field(scratch_metadata.in_bytes, meter_metadata.meter_len);
            add_to_field(scratch_metadata.in_packets, 1);
        }
        if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_MULTICAST) {
            add_to_field(scratch_metadata.in_bytes, meter_metadata.meter_len);
            add_to_field(scratch_metadata.in_packets, 1);
        }
        if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_BROADCAST) {
            add_to_field(scratch_metadata.in_bytes, meter_metadata.meter_len);
            add_to_field(scratch_metadata.in_packets, 1);
        }
    }
}

@pragma stage 5
@pragma table_write
table p4e_drop_stats {
    reads {
        control_metadata.p4e_drop_reason    : ternary;
    }
    actions {
        p4e_drop_stats;
    }
    size : DROP_STATS_TABLE_SIZE;
}

control egress_stats {
    if (control_metadata.is_local == TRUE) {
        apply(vnic_rx_stats);
    }
}

/*****************************************************************************/
/* Lif Tx stats                                                              */
/*****************************************************************************/
action lif_tx_stats(ucast_bytes, ucast_pkts, mcast_bytes, mcast_pkts,
                    bcast_bytes, bcast_pkts, pad1, pad2) {
    if ((egress_recirc.mapping_done == FALSE) or
        (control_metadata.rx_packet == TRUE) or
        (control_metadata.span_copy == TRUE) or
        (p4e_to_arm.valid == TRUE)) {
        //return;
    }

    // stats update implemented using tbladd in ASM
    if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_UNICAST) {
        add(scratch_metadata.in_bytes, ucast_bytes, meter_metadata.meter_len);
        add(scratch_metadata.in_packets, ucast_pkts, 1);
    }
    if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_MULTICAST) {
        add(scratch_metadata.in_bytes, mcast_bytes, meter_metadata.meter_len);
        add(scratch_metadata.in_packets, mcast_pkts, 1);
    }
    if (p4e_to_p4plus_classic_nic.l2_pkt_type == PACKET_TYPE_BROADCAST) {
        add(scratch_metadata.in_bytes, bcast_bytes, meter_metadata.meter_len);
        add(scratch_metadata.in_packets, bcast_pkts, 1);
    }
    modify_field(scratch_metadata.in_bytes, pad1);
    modify_field(scratch_metadata.in_packets, pad2);
}

@pragma stage 5
@pragma hbm_table
@pragma index_table
@pragma table_write
table lif_tx_stats {
    reads {
        control_metadata.lif_tx_stats_id    : exact;
    }
    actions {
        lif_tx_stats;
    }
    size : LIF_STATS_TABLE_SIZE;
}
