

/***
 *  Header Type Declarations
 ***/

/***
 *  P-vector Headers
 ***/

header_type eth_rx_cq_desc_p {
    // RX Completion Descriptor
    fields {
        status : 8;
        rsvd : 8;
        comp_index : 16;
        rss_hash : 32;
        csum : 16;
        vlan_tci : 16;
        len_lo : 8;
        rsvd2 : 2;
        len_hi : 6;
        rsvd3 : 4;
        rss_type : 4;
        color : 1;
        vlan_strip : 1;
        csum_ip_bad : 1;
        csum_ip_ok : 1;
        csum_udp_bad : 1;
        csum_udp_ok : 1;
        csum_tcp_bad : 1;
        csum_tcp_ok : 1;
    }
}

/***
 * D-vector Headers
 ***/

// d for stage 0
header_type eth_rx_qstate_d {
    // Max 512 bits
    fields {
        pc : 8;
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

        enable : 8;
        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        rss_type : 16;
        intr_assert_addr : 32;
        color : 1;
    }
}

header_type eth_rx_desc_d {
    fields {
        addr_lo : 48;
        rsvd0 : 4;
        addr_hi : 4;
        rsvd1 : 8;
        len : 16;
        rsvd2 : 5;
        opcode : 3;
        rsvd3 : 8;
        rsvd4 : 32;
    }
}

/***
 * K+I Headers
 ***/

header_type eth_rx_global_k {
    fields {
        qstate_addr : 34;
    }
}

header_type eth_rx_t0_s2s_k {
    fields {
        packet_len : 16;
        cq_desc_addr : 52;
        __pad : 4;
        intr_assert_addr : 32;
        intr_assert_data : 32;  // Should be byte-aligned for PHV2MEM
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

/* NOTE: Defined in Common-P4+
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_classic_nic_header_t p4_to_p4plus;
@pragma scratch_metadata
metadata p4_to_p4plus_classic_nic_header_t p4_to_p4plus_scratch;
*/

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_rx_global_k eth_rx_global;
@pragma scratch_metadata
metadata eth_rx_global_k eth_rx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
/*
@pragma pa_header_union ingress to_stage_1
metadata eth_rx_to_stage_1_k eth_rx_to_s1;
@pragma scratch_metadata
metadata eth_rx_to_stage_1_k eth_rx_to_s1_scratch;

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

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata eth_rx_t0_s2s_k eth_rx_t0_s2s;
@pragma scratch_metadata
metadata eth_rx_t0_s2s_k eth_rx_t0_s2s_scratch;
/*
@pragma pa_header_union ingress common_t1_s2s
metadata eth_rx_t1_s2s_k eth_rx_t1_s2s;
@pragma scratch_metadata
metadata eth_rx_t1_s2s_k eth_rx_t1_s2s_scratch;

@pragma pa_header_union ingress common_t2_s2s
metadata eth_rx_t2_s2s_k eth_rx_t2_s2s;
@pragma scratch_metadata
metadata eth_rx_t2_s2s_k eth_rx_t2_s2s_scratch;

@pragma pa_header_union ingress common_t3_s2s
metadata eth_rx_t3_s2s_k eth_rx_t3_s2s;
@pragma scratch_metadata
metadata eth_rx_t3_s2s_k eth_rx_t3_s2s_scratch;
*/

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
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
