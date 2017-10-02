

/***
 *  Header Type Declarations
 ***/

/***
 *  P-vector Headers
 ***/

header_type eth_tx_cq_desc_p {
    fields {
        completion_index : 16;
        rsvd0 : 8;
        queue_id : 8;
        rsvd1 : 80;
        err_code : 8;
        rsvd2 : 7;
        color : 1;
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
        pid : 16;                // 8 B

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
        c_index7 : 16;          // 40 B

        enable : 8;
        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;    // 59 B
    }
}

header_type eth_tx_desc_d {
    fields {
        addr : 64;
        len : 16;
        vlan_tag : 16;
        mss : 14;
        encap : 2;
        hdr_len : 10;
        offload : 2;
        eop : 1;
        cq_entry : 1;
        vlan_insert : 1;
        rsvd0 : 1;
    }
}

/***
 * K+I Headers
 ***/

header_type eth_tx_global_k {
    // global k (max 128)
    fields {
        lif     : 11;       // LIF number
        qtype   : 3;        // Queue type
        qid     : 24;       // Queue ID
    }
}

header_type eth_tx_to_stage_1_k {
    // to_stage k (max 128 bits)
    fields {
        cq_desc_addr : 64;
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

// App Header
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t eth_tx_app_header;

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata eth_tx_global_k eth_tx_global;
@pragma scratch_metadata
metadata eth_tx_global_k eth_tx_global_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_1
metadata eth_tx_to_stage_1_k eth_tx_to_s1;
@pragma scratch_metadata
metadata eth_tx_to_stage_1_k eth_tx_to_s1_scratch;

/*
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

// Stage N to N+1 PHV headers (Available in STAGE=N,N+1 MPUS=ALL)


// Part of the PHV after Common Area
@pragma dont_trim
metadata eth_tx_cq_desc_p eth_tx_cq_desc;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t txdma_intr;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
