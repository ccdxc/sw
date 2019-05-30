#include "../../../p4/common-p4+/capri_dma_cmd.p4"
#include "../../../p4/common-p4+/capri_doorbell.p4"

header_type txdma_control_metadata_t {
    fields {
        pad0                : 6;
        control_addr        : 34;
        pad1                : 6;
        rxdma_cindex_addr   : 34;
        pad2                : 6;
        rfc_table_addr      : 34;
        pad3                : 6;
        lpm1_base_addr      : 34;
        pad4                : 6;
        lpm1_next_addr      : 34;
        lpm1_key            : 128;

        payload_addr        : 40;
        cindex              : 16;

        pad5                : 6;
        rule_priority       : 10;
        p1_class_id         : 10;
        p2_class_id         : 10;
        p3_class_id         : 20;
        sacl_result         : 1;
        remote_vnic_mapping_tx_ohash_lkp : 1; //TODO-KSM: Replace with P4 to P4+ header
        pad6                :  4;
        vpc_id              : 10;   //TODO-AJEER: Change this to 8 bits
                                    //TODO-ART: Fill original vnic id in RxDMA
        nexthop_group_index : 16;

        // TODO-JOHN/MURTY: Find a way to pass the keys from RxDMA to TxDMA
        remote_ip           : 128;  //TODO-KSM: Fill this in RxDMA
        remote_vnic_mapping_tx_ohash : 32; //TODO-KSM: Replace with P4 to P4+ header
        pad7                :  6;
        svc_id              : 10;
    }
}

header_type scratch_metadata_t {
    fields {
        field1          :  1;
        field2          :  2;
        field10         : 10;
        field16         : 16;
        field32         : 32;
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
        ipv6_tx_da          : 128;       
        vpc_peer_base       : 40;
    }
}

// PHV instantiation
@pragma dont_trim
metadata txdma_control_metadata_t txdma_control;
@pragma dont_trim
metadata txdma_to_p4e_header_t txdma_to_p4e;

@pragma dont_trim
metadata doorbell_data_t    doorbell_data;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;       // dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2pkt_t payload_dma;         // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t rxdma_ci_update;     // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_ci_update;  // dma cmd 4

// Scratch metadata
@pragma dont_trim
@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_info_t          scratch_qstate_info;
