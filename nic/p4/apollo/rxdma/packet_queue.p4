action pkt_enqueue(PKTQ_QSTATE) {
    // in rxdma
    //          check sw_pindex0, cindex0
    //          tbl-wr sw_pindex0++
    //          doorbell(dma) pindex0
    // in txdma
    //          check sw_cindex0, pindex0
    //          tbl-wr sw_cindex0++
    //          doorbell(dma) cindex0
    // d-vector
    PKTQ_QSTATE_DVEC_SCRATCH(scratch_qstate_hdr, scratch_qstate_txdma_fte_q);

    // k-vector
    modify_field(scratch_metadata.slacl_result, p4_to_rxdma_header.slacl_result);
    modify_field(scratch_metadata.qid, capri_rxdma_intr.qid);
    modify_field(scratch_metadata.dma_size, (capri_p4_intr.packet_len +
                                             APOLLO_PREDICATE_HDR_SZ +
                                             APOLLO_P4_TO_TXDMA_HDR_SZ +
                                             APOLLO_I2E_HDR_SZ));

    // Ring0 goes to TxDMA and Ring1 goes to FTE
    // choose the ring based on slacl_result
    // check if the selected ring is full
    // increment pindex for the ring
    // compute the packet buffer address and dma desc+pkt to the packet buffer
    // ring door bell if it is TxDMA ring
    // for rings to FTE - ARM is polling so we really don't need a hw queue
    // set total rings = 1 to make the FTE rings invisible to hw, but use the
    // pindex1, cindex1... to manage the FTE rings
    // use cpu_qid_hash to select the fte ring (rss)
    modify_field(scratch_hash_results.cpu_qid_hash, hash_results.cpu_qid_hash);
}

@pragma stage 7
@pragma raw_index_table
@pragma table_write
table txdma_fte_queue {
    reads {
        // P4 pipeline must provide (LIF, Qtype, Qid)
        // TxDMA and FTE are two rings of the same Q (if it is allowed), if not
        // slacl action will compute and set qstate_addr for appropriate Q
        capri_rxdma_intr.qstate_addr : exact;
    }
    actions {
        pkt_enqueue;
    }
}

// stage 6: toeplitz table lkp => qid_hash
@pragma stage 6
@pragma hash_type 4
@pragma toeplitz_key toeplitz_key0.data toeplitz_key1.data toeplitz_key2.data
@pragma toeplitz_seed toeplitz_seed0.data toeplitz_seed1.data toeplitz_seed2.data
table rx_cpu_hash {
    reads {
        // Flit
        toeplitz_key0.data      : exact;
        toeplitz_key1.data      : exact;
        toeplitz_seed0.data     : exact;
        toeplitz_seed1.data     : exact;
        // Flit
        toeplitz_key2.data      : exact;
        toeplitz_seed2.data     : exact;
    }
    actions {
        rx_cpu_hash_result;
    }
}

action rx_cpu_hash_result() {
    // copy hash result (r7??) to phv
    modify_field(hash_results.cpu_qid_hash, 0);
}

// stage 5: lif table => toeplitz secret(seed)
@pragma stage 5
@pragma index_table
table toeplitz_seed {
    reads {
        // define a key that is always zero, single entry index table in sram
        // typically this is per lif, but in BIW mode there is only one lif
        udp_flow_meta.zero : exact;
    }
    actions {
        toeplitz_seed_init;
    }
    size : 1;
}

action toeplitz_seed_init (t_type, seed) {
    modify_field(scratch_toeplitz_seed.t_type, t_type);
    modify_field(scratch_toeplitz_seed.seed, seed);
    // seed bytes are actually moved to following phvs
    modify_field(toeplitz_seed0.data, 0);
    modify_field(toeplitz_seed1.data, 0);
    modify_field(toeplitz_seed2.data, 0);
}

// state 5: key table (mpu_only) pkt_fields (320bits) -> toeplitz_key
@pragma stage 5
table toeplitz_key {
    actions {
        toeplitz_key_init;
    }
}
action toeplitz_key_init () {
    // K vector (Input)
    modify_field(scratch_flow_key.flow_ktype, p4_to_rxdma_header.flow_ktype);
    modify_field(scratch_flow_key.flow_src, p4_to_rxdma_header.flow_src);
    modify_field(scratch_flow_key.flow_dst, p4_to_rxdma_header.flow_dst);
    modify_field(scratch_flow_key.flow_proto, p4_to_rxdma_header.flow_proto);
    modify_field(scratch_flow_key.flow_dport, p4_to_rxdma_header.flow_dport);
    modify_field(scratch_flow_key.flow_sport, p4_to_rxdma_header.flow_sport);
    // K bytes are actually moved to toeplitz key0,1,2
    modify_field(toeplitz_key0.data, 0);
    modify_field(toeplitz_key1.data, 0);
    modify_field(toeplitz_key2.data, 0);
}

control pkt_enqueue {
    if (p4_to_rxdma_header.fte_rss_enable == TRUE) {
        apply(toeplitz_key);
        apply(toeplitz_seed);
        apply(rx_cpu_hash);
    }
    if (p4_to_rxdma_header.slacl_result == 0x00) {
        // drop
    } else {
        // enqueue to FTE or TxDMA ring based on SF bit (I)
        apply(txdma_fte_queue);
    }
}
