/******************************************************************************
 * Overlay Nexthop Group
 *****************************************************************************/
action overlay_nexthop_group_info(nexthop_group_id, num_nexthop_groups) {
    if (rewrite_metadata.nexthop_group_id == 0) {
        // return
    }

    modify_field(scratch_metadata.num_nexthop_groups, num_nexthop_groups);
    if (num_nexthop_groups == 0) {
        modify_field(scratch_metadata.nexthop_group_id, nexthop_group_id);
    } else {
        modify_field(scratch_metadata.nexthop_group_id, nexthop_group_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthop_groups));
    }
    modify_field(rewrite_metadata.nexthop_group_id,
                 scratch_metadata.nexthop_group_id);
}

@pragma stage 2
table overlay_nexthop_group {
    reads {
        rewrite_metadata.nexthop_group_id   : exact;
    }
    actions {
        overlay_nexthop_group_info;
    }
    size : NEXTHOP_GROUP_TABLE_SIZE;
}

/******************************************************************************
 * Underlay Nexthop Group
 *****************************************************************************/
action underlay_nexthop_group_info(nexthop_id, num_nexthops) {
    if (rewrite_metadata.nexthop_group_id == 0) {
        // return
    }

    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_id, nexthop_id);
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(rewrite_metadata.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 3
table underlay_nexthop_group {
    reads {
        rewrite_metadata.nexthop_group_id   : exact;
    }
    actions {
        underlay_nexthop_group_info;
    }
    size : NEXTHOP_GROUP_TABLE_SIZE;
}

/******************************************************************************
 * Nexthop
 *****************************************************************************/
action nexthop_info(lif, qtype, qid, port, vni, ip_type, dipo, dmaco, dmaci) {
    if (rewrite_metadata.nexthop_id == 0) {
        egress_drop(P4E_DROP_NEXTHOP_INVALID);
    }
    modify_field(capri_intrinsic.tm_oport, port);
    if (port == TM_PORT_DMA) {
        modify_field(capri_intrinsic.lif, lif);
        modify_field(capri_rxdma_intrinsic.qtype, qtype);
        modify_field(capri_rxdma_intrinsic.qid, qid);
    }
}

@pragma stage 4
@pragma index_table
@pragma hbm_table
table nexthop {
    reads {
        rewrite_metadata.nexthop_id : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

control nexthops {
    if (control_metadata.rx_packet == FALSE) {
        apply(overlay_nexthop_group);
        apply(underlay_nexthop_group);
    }
    apply(nexthop);
}
