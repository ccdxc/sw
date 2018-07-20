header_type lpm_metadata_t {
    fields {
        done        : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        lpm_data        : 15;
    }
}

metadata lpm_metadata_t         lpm_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
