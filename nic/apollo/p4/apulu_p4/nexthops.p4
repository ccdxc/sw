/******************************************************************************
 * Overlay Nexthop Group
 *****************************************************************************/
action overlay_nexthop_group_info(nexthop_id, nexthop_type, num_nexthops) {
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_id, nexthop_id);
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(rewrite_metadata.nexthop_type, nexthop_type);
    modify_field(txdma_to_p4e.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 2
table overlay_nexthop_group {
    reads {
        txdma_to_p4e.nexthop_id : exact;
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
    modify_field(scratch_metadata.num_nexthops, num_nexthops);
    if (num_nexthops == 0) {
        modify_field(scratch_metadata.nexthop_id, nexthop_id);
    } else {
        modify_field(scratch_metadata.nexthop_id, nexthop_id +
            (p4e_i2e.entropy_hash % scratch_metadata.num_nexthops));
    }
    modify_field(txdma_to_p4e.nexthop_id, scratch_metadata.nexthop_id);
}

@pragma stage 3
table underlay_nexthop_group {
    reads {
        txdma_to_p4e.nexthop_id : exact;
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
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    } else {
        modify_field(capri_intrinsic.tm_oq, capri_intrinsic.tm_iq);
    }
    if (txdma_to_p4e.nexthop_id == 0) {
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
        txdma_to_p4e.nexthop_id : exact;
    }
    actions {
        nexthop_info;
    }
    size : NEXTHOP_TABLE_SIZE;
}

control nexthops {
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_OVERLAY) {
        apply(overlay_nexthop_group);
    }
    if (rewrite_metadata.nexthop_type == NEXTHOP_TYPE_UNDERLAY) {
        apply(underlay_nexthop_group);
    }
    apply(nexthop);
}
