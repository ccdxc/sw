/*****************************************************************************/
/* NAT To find Local IP for Rx Direction (Switch to Host Traffic)           */
/*****************************************************************************/
action find_local_ip(nat_ip) {
        // No need to do direction check here, as for Traffic from host,
        // xlate_idx would be zero, so nat_ip would be zero
        if (nat_ip != 0) {
            // Update FlowDst with this Local IP address derived from NAT table
            // Only do it if nat_ip is valid, otherwise it means no NAT and
            // dont need to update
            modify_field(p4_to_rxdma.flow_dst, nat_ip);
        }
}

@pragma stage 1
@pragma hbm_table
@pragma index_table
table nat_rxdma {
    reads {
        rxdma_control.xlate_idx  : exact;
    }
    actions {
        find_local_ip;
    }
    size : NAT_TABLE_SIZE;
}

// Even for non-NAT traffic, we always does NAT lookup in CPS Path, so
// leave the entry zero with zero nat_ip, and in that case we will not override the
// flow key passed from the P4 for traffic towards host
control nat_rxdma {
    apply(nat_rxdma);
}
