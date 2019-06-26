/******************************************************************************/
/* VNIC mapping                                                               */
/******************************************************************************/
action vnic_mapping_info(epoch, vnic_id, vpc_id) {
    // if table lookup is a miss, return

    modify_field(control_metadata.epoch, epoch);
    modify_field(vnic_metadata.vnic_id, vnic_id);
    modify_field(vnic_metadata.vpc_id, vpc_id);
}

@pragma stage 1
table vnic_mapping {
    reads {
        capri_intrinsic.lif : ternary;
        ctag_1.vid          : ternary;
        ethernet_1.srcAddr  : ternary;
    }
    actions {
        vnic_mapping_info;
    }
    size : VNIC_MAPPING_TABLE_SIZE;
}

control ingress_vnic_info {
    if (vxlan_1.valid == FALSE) {
        apply(vnic_mapping);
    }
}

/******************************************************************************/
/* Egress VNIC info                                                           */
/******************************************************************************/
action egress_vnic_info(vr_mac, ca_mac, local_vlan, port, lif, qtype, qid) {
    remove_header(txdma_to_p4e);
    remove_header(predicate_header);
    remove_header(p4e_i2e);
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    // modify_field(rewrite_metadata.pa_mac, R5);
    if (control_metadata.direction == RX_FROM_SWITCH) {
        if (RX_REWRITE(rewrite_metadata.flags, SMAC, FROM_VRMAC)) {
            modify_field(ethernet_1.srcAddr, vr_mac);
        }
        modify_field(ethernet_1.dstAddr, ca_mac);
        modify_field(capri_intrinsic.tm_oport, port);
        if (local_vlan != 0) {
            modify_field(ethernet_1.etherType, ETHERTYPE_VLAN);
            add_header(ctag_1);
            modify_field(ctag_1.etherType, ethernet_1.etherType);
            modify_field(ctag_1.vid, local_vlan);
            add(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len, 4);
        }
        if (port == TM_PORT_DMA) {
            modify_field(capri_intrinsic.lif, lif);
            modify_field(capri_rxdma_intrinsic.qtype, qtype);
            modify_field(capri_rxdma_intrinsic.qid, qid);
        }
    }
}

@pragma stage 1
@pragma index_table
table egress_vnic_info {
    reads {
        p4e_i2e.vnic_id : exact;
    }
    actions {
        egress_vnic_info;
    }
    size : VNIC_INFO_TABLE_SIZE;
}

control egress_vnic_info {
    apply(egress_vnic_info);
}
