/*****************************************************************************/
/* QOS table processing                                                      */
/*****************************************************************************/

header_type qos_metadata_t {
    fields {
        qos_class_id : 5;
        cos_en       : 1;
        cos          : 3;
        dscp_en      : 1;
        dscp         : 8;
    }
}
metadata qos_metadata_t qos_metadata;

action qos(egress_tm_oq, dest_tm_oq, cos_en, cos, dscp_en, dscp) {
    modify_field(control_metadata.lif_tx_stats_idx,
                 ((control_metadata.src_lif << 4) +
                  (LIF_STATS_TX_UCAST_BYTES_OFFSET / 64)));
    /* copy the oq to iq, needed by PBC */
    if (capri_intrinsic.tm_oq != TM_P4_RECIRC_QUEUE) {
        modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);
    }

    /* qos rewrite data */
    modify_field(qos_metadata.cos_en, cos_en);
    modify_field(qos_metadata.dscp_en, dscp_en);
    modify_field(qos_metadata.cos, cos);
    modify_field(qos_metadata.dscp, dscp);

    /* Output queue selection */
    modify_field(capri_intrinsic.tm_oq, egress_tm_oq);
    modify_field(control_metadata.dest_tm_oq, dest_tm_oq);

    /* promote size of data fields to multiple of bytes */
    modify_field(scratch_metadata.size8, cos_en);
    modify_field(scratch_metadata.size8, cos);
    modify_field(scratch_metadata.size8, dscp_en);
}

@pragma stage 4
table qos {
    reads {
        qos_metadata.qos_class_id : exact;
    }
    actions {
        qos;
    }
    size: QOS_TABLE_SIZE;
}

control process_qos {
    apply(qos);
}
