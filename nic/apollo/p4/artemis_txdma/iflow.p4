// d-vector should match artemis_rx_to_tx2_header_t
action iflow_rx2tx(flow_hash,
             epoch,
             parent_hint_index,
             flow_nrecircs,
             parent_hint_slot,
             parent_is_hint,
             ipaf,
             parent_valid, pad0,
             vpc_id,
             pad1,
             flow_src,
             flow_sport,
             flow_dport,
             flow_proto,
             flow_dst,
             service_tag) {
    modify_field(capri_p4_intr.recirc, FALSE);
    //modify_field(txdma_predicate.pass_three, FALSE);

    modify_field(scratch_metadata.field32, flow_hash);
    modify_field(scratch_metadata.field8, epoch);
    modify_field(scratch_metadata.field22, parent_hint_index);
    modify_field(scratch_metadata.field3, flow_nrecircs);
    modify_field(scratch_metadata.field3, parent_hint_slot);
    modify_field(scratch_metadata.field1, parent_is_hint);
    modify_field(scratch_metadata.field1, ipaf);
    modify_field(scratch_metadata.field1, parent_valid);
    modify_field(scratch_metadata.field1, pad0);

    modify_field(scratch_metadata.field8, vpc_id);
    modify_field(scratch_metadata.field16, pad1);
    modify_field(scratch_metadata.field128, flow_src);
    modify_field(scratch_metadata.field16, flow_sport);
    modify_field(scratch_metadata.field16, flow_dport);
    modify_field(scratch_metadata.field8, flow_proto);
    modify_field(scratch_metadata.field128, flow_dst);
    modify_field(scratch_metadata.field32, service_tag);
    modify_field(scratch_metadata.field128, rx_to_tx_hdr.nat_ip);
    modify_field(scratch_metadata.field1, rx_to_tx_hdr.iptype);
    modify_field(scratch_metadata.field1, rx_to_tx_hdr.direction);
    modify_field(scratch_metadata.field16, rx_to_tx_hdr.xlate_port);
}

@pragma stage 0
@pragma raw_index_table
table iflow_rx2tx {
    reads {
        txdma_control.pktdesc_addr  : exact;
    }
    actions {
        iflow_rx2tx;
    }
}

action iflow_entry(flow) {
    modify_field(scratch_metadata.field512, flow);
}

@pragma stage 1
@pragma raw_index_table
table iflow_entry {
    reads {
        txdma_control.pktdesc_addr  : exact;
    }
    actions {
        iflow_entry;
    }
}

control iflow {
    apply(iflow_rx2tx);
    apply(iflow_entry);
}
