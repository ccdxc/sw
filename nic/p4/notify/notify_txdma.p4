

/*
 * NOTIFY
 */

header_type notify_qstate {
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
        host_pindex : 16;

        // sta
        host_color : 1;
        rsvd1 : 7;

        // cfg
        enable : 1;
        host_queue : 1;
        intr_enable : 1;
        rsvd2 : 5;

        ring_base : 64;
        ring_size : 16;

        host_ring_base : 64;
        host_ring_size : 16;
        host_intr_assert_index : 16;
    }
}

// 64 B
header_type notify_event_desc {
    fields {
        data : 504;
        color : 1;
        rsvd0 : 7;
    }
}

header_type notify_global_k {
    fields {
        dma_cur_index : 6;
        intr_enable : 1;    // generate an interrupt
    }
}

header_type notify_t0_s2s_k {
    fields {
        host_desc_addr : 64;
        intr_assert_index : 16;
        intr_assert_data : 32;  /* Byte aligned for DMA */
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata notify_qstate notify_qstate;

@pragma scratch_metadata
metadata notify_event_desc notify_event_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata notify_global_k notify_global;
@pragma scratch_metadata
metadata notify_global_k notify_global_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata notify_t0_s2s_k notify_t0_s2s;
@pragma scratch_metadata
metadata notify_t0_s2s_k notify_t0_s2s_scratch;

/*****************************************************************************
 * P-vector
 *****************************************************************************/

@pragma pa_align 512
@pragma dont_trim
metadata notify_event_desc notify_host_event_desc;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma2;
