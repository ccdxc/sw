#include "../../common-p4+/capri_dma_cmd.p4"
#include "../../common-p4+/capri_doorbell.p4"

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
        sl_result      : 2;    // (sf,sl) encoded value
    }
}

header_type rxdma_predicates_t {
    fields {
        sl_result                   : 2;    // (sf,sl) encoded value
        udp_flow_lkp_continue       : 1;
        udp_flow_lkp_result         : 2;
        pad                         : 3;
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
        sl_result       : 2;
        qid             : 24;
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
        zero                        : 1;
        pad0                        : 3;
        udp_qid_tbl_idx             : 12;   // stg_id | udp_oflow_index[10:0]
        udp_flow_qid                : 8;
    }
}

// Phv header instantiation -
// start with intrinsic followed by p4_to_rxdma..., followed by rxdma metadata
@pragma dont_trim
metadata cap_phv_intr_global_t capri_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_p4_t capri_p4_intrinsic;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t capri_rxdma_intrinsic;

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
metadata qstate_txdma_fte_Q_t   scratch_qstate_txdma_fte_Q;

@pragma dont_trim
@pragma scratch_metadata
metadata udp_scratch_metadata_t udp_scratch;

@pragma dont_trim
metadata udp_flow_metadata_t    udp_flow_meta;


@pragma pa_align 512
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

