header_type slacl_metadata_t {
    fields {
        pad0        : 6;
        addr3       : 34;
        pad1        : 4;
        stats_index : 20;
        class_id0   : 10;
        class_id1   : 10;
        class_id2   : 8;
        class_id3   : 8;
        class_id4   : 4;
        class_id5   : 10;
        class_id6   : 10;
        class_id7   : 10;
        drop        : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        in_packets      : 64;
        in_bytes        : 64;
        rule_id         : 10;
        class_ids       : 510;
        class_id10      : 10;
        class_id8       : 8;
        class_pad       : 2;
    }
}

metadata slacl_metadata_t       slacl_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
