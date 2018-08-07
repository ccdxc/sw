
#include "common.p4"

header_type adminq_global_k {
    fields {
        dma_cur_flit : 4;
        dma_cur_index : 2;
    }
}

header_type adminq_t0_s2s_k {
    fields {
        lif : 11;
        qtype : 3;
        qid : 24;
        adminq_qstate_addr : 34;
        nicmgr_qstate_addr : 34;
        host_queue : 1;
    }
}

header_type adminq_to_s2_k {
    fields {
        nicmgr_db_data : 64;    /* Byte aligned for DMA */
        intr_assert_data : 32;  /* Byte aligned for DMA */
    }
}

header_type adminq_to_s3_k {
    fields {
        adminq_ci : 16;
    }
}

/*****************************************************************************
 *  D-vector
 *****************************************************************************/

@pragma scratch_metadata
metadata adminq_cmd_desc adminq_cmd_desc;

/*****************************************************************************
 *  K-vector
 *****************************************************************************/

// Union with Common-TXDMA PHV headers

// Global PHV headers (Available in STAGES=ALL, MPUS=ALL)
@pragma pa_header_union ingress common_global
metadata adminq_global_k adminq_global;
@pragma scratch_metadata
metadata adminq_global_k adminq_global_scratch;

// Stage to Stage headers (Available in STAGES=ALL, MPUS=N)
@pragma pa_header_union ingress common_t0_s2s
metadata adminq_t0_s2s_k adminq_t0_s2s;
@pragma scratch_metadata
metadata adminq_t0_s2s_k adminq_t0_s2s_scratch;

// To Stage N PHV headers (Available in STAGE=N, MPUS=ALL)
@pragma pa_header_union ingress to_stage_2
metadata adminq_to_s2_k adminq_to_s2;
@pragma scratch_metadata
metadata adminq_to_s2_k adminq_to_s2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata adminq_to_s3_k adminq_to_s3;
@pragma scratch_metadata
metadata adminq_to_s3_k adminq_to_s3_scratch;

/*****************************************************************************
 * P-vector
 *****************************************************************************/

@pragma pa_align 512
@pragma dont_trim
metadata nicmgr_req_desc nicmgr_req_desc;

// DMA headers
@pragma pa_align 512
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma2;
