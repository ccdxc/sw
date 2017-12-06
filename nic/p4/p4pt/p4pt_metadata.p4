

// defines app header passed from p4 to p4_plus as input to first stage
header_type p4pt_iscsi_app_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        p4pt_idx            : 24;
        payload_len         : 16;
        pad                 : 6;
        flow_role           : 1;
        flow_dir            : 1;

        // raw iscsi header: ref: https://tools.ietf.org/html/rfc3720#section-10.2.4
        // upto 33 bytes (or lesser); union casted from payload
        skip0               : 2;
        opcode              : 6;  // cmd = 0x01, resp = 0x21
        skip1               : 1;
        cmd_read            : 1;
        cmd_write           : 1;
        skip2               : 13;
        resp_status         : 8;  // only valid for resp
        skip3               : 32; 
        cmd_lun             : 64;
        tag_id              : 32;
        cmd_data_length     : 32; // for cmd, from expected data transfer length
        skip4               : 64;
        cmd_scsi_cdb_op     : 8;  // specifies read/write scsi operation
    }
}

// p4ptcb index by p4pt_idx, contains l7 records for various keys within l7 payload
header_type p4pt_tcb_t {
    fields {
        // L7 Records - populated dynamically as sessions discovered within a flow
        // There are upto 10 kept at a given time for each tcb (connection)
        // These records are identified using uint32 key (a hash, or the value itself)
        // The address for the record is base_address + num_rec*sizeof(rec)
        // A record is protocol specific cb e.g. p4pt_tcb_iscsi_rec
        // TBD: base_addr and rec_size could be predefined for a given protocol instead

        next_idx:        4;  // circular index allocated to next command
        base_addr:       16; // base_address of the first l7-rec
        rec_size:        4;  // rec_size is the rec's size in words (32 bits)
        reserved0:       8;

        rec_key0:        32;
        rec_key1:        32;
        rec_key2:        32;
        rec_key3:        32;
        rec_key4:        32;
        rec_key5:        32;
        rec_key6:        32;
        rec_key7:        32;
        rec_key8:        32;
        rec_key9:        32;

        reserved:        160;
    }
}

// p4pt_tcb_iscsi_stats define common metrics per tcb (aka connection) 
header_type p4pt_tcb_iscsi_stats_t {
    fields {
         read_reqs:             32;
         write_reqs:            32;
         read_bytes:            32;
         write_bytes:           64;
         read_errors:           64;
         write_errors:          32;
         total_read_latency:    64;
         total_read_resps:      32;
         total_write_latency:   64;
         total_write_resps:     32;
         reserved:              64;
    }
}

// p4pt_tcb_iscsi_latency_distribution define latency distribution in 16 buckets of log scale
// total capacity: 512 bits
header_type p4pt_tcb_iscsi_read_latency_distribution_t {
    fields {
        range0:  32;
        range1:  32;
        range2:  32;
        range3:  32;
        range4:  32;
        range5:  32;
        range6:  32;
        range7:  32;
        range8:  32;
        range9:  32;
        range10: 32;
        range11: 32;
        range12: 32;
        range13: 32;
        range14: 32;
        range15: 32;
    }
}

// p4pt_tcb_iscsi_write_latency_distribution_t define latency distribution in 16 buckets of log scale
// total capacity: 512 bits
header_type p4pt_tcb_iscsi_write_latency_distribution_t {
    fields {
        range0:  32;
        range1:  32;
        range2:  32;
        range3:  32;
        range4:  32;
        range5:  32;
        range6:  32;
        range7:  32;
        range8:  32;
        range9:  32;
        range10: 32;
        range11: 32;
        range12: 32;
        range13: 32;
        range14: 32;
        range15: 32;
    }
}
  
// each p4pt_iscsi_rec keeps per (lun, tag_id) telemetry information
header_type p4pt_tcb_iscsi_rec_t {
    fields {
        reserved:       22; // reserved
        read:           1;  // 1 if this is a read operation
        write:          1;  // 1 if this is write operation
        status:         8;  // last status code
        tag_id:         32; // tag_id
        lun:            64; // lun corresponding to this rec
        req_timestamp:  64; // timestamp for req/resp correlation
        data_length:    32; // data_length (read/write) request
        reserved1:      32; // reserved
   }
}
  
// p4pt global header; contains enough information to do lookups in various stages
// max: 128 bits
header_type p4pt_global_t {
    fields {
        qid:            24; // protocol identifier
        p4pt_idx:       24; // ptp4_idx is the tcb's idx
        rec_idx:        4;  // rec_idx (0-9) that this session is assigned to or belong to
        latency:        32; // would latency be more than 32 bits wide?
        exit:           1;  // equivalent of bail out, no further processing needed
        reserved:       43; // reserved
    }
}
  
// p4pt inter stage header; contains enough information to do lookups in various stages
// max: 160 bits
header_type p4pt_s2s_t {
    fields {
        req:            1;  // 1 if this is a request packet
        resp:           1;  // 1 if this is a response packet
        read:           1;  // 1 if this is a read operation, 0 otherwise
        write:          1;  // 1 if this is a read operation, 0 otherwise
        reserved:       28; // reserved
        status:         8;  // only valid for resp
        data_length:    24;
        lun:            64; // only for cmd
        tag_id:         32;
    }
}
