/*****************************************************************************/
/* NAT To find Local IP for Rx Direction (Switch to Host Traffic)           */
/*****************************************************************************/
action find_local_ip(nat_ip) {
        // Only do it if nat_ip is valid, otherwise it means no NAT and
        // dont need to update
        if (nat_ip != 0) {
            if (p4_to_rxdma.direction == TX_FROM_HOST) {
                // In Tx dir, Policy evaluation should happen based on pre-nat src addr
                // so dont override the flow keys, but NAT address is reqd as rflow
                // sip.  Carry this nat ip in nat_ip of rx_to_tx header.
                modify_field(rx_to_tx_hdr.nat_ip, nat_ip);
            } else {  // RX Direction / TO_HOST
                // Update FlowDst with this Local IP address derived from NAT table
                modify_field(p4_to_rxdma.flow_dst, nat_ip);
                // Update FlowDst with this Xlate_port received from P4, if its non-zero
                modify_field(p4_to_rxdma.flow_dport, p4_to_rxdma2.xlate_port);
            }
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
