action iflow_rx2tx(flow_hash,
             parent_hint_index,
             flow_nrecircs,
             parent_hint_slot,
             parent_is_hint,
             ipaf) {
    modify_field(capri_p4_intr.recirc, FALSE);
    //modify_field(txdma_predicate.pass_three, FALSE);

    modify_field(scratch_metadata.field32, flow_hash);
    modify_field(scratch_metadata.field22, parent_hint_index);
    modify_field(scratch_metadata.field3, flow_nrecircs);
    modify_field(scratch_metadata.field3, parent_hint_slot);
    modify_field(scratch_metadata.field1, parent_is_hint);
    modify_field(scratch_metadata.field1, ipaf);
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
