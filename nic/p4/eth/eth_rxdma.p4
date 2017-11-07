

/***
 *  Header Type Declarations
 ***/

/***
 *  P-vector Headers
 ***/

header_type eth_rx_cq_desc_p {
    // RX Completion Descriptor
    fields {
        completion_index : 16;
        queue_id : 8;
        err_code : 8;
        // From - P4
        flags : 16;
        vlan_tag : 16;
        checksum : 32;
        bytes_written : 16;
        // From - RSS table
        rss_type : 8;
        rss_hash : 32;
        // END
        rsvd0 : 103;
        color : 1;
    }
}

/***
 * D-vector Headers
 ***/

// d for stage 0
header_type eth_rx_qstate_d {
    fields {
        //pc : 8;
        rsvd : 8;
        cosA : 4;
        cosB : 4;
        cos_sel : 8;
        eval_last : 8;
        host : 4;
        total : 4;
        pid : 16;

        p_index0 : 16;
        c_index0 : 16;
        p_index1 : 16;
        c_index1 : 16;
        p_index2 : 16;
        c_index2 : 16;
        p_index3 : 16;
        c_index3 : 16;
        p_index4 : 16;
        c_index4 : 16;
        p_index5 : 16;
        c_index5 : 16;
        p_index6 : 16;
        c_index6 : 16;
        p_index7 : 16;
        c_index7 : 16;

        enable : 8;
        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        color : 1;
    }
}

header_type eth_rx_desc_d {
    fields {
        addr : 64;
        len : 16;
        rsvd0: 48;
    }
}

/***
 * K+I Headers
 ***/

header_type eth_rx_global_k {
    // global k (max 128 bits)
    fields {
        packet_len : 16;
    }
}

header_type eth_rx_to_stage_1_k {
    // to_stage k (max 128 bits)
    fields {
        cq_desc_addr : 64;
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata eth_rx_qstate_d eth_rx_qstate;

@pragma scratch_metadata
metadata eth_rx_desc_d eth_rx_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-RXDMA PHV headers

// App header (Available in STAGE=0)
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_classic_nic_header_t p4_to_p4plus;
@pragma scratch_metadata
metadata p4_to_p4plus_classic_nic_header_t p4_to_p4plus_scratch;

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_rx_global_k eth_rx_global;
@pragma scratch_metadata
metadata eth_rx_global_k eth_rx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_1
metadata eth_rx_to_stage_1_k eth_rx_to_s1;
@pragma scratch_metadata
metadata eth_rx_to_stage_1_k eth_rx_to_s1_scratch;

/*
@pragma pa_header_union ingress to_stage_2
metadata eth_rx_to_stage_2_k eth_rx_to_s2;
@pragma scratch_metadata
metadata eth_rx_to_stage_2_k eth_rx_to_s2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata eth_rx_to_stage_3_k eth_rx_to_s3;
@pragma scratch_metadata
metadata eth_rx_to_stage_3_k eth_rx_to_s3_scratch;

@pragma pa_header_union ingress to_stage_4
metadata eth_rx_to_stage_4_k eth_rx_to_s4;
@pragma scratch_metadata
metadata eth_rx_to_stage_4_k eth_rx_to_s4_scratch;

@pragma pa_header_union ingress to_stage_5
metadata eth_rx_to_stage_5_k eth_rx_to_s5;
@pragma scratch_metadata
metadata eth_rx_to_stage_5_k eth_rx_to_s5_scratch;

@pragma pa_header_union ingress to_stage_6
metadata eth_rx_to_stage_6_k eth_rx_to_s6;
@pragma scratch_metadata
metadata eth_rx_to_stage_6_k eth_rx_to_s6_scratch;

@pragma pa_header_union ingress to_stage_7
metadata eth_rx_to_stage_7_k eth_rx_to_s7;
@pragma scratch_metadata
metadata eth_rx_to_stage_7_k eth_rx_to_s7_scratch;
*/

// Stage N to N+1 PHV headers (Available in STAGE=N,N+1 MPUS=ALL)

/*****************************************************************************
 *  P-vector
 *****************************************************************************/

// Part of the PHV after K
@pragma dont_trim
metadata eth_rx_cq_desc_p eth_rx_cq_desc;

@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;
@pragma dont_trim       // HACK: Workaround for dma command padding issue
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim       // HACK: Workaround for dma command padding issue
metadata dma_cmd_phv2mem_t dma_cmd3;

