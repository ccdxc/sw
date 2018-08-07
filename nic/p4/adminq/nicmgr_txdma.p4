
#include "common.p4"

header_type nicmgr_global_k {
    fields {
        dma_cur_flit : 4;
        dma_cur_index : 2;
    }
}

header_type nicmgr_t0_s2s_k {
    fields {
        lif : 11;
        qtype : 3;
        qid : 24;
        comp_index : 16;
        nicmgr_qstate_addr : 34;
        host_queue : 1;
    }
}

header_type nicmgr_to_s2_k {
    fields {
        nicmgr_db_data : 64;      /* Byte aligned for DMA */
        intr_assert_data : 32;      /* Byte aligned for DMA */
    }
}

header_type nicmgr_to_s3_k {
    fields {
        nicmgr_ci : 16;
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/

@pragma scratch_metadata
metadata nicmgr_resp_desc nicmgr_resp_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata nicmgr_global_k nicmgr_global;
@pragma scratch_metadata
metadata nicmgr_global_k nicmgr_global_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata nicmgr_t0_s2s_k nicmgr_t0_s2s;
@pragma scratch_metadata
metadata nicmgr_t0_s2s_k nicmgr_t0_s2s_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_2
metadata nicmgr_to_s2_k nicmgr_to_s2;
@pragma scratch_metadata
metadata nicmgr_to_s2_k nicmgr_to_s2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata nicmgr_to_s3_k nicmgr_to_s3;
@pragma scratch_metadata
metadata nicmgr_to_s3_k nicmgr_to_s3_scratch;

/*****************************************************************************
 * P-vector
 *****************************************************************************/

@pragma pa_align 128
@pragma dont_trim
metadata adminq_comp_desc adminq_comp_desc;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma2;
