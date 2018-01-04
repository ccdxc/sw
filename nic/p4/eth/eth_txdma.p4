

/***
 *  Header Type Declarations
 ***/

/***
 *  P-vector Headers
 ***/

header_type eth_tx_cq_desc_p {
    fields {
        status : 8;
        rsvd : 8;
        comp_index : 16;
        rsvd2 : 64;
        rsvd3 : 24;
        color : 1;
        rsvd4 : 7;
    }
}

/***
 *  D-vector Headers
 ***/

header_type eth_tx_qstate_d {
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

        enable : 8;
        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        intr_assert_addr : 32;
        color : 1;
    }
}

header_type eth_tx_desc_d {
    fields {
        addr : 64;
        //addr_lo : 32;
        //addr_hi : 20;
        //rsvd : 4;
        //num_sg_elems : 5;
        //opcode : 3;
        len : 16;
        vlan_tci : 16;
        hdr_len : 10;
        rsvd2 : 3;
        V : 1;
        C : 1;
        O : 1;
        mss_or_csumoff : 14;
        rsvd3_or_rsvd4 : 2;
    }
}

/***
 * K+I Headers
 ***/

header_type eth_tx_global_k {
    fields {
        lif     : 11;
        qtype   : 3;
        qid     : 24;
    }
}

header_type eth_tx_t0_s2s_k {
    fields {
        cq_desc_addr : 64;
        intr_assert_addr : 32;
        intr_assert_data : 32;
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata eth_tx_qstate_d eth_tx_qstate;

@pragma scratch_metadata
metadata eth_tx_desc_d eth_tx_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_tx_global_k eth_tx_global;
@pragma scratch_metadata
metadata eth_tx_global_k eth_tx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
/*
@pragma pa_header_union ingress to_stage_1
metadata eth_tx_to_stage_1_k eth_tx_to_s1;
@pragma scratch_metadata
metadata eth_tx_to_stage_1_k eth_tx_to_s1_scratch;

@pragma pa_header_union ingress to_stage_2
metadata eth_tx_to_stage_2_p eth_tx_to_s2;
@pragma scratch_metadata
metadata eth_tx_to_stage_2_p eth_tx_to_s2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata eth_tx_to_stage_3_p eth_tx_to_s3;
@pragma scratch_metadata
metadata eth_tx_to_stage_3_p eth_tx_to_s3_scratch;

@pragma pa_header_union ingress to_stage_4
metadata eth_tx_to_stage_4_p eth_tx_to_s4;
@pragma scratch_metadata
metadata eth_tx_to_stage_4_p eth_tx_to_s4_scratch;

@pragma pa_header_union ingress to_stage_5
metadata eth_tx_to_stage_5_p eth_tx_to_s5;
@pragma scratch_metadata
metadata eth_tx_to_stage_5_p eth_tx_to_s5_scratch;

@pragma pa_header_union ingress to_stage_6
metadata eth_tx_to_stage_6_p eth_tx_to_s6;
@pragma scratch_metadata
metadata eth_tx_to_stage_6_p eth_tx_to_s6_scratch;

@pragma pa_header_union ingress to_stage_7
metadata eth_tx_to_stage_7_p eth_tx_to_s7;
@pragma scratch_metadata
metadata eth_tx_to_stage_7_p eth_tx_to_s7_scratch;
*/

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata eth_tx_t0_s2s_k eth_tx_t0_s2s;
@pragma scratch_metadata
metadata eth_tx_t0_s2s_k eth_tx_t0_s2s_scratch;
/*
@pragma pa_header_union ingress common_t1_s2s
metadata eth_tx_t1_s2s_k eth_tx_t1_s2s;
@pragma scratch_metadata
metadata eth_tx_t1_s2s_k eth_tx_t1_s2s_scratch;

@pragma pa_header_union ingress common_t2_s2s
metadata eth_tx_t2_s2s_k eth_tx_t2_s2s;
@pragma scratch_metadata
metadata eth_tx_t2_s2s_k eth_tx_t2_s2s_scratch;

@pragma pa_header_union ingress common_t3_s2s
metadata eth_tx_t3_s2s_k eth_tx_t3_s2s;
@pragma scratch_metadata
metadata eth_tx_t3_s2s_k eth_tx_t3_s2s_scratch;
*/

/*****************************************************************************
 * P-vetor
 *****************************************************************************/

// App Header
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t eth_tx_app_hdr;

// Part of the PHV after Common Area
@pragma dont_trim
metadata eth_tx_cq_desc_p eth_tx_cq_desc;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
