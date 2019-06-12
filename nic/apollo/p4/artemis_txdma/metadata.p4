#include "../../../p4/common-p4+/capri_dma_cmd.p4"
#include "../../../p4/common-p4+/capri_doorbell.p4"

header_type txdma_predicate_metadata_t {
    fields {
        p4plus_app_id       : 4;
        mapping_en          : 1;
        pass_two            : 1;
        pass_skip           : 1;
        cps_path_en         : 1;
    }
}

header_type txdma_control_metadata_t {
    fields {
        pktdesc_addr        : 40;
        rxdma_cindex_addr   : 40;
        rfc_table_addr      : 40;
        lpm1_base_addr      : 40;
        lpm1_next_addr      : 40;
        lpm1_key            : 128;

        payload_addr        : 40;
        cindex              : 16;

        mapping_ohash_lkp   : 1;
        pad0                : 1;
        rfc_p1_classid      : 10;
        rule_priority       : 10;
        svc_id              : 10;

        vpc_id              : 8;
        mapping_ohash       : 32;

        pad1                : 4;
        rfc_index           : 20;
    }
}

header_type scratch_metadata_t {
    fields {
        field1          :  1;
        field2          :  2;
        field7          :  7;
        field8          :  8;
        field10         : 10;
        field16         : 16;
        field20         : 20;
        field32         : 32;
        field40         : 40;
        field64         : 64;
        field128        : 128;
        field512        : 512;
        payload_addr    : 40;
        payload_len     : 14;
        qid             : 24;
        lif             : 11;
        // TMP: Fileds added temporarily
        pad0                : 4;
        flag                : 1;
        hint_valid          : 1;
        mapping_hash        : 11;
        mapping_hint        : 18;
    }
}

// PHV instantiation
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata txdma_predicate_metadata_t txdma_predicate;

@pragma dont_trim
@pragma pa_header_union ingress to_stage_1
metadata doorbell_data_t    doorbell_data;

// Scratch metadata
@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma scratch_metadata
metadata qstate_info_t          scratch_qstate_info;

@pragma dont_trim
metadata txdma_control_metadata_t txdma_control;

@pragma dont_trim
metadata session_info_hint_t session_info_hint;

@pragma pa_align 512
@pragma dont_trim
metadata artemis_rx_to_tx_header_t  rx_to_tx_hdr;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;       // dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2pkt_t payload_dma;         // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t rxdma_ci_update;     // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_ci_update;  // dma cmd 4

