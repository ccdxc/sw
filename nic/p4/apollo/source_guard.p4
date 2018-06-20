/*****************************************************************************/
/* Source guard feature                                                      */
/*****************************************************************************/
action source_guard_info(entry_valid, src,
                         hash1, hint1, hash2, hint2, hash3, hint3, hash4, hint4,
                         hash5, hint5, hash6, hint6, hashn, hintn) {

    if (entry_valid == TRUE) {
        if (control_metadata.direction == RX_PACKET) {
            if (((ipv4_1.valid == TRUE) and (ipv4_1.srcAddr != src)) or
                ((ipv6_1.valid == TRUE) and (ipv6_1.srcAddr != src))) {
                modify_field(control_metadata.drop_reason, 0);
            }
        } else {
            if (ethernet_1.srcAddr != src) {
                modify_field(control_metadata.drop_reason, 0);
            }
        }
        // if hardware register indicates hit, take the results
        modify_field(service_header.source_guard_done, TRUE);

        // if hardware register indicates miss, compare hints and setup
        // to perform lookup in overflow table
        modify_field(service_header.source_guard_ohash, hint1);
        modify_field(control_metadata.source_guard_ohash_lkp, TRUE);
    }

    modify_field(scratch_metadata.flag, entry_valid);
    modify_field(scratch_metadata.ep_hash, hash1);
    modify_field(scratch_metadata.ep_hash, hash2);
    modify_field(scratch_metadata.ep_hash, hash3);
    modify_field(scratch_metadata.ep_hash, hash4);
    modify_field(scratch_metadata.ep_hash, hash5);
    modify_field(scratch_metadata.ep_hash, hash6);
    modify_field(scratch_metadata.ep_hash, hashn);
    modify_field(scratch_metadata.ep_hint, hint1);
    modify_field(scratch_metadata.ep_hint, hint2);
    modify_field(scratch_metadata.ep_hint, hint3);
    modify_field(scratch_metadata.ep_hint, hint4);
    modify_field(scratch_metadata.ep_hint, hint5);
    modify_field(scratch_metadata.ep_hint, hint6);
    modify_field(scratch_metadata.ep_hint, hintn);
    modify_field(scratch_metadata.ip_src, src);
}

@pragma stage 2
@pragma hbm_table
table source_guard {
    reads {
        control_metadata.subnet_id  : exact;
        key_metadata.src            : exact;
    }
    actions {
        source_guard_info;
    }
    size : SOURCE_GUARD_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
@pragma overflow_table source_guard
table source_guard_ohash {
    reads {
        service_header.source_guard_ohash : exact;
    }
    actions {
        source_guard_info;
    }
    size : SOURCE_GUARD_OHASH_TABLE_SIZE;
}

control source_guard {
    if (service_header.valid == FALSE) {
        apply(source_guard);
    }
    if (control_metadata.source_guard_ohash_lkp == TRUE) {
        apply(source_guard_ohash);
    }
}
