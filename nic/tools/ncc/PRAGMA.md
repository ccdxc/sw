Pragmas
=======

* PHV Allocation
    * deparser_pad_header
    * deparser_variable_length_header
    * pa_align <bit_alignement_in_power_of_2_upto_512>
        Align the header to specified phv bit boundary
    * pa_atomic
        Keep all the fields of the metadata header together
    * pa_field_union <direction> <list of header.field>
        Create field union of two or more header fields. Metadata fields are allowed only for
        synthetic header
    * pa_header_union <direction> <list of header instances>
        Create union of two or more header instances
    * pa_parser_local
        Designate the header as parser local variable, no phv is allocated for it
    * parser_end_offset <field_name>
        Designate the specified field to carry parser end offset. It must be allocated at specific 
        phv locations allowed by the hardware
    * scratch_metadata
        No phv is allocated for this metadata
    * synthetic_header
        Header is constructed using fields from other packet and metadata headers.
        Must use field unions to create the mapping between synthetic field and its source.
        Synthetic headers cannot be extracted. Only deparsed.

* Parser
    * allow_set_meta <metadata_fieldname>
        Allows set_metadata operation in the egress parser. These are ignored in egress parser by 
        default
    * capture_payload_offset
        Instructs parser to caprture the offset in a given state
    * deparse_only
        Following state is not used by the parser
    * dont_advance_packet
        Extract data from packet but do not advance the offset. Allows repeated extract of the same
        bytes.
    * dont_capture_payload_offset
        Do not capture current offset in the follwoing state. Applied to end states, where offset is
        captured by default
    * hdr_len <field_name>
        Specifies a field that contains the length of the header. Used for varialble len
        headers and payload parsing
    * header_ordering <header list>
        Used to determine header ordering when there are loops in parse graph (E.g option parsing)
    * no_extract
        Advance the offset w/o actually extracting the packet bytes
    * no_ohi <direction>
        Do not place header fields to ohi even if not used in the pipeline (useful for debugging)
    * packet_len_check <profile_name> len <gt|lt|eq> <ohi_field_name>
    * packet_len_check <profile_name> start <field_name>
        Instruct the hardware to create packe len check profile and ensure required bytes are
        present in the packet
    * parser_share_ohi <list of ohi_fields>
        Share the same ohi slot for listed field
    * parser_write_only
        Assign all fields of the header to ohi slots
    * xgress <dir>
        Following state is applicable only in the specified direction (pipeline)

* Table
    * enable_rate_limit
    * hash_type
        Specify the polynomial selector for hashing
    * hbm_table
        Locate table in HBM memory
    * index_table
        Use direct index lookup for this table irrespective of table size and key width
    * little_endian
        Header files for MPU assembler use little endian format for action parameters
    * memory_only
        The table does not perform phvwr, hardware does not have to wait for mpu to finish execution
    * numthreads <N>
        The same table is launched on N consecutive cycles using different table profiles
    * overflow_table <parent_hash_table>
        The following table is overflow(collision) table for specified parent hash table
    * policer_table
        The table is used as a policer
    * raw_index_table
    * raw_table
    * scratch_metadata
    * stage <X>
        Place the table in stage X
    * table_write
        Create a 'locked' table
    * three_color
    * toeplitz_key <list of fields that contain toeplitz key>
    * toeplitz_seed <list of fields that contain toeplits seed/secret>

* Checksum
    * checksum gress :
        Specifies pipeline where checksum verification or computation is done.
    * checksum hdr_len_expr <parser expression>:
        Specifies L3 header length expression. Used when ipv4 options are present.
        IPV4 header checksum length has to include option length.
    * checksum update_len <field name from header instance with pragma deparser_variable_length_header>:
        Specifies field name that is used as length overwhich checksum is computed  by deparser.
    * checksum verify_len <write_only_ohi_variable>
        Specifies field name that is used as length overwhich checksum is verified by parser.
    * checksum gso_csum_header
        Applied on header instance. The fields of this header align with parser requirement
        of phv location for GSO checksum.
    * checksum generic_checksum_start <header-field>
        Applied on parser state to trigger GSO checksum computation. Applied on those parser states
        to which parser engine branches because GSO valid is true in instrinsic header.
    * checksum generic_checksum_offset <header-field>
        Specifies location of checksum value in the packet where computed value should be stored.
    * checksum udp_option
        Specifies checksum field list is udp option checksum even though "payload" field
        is specified in field list.
    * checksum verify {ingress|egress|xgress}
        Specifies the pipeline(s) in which this checksum has to be verified
    * checksum update {ingress|egress|xgress}
        Specifies the pipeline(s) in whcih this checksum has to be computed

* PD API
    * include_k_in_d
        Directive for code generation to include key fields in data.

