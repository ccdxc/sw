header_type scratch_metadata_t {
    fields {
        field8          :   8;
        field10         :  10;
        field16         :  16;
        field32         :  32;
        field40         :  40;
        field64         :  64;
        field128        : 128;
        qid             :  24;
        dma_size        :  16;
        flag            :   1;
        field7          :   7;
        remote_ip       : 128;
        pad1            :   2;
        
        timestamp        : 48;
        field4          :   4;
        field12         :  12;
        field20         :  20;
        field23         :  23;
        pad             :   9;
    }
}

header_type flow_key_t {
    fields {
        pad             : 12;
        flow_ktype      : 4;
        flow_src        : 128;
        flow_dst        : 128;
        flow_proto      : 8;
        flow_dport      : 16;
        flow_sport      : 16;
    }
}

header_type lpm_metadata_t {
    fields {
        lpm1_key        : 128;
        lpm1_base_addr  : 40;
        lpm1_next_addr  : 40;

        lpm2_key        : 128;
        lpm2_base_addr  : 40;
        lpm2_next_addr  : 40;

        meter_base_addr : 40;
    }
}

header_type rxdma_control_metadata_t {
    fields {
        xlate_idx       : 16;
    }
}

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma dont_trim
@pragma scratch_metadata
metadata qstate_info_t          scratch_qstate_info;

@pragma scratch_metadata
metadata flow_key_t             scratch_flow_key;

@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata artemis_p4_to_rxdma_header_t p4_to_rxdma;
@pragma dont_trim
@pragma pa_header_union ingress ext_app_header
metadata artemis_p4_to_rxdma_header2_t p4_to_rxdma2;
@pragma dont_trim
@pragma pa_header_union ingress to_stage_0
metadata artemis_p4_to_rxdma_header3_t p4_to_rxdma3;

@pragma dont_trim
metadata doorbell_addr_t        doorbell_addr;
@pragma dont_trim
@pragma pa_header_union ingress to_stage_1
metadata doorbell_data_t        doorbell_data;

@pragma dont_trim
metadata lpm_metadata_t         lpm_metadata;

@pragma dont_trim
metadata artemis_rx_to_tx_header_t rx_to_tx_hdr;

@pragma dont_trim
@pragma pa_header_union ingress rx_to_tx_hdr
metadata artemis_rx_to_tx_header4_t rx_to_tx_hdr4;

// DMA commands
@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2mem_t      pktdesc_phv2mem;

@pragma dont_trim
metadata dma_cmd_phv2mem_t      pktdesc2_part1_phv2mem;

@pragma dont_trim
metadata dma_cmd_phv2mem_t      pktdesc2_part2_phv2mem;

@pragma dont_trim
metadata dma_cmd_pkt2mem_t      pktbuf_pkt2mem;

@pragma dont_trim
metadata dma_cmd_phv2mem_t      doorbell_phv2mem;

@pragma dont_trim
metadata rxdma_control_metadata_t   rxdma_control;
