#include "../../common-p4+/capri_dma_cmd.p4"
#include "../../common-p4+/capri_doorbell.p4"

header_type slacl_metadata_t {
    fields {
        pad0                        : 6;
        sport_table_addr            : 34;
        pad1                        : 6;
        ipv4_table_addr             : 34;
        pad2                        : 6;
        proto_dport_table_addr      : 34;
        pad3                        : 6;
        p1_table_addr               : 34;
        pad4                        : 6;
        p2_table_addr               : 34;
        proto_dport_class_id        : 8;
        ipv4_lpm_s2_offset          : 16;
        proto_dport_lpm_s2_offset   : 16;
        proto_dport                 : 24;
        sport_class_id              : 7;
        ip_sport_class_id           : 17;
        p1_class_id                 : 10;
        pad7                        : 30;
        ip                          : 128;
    }
}

header_type scratch_metadata_t {
    fields {
        in_packets      : 64;
        in_bytes        : 64;
        rule_id         : 10;
        class_id10      : 10;
        class_id16      : 16;
        class_id8       : 8;
        qid             : 24;
        dma_size        : 16;
        slacl_result    : 2;
        pad2            : 2;
        data510         : 510;
        data512         : 512;
        ipv4_addr       : 32;
        proto_dport     : 24;
    }
}

// UDP ordering -
header_type udp_scratch_metadata_t {
    fields {
        entry_valid         : 1;
        udp_flow_hit        : 1;
        udp_queue_delete    : 1;
        udp_flow_lkp_result : 2;
        pad0                : 1;
        udp_q_counter       : 10;

        udp_flow_qid        : 8;
        qid_bitmap          : 64;
    }
}
header_type udp_flow_metadata_t {
    fields {
        zero                : 1;
        udp_flow_lkp_result : 2;
        pad0                : 1;
        udp_qid_tbl_idx     : 12;   // stg_id | udp_oflow_index[10:0]
        udp_flow_qid        : 8;
    }
}
header_type toeplitz_seed_t {
    fields {
        t_type  : 8;
        seed    : 320;
    }
}
header_type toeplitz_key0_t {
    fields {
        data : 128;
    }
}
header_type toeplitz_key1_t {
    fields {
        data : 128;
    }
}
header_type toeplitz_key2_t {
    fields {
        data : 64;
    }
}

header_type toeplitz_result_t {
    fields {
        cpu_qid_hash : 32;
    }
}

header_type flow_key_t {
    fields {
        pad                 : 12;
        flow_ktype          : 4;
        flow_src            : 128;
        flow_dst            : 128;
        flow_proto          : 8;
        flow_dport          : 16;
        flow_sport          : 16;
    }
}

// Phv header instantiation -
// start with intrinsic followed by p4_to_rxdma..., followed by rxdma metadata
@pragma dont_trim
metadata cap_phv_intr_global_t capri_intr;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intr;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t capri_rxdma_intr;

@pragma dont_trim
metadata p4_to_arm_header_t p4_to_arm_header;
@pragma dont_trim
metadata p4_to_rxdma_header_t p4_to_rxdma_header;

@pragma dont_trim
metadata slacl_metadata_t       slacl_metadata;

@pragma dont_trim
@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_txdma_fte_q_t   scratch_qstate_txdma_fte_q;

@pragma dont_trim
@pragma scratch_metadata
metadata udp_scratch_metadata_t udp_scratch;

@pragma dont_trim
metadata udp_flow_metadata_t    udp_flow_metadata;

metadata toeplitz_result_t      hash_results;

@pragma scratch_metadata
metadata toeplitz_result_t      scratch_hash_results;
@pragma scratch_metadata
metadata toeplitz_seed_t        scratch_toeplitz_seed;

@pragma scratch_metadata
metadata flow_key_t             scratch_flow_key;

@pragma pa_align 512
// key and seed MUST come from the same flit (h/w req.t)
metadata toeplitz_key0_t  toeplitz_key0;  // from packet
metadata toeplitz_key0_t  toeplitz_seed0; // same size as key
metadata toeplitz_key1_t  toeplitz_key1;  // from packet
metadata toeplitz_key1_t  toeplitz_seed1; // same size as key

@pragma pa_align 512
// key and seed MUST come from the same flit (h/w req.t)
metadata toeplitz_key2_t  toeplitz_key2;  // from packet
metadata toeplitz_key2_t  toeplitz_seed2; // same size as key

@pragma dont_trim
metadata doorbell_addr_t    doorbell_addr;
@pragma dont_trim
metadata doorbell_data_t    doorbell_data;

// DMA commands
@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd_pkt2mem;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_phv2mem;
