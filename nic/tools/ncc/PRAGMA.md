Pragmas
=======

* PD API
    * include_k_in_d

* PHV Allocation
    * deparser_variable_length_header
    * pa_align
    * pa_atomic
    * pa_field_union
    * pa_header_union
    * pa_parser_local
    * scratch_metadata
    * parser_end_offset
    * synthetic_header

* Parser
    * allow_set_meta (egress parser states only)
    * capture_payload_offset
    * dont_capture_payload_offset
    * deparse_only
    * dont_advance_packet
    * hdr_len
    * header_ordering
    * no_ohi
    * parser_write_only
    * no_extract
    * packet_len_check <unit_name> <start/len> ...
    * xgress

* Table
    * hash_type
    * hbm_table
    * overflow_table
    * policer_table
    * raw_index_table
    * raw_table
    * stage
    * table_write
    * toeplitz_key
    * toeplitz_seed
    * num_threads <N>
    * memory_only

* Checksum
    * checksum gress
    * checksum hdr_len_expr
    * checksum update_len <header-field>
    * checksum verify_len <write_only_ohi_variable>
    * checksum gso_csum_header
    * checksum generic_checksum_start <header-field>
    * checksum generic_checksum_offset <header-field>
    * checksum l2_complete_checksum
    * checksum udp_option

