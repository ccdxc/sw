/*****************************************************************************/
/* Policy (IPv6 and non-IP)                                                  */
/*****************************************************************************/
@pragma capi appdatafields session_index epoch flow_role
@pragma capi hwfields_access_api
action txdma_flow_hash(entry_valid, session_index, epoch, flow_role, hash1, hint1,
                 hash2, hint2, hash3, hint3, hash4, hint4, more_hashes,
                 more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        //modify_field(service_header.flow_done, TRUE);
        modify_field(scratch_metadata.session_index, session_index);
        modify_field(scratch_metadata.flow_role, flow_role);
        //modify_field(p4i_i2e.entropy_hash, scratch_metadata.flow_hash);
        modify_field(scratch_metadata.epoch, epoch);
        modify_field(scratch_metadata.epoch, key3.epoch);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash3)) {
            modify_field(scratch_metadata.flow_hint, hint3);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash4)) {
            modify_field(scratch_metadata.flow_hint, hint4);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.flow_hash, hash3);
    modify_field(scratch_metadata.flow_hash, hash4);
    modify_field(scratch_metadata.field8, key3.flow_lkp_type);
}

@pragma stage 2
@pragma hbm_table
table txdma_flow {
    reads {
        key3.vpc_id     : exact;
        key3.ktype      : exact;
        key1.src        : exact;
        key2.src        : exact;
        key2.dst        : exact;
        key3.dst        : exact;
        key1.proto      : exact;
        key3.sport      : exact;
        key3.dport      : exact;
    }
    actions {
        txdma_flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table txdma_flow
table txdma_flow_ohash_1 {
    reads {
        key1.flow_ohash : exact;
    }
    actions {
        txdma_flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

#if 0
@pragma stage 4
@pragma hbm_table
@pragma overflow_table txdma_flow
table txdma_flow_ohash_2 {
    reads {
        key1.flow_ohash : exact;
    }
    actions {
        txdma_flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}
#endif

/*****************************************************************************/
/* Policy (IPv4)                                                             */
/*****************************************************************************/
@pragma capi appdatafields session_index epoch flow_role
@pragma capi hwfields_access_api
action txdma_ipv4_flow_hash(entry_valid, session_index, epoch, flow_role,
                      hash1, hint1, hash2, hint2, more_hashes, more_hints) {
    if (entry_valid == TRUE) {
        // if hardware register indicates hit, take the results
        //modify_field(service_header.flow_done, TRUE);
        modify_field(scratch_metadata.session_index, session_index);
        modify_field(scratch_metadata.flow_role, flow_role);
        //modify_field(p4i_i2e.entropy_hash, scratch_metadata.flow_hash);
        modify_field(scratch_metadata.epoch, epoch);
        modify_field(scratch_metadata.epoch, key_ipv4.epoch);

        // if hardware register indicates miss, compare hashes with r1
        // (scratch_metadata.flow_hash) and setup lookup in overflow table
        modify_field(scratch_metadata.flow_hash,
                     scratch_metadata.flow_hash);
        modify_field(scratch_metadata.hint_valid, FALSE);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash1)) {
            modify_field(scratch_metadata.flow_hint, hint1);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flow_hash == hash2)) {
            modify_field(scratch_metadata.flow_hint, hint2);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }
        modify_field(scratch_metadata.flag, more_hashes);
        if ((scratch_metadata.hint_valid == FALSE) and
            (scratch_metadata.flag == TRUE)) {
            modify_field(scratch_metadata.flow_hint, more_hints);
            modify_field(scratch_metadata.hint_valid, TRUE);
        }

    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.flow_hash, hash1);
    modify_field(scratch_metadata.flow_hash, hash2);
    modify_field(scratch_metadata.field8, key2_ipv4.flow_lkp_type);
}

@pragma stage 2
@pragma hbm_table
table txdma_ipv4_flow {
    reads {
        key_ipv4.vpc_id      : exact;
        key_ipv4.ipv4_src    : exact;
        key_ipv4.ipv4_dst    : exact;
        key_ipv4.proto       : exact;
        key_ipv4.sport       : exact;
        key_ipv4.dport       : exact;
    }
    actions {
        txdma_ipv4_flow_hash;
    }
    size : FLOW_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
@pragma overflow_table txdma_ipv4_flow
table txdma_ipv4_flow_ohash_1 {
    reads {
        key_ipv4.flow_ohash   : exact;
    }
    actions {
        txdma_ipv4_flow_hash;
    }
    size : FLOW_OHASH_TABLE_SIZE;
}

control flow_lookup {
    apply(txdma_ipv4_flow);
    apply(txdma_flow);

    apply(txdma_ipv4_flow_ohash_1);
    apply(txdma_flow_ohash_1);
    //apply(txdma_flow_ohash_2);
}
