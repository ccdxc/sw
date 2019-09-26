

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
        num_sg_elems : 8;
        comp_index : 16;
        rss_hash : 32;
        csum : 16;
        vlan_tci : 16;
        len : 16;
        csum_calc : 1;
        vlan_strip : 1;
        csum_ip_bad : 1;
        csum_ip_ok : 1;
        csum_udp_bad : 1;
        csum_udp_ok : 1;
        csum_tcp_bad : 1;
        csum_tcp_ok : 1;
        color : 1;
        pkt_type : 7;
    }
}

/***
 * D-vector Headers
 ***/

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
        comp_index : 16;

        // sta
        color : 1;
        rsvd1 : 7;

        // cfg
        enable : 1;
        host_queue : 1;
        cpu_queue : 1;
        intr_enable : 1;
        debug : 1;
        rsvd2 : 3;

        ring_base : 64;
        ring_size : 8;
        cq_ring_base : 64;
        intr_assert_index : 16;
        sg_ring_base : 64;

        lg2_desc_sz : 4;
        lg2_cq_desc_sz : 4;
        lg2_sg_desc_sz : 4;
        sg_max_elems : 4;
    }
}

header_type eth_rx_desc_d {
    fields {
        opcode : 8;
        rsvd : 40;
        len : 16;
        addr : 64;
    }
}

#define HEADER_SG_ELEM(n) \
    addr##n : 64; \
    len##n : 16; \
    rsvd##n : 48;

header_type eth_sg_desc_d {
    fields {
        HEADER_SG_ELEM(0)
        HEADER_SG_ELEM(1)
        HEADER_SG_ELEM(2)
        HEADER_SG_ELEM(3)
    }
}

/***
 * K+I Headers
 ***/

header_type eth_rx_global_k {
    fields {
        dma_cur_index : 6;
        sg_desc_addr : 64;
        host_queue : 1;
        cpu_queue : 1;
        intr_enable : 1;
        lif : 11;
        stats : 32;
        drop : 1;
    }
}

header_type eth_rx_t0_s2s_k {
    fields {
        pkt_len : 16;
        cq_desc_addr : 64;
        intr_assert_index : 16;
        intr_assert_data : 32;  // Should be byte-aligned for PHV2MEM
    }
}

header_type eth_rx_t1_s2s_k {
    fields {
        l2_pkt_type : 2;
        pkt_type : 6;
        pkt_len : 16;
        rem_pkt_bytes : 14;
        rem_sg_elems : 4;
        sg_max_elems : 4;
    }
}

header_type eth_rx_to_s1_k {
    fields {
        qstate_addr : 64;
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata eth_rx_qstate_d eth_rx_qstate;

@pragma scratch_metadata
metadata eth_rx_desc_d eth_rx_desc;

@pragma scratch_metadata
metadata eth_sg_desc_d eth_sg_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-RXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_rx_global_k eth_rx_global;
@pragma scratch_metadata
metadata eth_rx_global_k eth_rx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_1
metadata eth_rx_to_s1_k eth_rx_to_s1;
@pragma scratch_metadata
metadata eth_rx_to_s1_k eth_rx_to_s1_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata eth_rx_t0_s2s_k eth_rx_t0_s2s;
@pragma scratch_metadata
metadata eth_rx_t0_s2s_k eth_rx_t0_s2s_scratch;

@pragma pa_header_union ingress common_t1_s2s
metadata eth_rx_t1_s2s_k eth_rx_t1_s2s;
@pragma scratch_metadata
metadata eth_rx_t1_s2s_k eth_rx_t1_s2s_scratch;

/*****************************************************************************
 *  P-vector
 *****************************************************************************/

// Part of the PHV after K
@pragma dont_trim
metadata eth_rx_cq_desc_p eth_rx_cq_desc;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_mem2pkt_t mem2pkt;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t phv2pkt;
@pragma dont_trim
metadata dma_cmd_phv2mem_t phv2mem;
@pragma dont_trim
metadata dma_cmd_pkt2mem_t pkt2mem;
@pragma dont_trim
metadata dma_cmd_mem2mem_t mem2mem;
@pragma dont_trim
@pragma pa_header_union ingress mem2pkt phv2pkt phv2mem pkt2mem mem2mem
metadata dma_cmd_generic_t dma;
