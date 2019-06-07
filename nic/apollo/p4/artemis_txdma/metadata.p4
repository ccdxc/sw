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

        remote_vnic_mapping_tx_ohash_lkp : 1; //TODO-KSM: Replace with P4 to P4+ header
        sacl_result         : 1;
        rfc_p1_classid      : 10;
        rule_priority       : 10;
        svc_id              : 10;

        vpc_id              : 8;
        nexthop_group_index : 16;
        remote_vnic_mapping_tx_ohash : 32; //TODO-KSM: Replace with P4 to P4+ header

        pad0                : 4;
        rfc_index           : 20;
        ipv6_tx_da          : 128;
    }
}

header_type scratch_metadata_t {
    fields {
        field1          :  1;
        field2          :  2;
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
        nexthop_group_index : 10;
        remote_vnic_mapping_tx_done : 1;
        dst_slot_id_valid   : 1;
        pad0                : 4;
        dst_slot_id         : 24;
        flag                : 1;
        hint_valid          : 1;
        vnic_mapping_hash   : 11;
        remote_vnic_mapping_tx_ohash_lkp    : 1;
        vnic_mapping_hint   : 18;
        remote_vnic_mapping_tx_ohash : 32;
        vpc_peer_base       : 40;
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

