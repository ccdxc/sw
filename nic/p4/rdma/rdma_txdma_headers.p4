
header_type roce_recirc_header_t {
    fields {
        recirc_reason              : 4;
        app_header_table_valid     : 4;
        recirc_spec_cindex         : 16;
    }
}
