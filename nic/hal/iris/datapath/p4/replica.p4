/*****************************************************************************/
/* Replica processing                                                        */
/*****************************************************************************/
action set_replica_rewrites() {
    // modify_field(control_metadata.nic_mode_e,
    //              control_metadata.i2e_flags >> P4_I2E_FLAGS_NIC_MODE);
    modify_field(tunnel_metadata.tunnel_originate_egress,
                 tunnel_metadata.tunnel_originate);
    modify_field(tunnel_metadata.tunnel_terminate_egress,
                 tunnel_metadata.tunnel_terminate);
    modify_field(control_metadata.src_tm_iq, capri_intrinsic.tm_iq);
    modify_field(capri_intrinsic.tm_iq, capri_intrinsic.tm_oq);

    if ((capri_intrinsic.tm_instance_type == TM_INSTANCE_TYPE_CPU) or
        (capri_intrinsic.tm_instance_type == TM_INSTANCE_TYPE_CPU_AND_DROP)) {
        modify_field(control_metadata.dst_lport, CPU_LPORT);
        modify_field(control_metadata.cpu_copy, TRUE);
        modify_field(rewrite_metadata.tunnel_rewrite_index, 0);
        modify_field(rewrite_metadata.rewrite_index, 0);
    }

    if (tm_replication_data.valid == TRUE) {
        if ((tm_replication_data.repl_type == TM_REPL_TYPE_DEFAULT) or
            (tm_replication_data.repl_type == TM_REPL_TYPE_TO_CPU_REL_COPY)) {
            // Only in these cases we need to do source port suppression check.
            // For the case where we will honor ingress we will never have the
            // FTE Programming where src_lport and dst_lport are same.
            if ((control_metadata.src_lport == tm_replication_data.lport) or
                ((tm_replication_data.is_tunnel == TRUE) and
                 (tunnel_metadata.tunnel_terminate == TRUE))) {
                modify_field(control_metadata.same_if_check_failed, TRUE);
            }
            modify_field(control_metadata.dst_lport, tm_replication_data.lport);
            modify_field(control_metadata.qtype, tm_replication_data.qtype);
            modify_field(rewrite_metadata.rewrite_index,
                     tm_replication_data.rewrite_index);

            if (tm_replication_data.is_qid == TRUE) {
                modify_field(control_metadata.qid,
                         tm_replication_data.qid_or_vnid);
            } else {
                modify_field(rewrite_metadata.tunnel_vnid,
                         tm_replication_data.qid_or_vnid);
            }
            if (tm_replication_data.is_tunnel == TRUE) {
                modify_field(tunnel_metadata.tunnel_originate_egress, TRUE);
            } else {
                modify_field(tunnel_metadata.tunnel_originate_egress, FALSE);
            }
            modify_field(rewrite_metadata.tunnel_rewrite_index,
                         tm_replication_data.tunnel_rewrite_index);
        }

        if (tm_replication_data.repl_type == TM_REPL_TYPE_HONOR_INGRESS) {
            // Honor all the rewrites that Ingress flow lookup has given out.
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
