

/*
 * EDMA
 */

header_type edma_qstate {
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
        intr_enable : 1;
        rsvd2 : 6;

        ring_base : 64;
        ring_size : 16;
        cq_ring_base : 64;
        intr_assert_index : 16;
    }
}

// 32 B
header_type edma_cmd_desc {
    fields {
        opcode : 8;
        len : 16;
        src_lif : 16;
        src_addr : 64;
        dst_lif : 16;
        dst_addr : 64;
        rsvd0 : 72;
    }
}

// 16 B
header_type edma_comp_desc {
    fields {
        status : 8;
        rsvd : 8;
        comp_index : 16;
        data : 88;
        color : 1;
        rsvd2 : 7;
    }
}

header_type edma_global_k {
    fields {
        dma_cur_index : 6;
        intr_enable : 1;    // generate an interrupt
    }
}

header_type edma_t0_s2s_k {
    fields {
        cq_desc_addr : 64;
        intr_assert_index : 16;
        intr_assert_data : 32;  /* Byte aligned for DMA */
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata edma_qstate edma_qstate;

@pragma scratch_metadata
metadata edma_cmd_desc edma_cmd_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata edma_global_k edma_global;
@pragma scratch_metadata
metadata edma_global_k edma_global_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata edma_t0_s2s_k edma_t0_s2s;
@pragma scratch_metadata
metadata edma_t0_s2s_k edma_t0_s2s_scratch;

/*****************************************************************************
 * P-vector
 *****************************************************************************/

@pragma pa_align 512
@pragma dont_trim
metadata edma_comp_desc edma_comp_desc;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma2;
