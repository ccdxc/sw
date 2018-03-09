header_type capri_i2e_metadata_t {
    // This is a dummy header, compiler will find all the fields shared between
    // ingress and egress pipes and populate this header
    // XXX Can provide a cmd line option to use user defined i2e header. When
    // defined by a user, it must be a synthetic header
    fields {
        _dummy : 32;
    }
}
