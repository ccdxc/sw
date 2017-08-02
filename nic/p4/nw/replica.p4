/*****************************************************************************/
/* Replica processing                                                        */
/*****************************************************************************/
action set_replica_rewrites() {
    modify_field(control_metadata.flow_miss_egress, control_metadata.flow_miss);
    modify_field(tunnel_metadata.tunnel_originate_egress,
                 tunnel_metadata.tunnel_originate);
    modify_field(tunnel_metadata.tunnel_terminate_egress,
                 tunnel_metadata.tunnel_terminate);

    if (tm_replication_data.valid == TRUE) {
        modify_field(capri_intrinsic.lif, tm_replication_data.lif);
        modify_field(capri_rxdma_intrinsic.qtype, tm_replication_data.qtype);
        modify_field(rewrite_metadata.rewrite_index,
                     tm_replication_data.rewrite_index);
        if (tm_replication_data.tunnel_rewrite_index == 0) {
            modify_field(capri_rxdma_intrinsic.qid,
                         tm_replication_data.qid_or_vnid);
        } else {
            modify_field(tunnel_metadata.tunnel_originate_egress, TRUE);
            modify_field(rewrite_metadata.tunnel_rewrite_index,
                         tm_replication_data.tunnel_rewrite_index);
            modify_field(rewrite_metadata.tunnel_vnid,
                         tm_replication_data.qid_or_vnid);
        }
        remove_header(tm_replication_data);
    }
}

@pragma stage 0
table replica {
    actions {
        set_replica_rewrites;
    }
}

control process_replica {
    apply(replica);
}
