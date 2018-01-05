/*
 *  Virtio TX (TXDMA) support
 */

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action           read_virtq_tx_qstate
#define tx_table_s1_t0_action           read_tx_virtq_avail
#define tx_table_s2_t0_action           check_tx_reqs
#define tx_table_s3_t0_action           read_tx_head_desc_idx
#define tx_table_s4_t0_action           read_tx_head_desc
#define tx_table_s5_t0_action           read_tx_payload_desc
#if 0
#define tx_table_s5_t1_action           read_tx_virtio_net_hdr
#endif

#include "../common-p4+/common_txdma.p4"
#include "virtio_common.h"



/* To Stage 1 */
header_type to_stage_1_phv_t {
    fields {
        qstate_addr             : 36;
    }
}

#define TO_S1_SCRATCH   to_s1_scratch
#define TO_S1           to_s1

#define GENERATE_TO_S1_K                                                    \
    modify_field(TO_S1_SCRATCH.qstate_addr, TO_S1.qstate_addr);


@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;

@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;

/* To Stage 2 */
header_type to_stage_2_phv_t {
    fields {
        tx_virtq_avail_addr     : 64;
        qid                     : 24;
        tx_virtq_avail_idx      : 16;
        lif                     : 11;
    }
}

#define TO_S2_SCRATCH   to_s2_scratch
#define TO_S2           to_s2

#define GENERATE_TO_S2_K                                                    \
    modify_field(TO_S2_SCRATCH.tx_virtq_avail_addr, TO_S2.tx_virtq_avail_addr); \
    modify_field(TO_S2_SCRATCH.qid, TO_S2.qid);                                 \
    modify_field(TO_S2_SCRATCH.tx_virtq_avail_idx, TO_S2.tx_virtq_avail_idx);   \
    modify_field(TO_S2_SCRATCH.lif, TO_S2.lif);


@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;

@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;


/* to Stage 3 */
header_type to_stage_3_phv_t {
    fields {
        tx_virtq_desc_addr      : 64;
    }
}
#define TO_S3_SCRATCH   to_s3_scratch
#define TO_S3           to_s3

#define GENERATE_TO_S3_K                                                    \
    modify_field(TO_S3_SCRATCH.tx_virtq_desc_addr, TO_S3.tx_virtq_desc_addr);

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;


/* to Stage 4 */
header_type to_stage_4_phv_t {
    fields {
        tx_virtq_desc_addr      : 64;
        tx_virtq_used_addr      : 64;
    }
}
#define TO_S4_SCRATCH   to_s4_scratch
#define TO_S4           to_s4

#define GENERATE_TO_S4_K                                                    \
    modify_field(TO_S4_SCRATCH.tx_virtq_desc_addr, TO_S4.tx_virtq_desc_addr);   \
    modify_field(TO_S4_SCRATCH.tx_virtq_used_addr, TO_S4.tx_virtq_used_addr);

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

/* to Stage 5 */
header_type to_stage_5_phv_t {
    fields {
        tx_virtq_used_addr      : 64;
    }
}
#define TO_S5_SCRATCH   to_s5_scratch
#define TO_S5           to_s5

#define GENERATE_TO_S5_K                                                    \
    modify_field(TO_S5_SCRATCH.tx_virtq_used_addr, TO_S5.tx_virtq_used_addr);   \

@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;

@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

/* Global K definition */
header_type virtio_tx_global_k {
    fields {
        features                : 64;
        head_desc_idx           : 16;
        /* may need to be 17 bits with support for TSO + virtio_net_hdr ? */
        bytes_consumed          : 16;
        tx_virtq_used_pi        : 16;
    }
}
#define VIRTIO_GLOBAL_PHV_SCRATCH   virtio_txvirtio_tx_global_phv_scratch
#define VIRTIO_GLOBAL_PHV           virtio_tx_global_phv
#define GENERATE_GLOBAL_K                                                   \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.features, VIRTIO_GLOBAL_PHV.features); \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.head_desc_idx, VIRTIO_GLOBAL_PHV.head_desc_idx);     \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.bytes_consumed, VIRTIO_GLOBAL_PHV.bytes_consumed);   \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.tx_virtq_used_pi, VIRTIO_GLOBAL_PHV.tx_virtq_used_pi);


@pragma scratch_metadata
metadata virtio_tx_global_k VIRTIO_GLOBAL_PHV_SCRATCH;

@pragma pa_header_union ingress common_global
metadata virtio_tx_global_k VIRTIO_GLOBAL_PHV;

/* Stage-to-stage, Table 0 */
header_type virtio_s2s_t0_t {
    fields {
        tx_queue_size_mask      : 16;
        tx_intr_assert_addr     : 32;
        tx_intr_assert_data     : 32;
        no_interrupt            : 1;
        pend_work_scheduled     : 1;
    }
}
#define VIRTIO_S2S_T0_PHV_SCRATCH   virtio_s2s_t0_phv_scratch
#define VIRTIO_S2S_T0_PHV           virtio_s2s_t0_phv
#define GENERATE_VIRTIO_S2S_T0_K                                            \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.tx_queue_size_mask, VIRTIO_S2S_T0_PHV.tx_queue_size_mask);     \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.tx_intr_assert_addr, VIRTIO_S2S_T0_PHV.tx_intr_assert_addr);   \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.tx_intr_assert_data, VIRTIO_S2S_T0_PHV.tx_intr_assert_data);   \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.no_interrupt, VIRTIO_S2S_T0_PHV.no_interrupt);       \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.pend_work_scheduled, VIRTIO_S2S_T0_PHV.pend_work_scheduled);

@pragma scratch_metadata
metadata virtio_s2s_t0_t VIRTIO_S2S_T0_PHV_SCRATCH;

@pragma pa_header_union ingress common_t0_s2s
metadata virtio_s2s_t0_t VIRTIO_S2S_T0_PHV;

/*---------------------------------------------------------------------------*/
/*                                  PHV Scratch                              */
/*---------------------------------------------------------------------------*/
// App Header
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t eth_tx_app_hdr;

@pragma dont_trim
metadata virtq_used_elem vq_used_elem; 

header_type pad_to_dma_cmds_t {
    fields {
        pad                     : 176;
    }
}

@pragma dont_trim
metadata virtq_used_idx vq_used_idx;
@pragma dont_trim
metadata pad_to_dma_cmds_t pad_to_dma_cmds;

/*---------------------------------------------------------------------------*/
/*                                  DMA Commands                             */
/*---------------------------------------------------------------------------*/

@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_cmd;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t packet_cmd;
@pragma dont_trim
metadata dma_cmd_phv2mem_t vq_used_ring_elem_cmd;
@pragma dont_trim
metadata dma_cmd_phv2mem_t vq_used_idx_cmd;
@pragma dont_trim
metadata dma_cmd_phv2mem_t interrupt_cmd;


/*---------------------------------------------------------------------------*/
/*                                  Actions                                  */
/*---------------------------------------------------------------------------*/

/* Stage 0, Table 0 */

@pragma scratch_metadata
metadata virtio_tx_qstate VIRTIO_TX_QSTATE_D_SCRATCH;

action read_virtq_tx_qstate(VIRTIO_TX_QSTATE_D_PARAMS)
{
    GENERATE_VIRTIO_TX_QSTATE_D
}


/* Stage 1, Table 0 */
@pragma scratch_metadata
metadata virtq_avail VIRTQ_AVAIL_D_SCRATCH;

action read_tx_virtq_avail(VIRTQ_AVAIL_D_PARAMS)
{
    GENERATE_GLOBAL_K

    GENERATE_TO_S1_K

    GENERATE_VIRTQ_AVAIL_D
}

/* Stage 2, Table 0 */
action check_tx_reqs(VIRTIO_TX_QSTATE_NON_STG0_D_PARAMS)
{
    GENERATE_GLOBAL_K

    GENERATE_TO_S2_K

    GENERATE_VIRTIO_TX_QSTATE_NON_STG0_D
}

/* Stage 3, Table 0 */
@pragma scratch_metadata
metadata virtq_desc_idx VIRTQ_DESC_IDX_D_SCRATCH;

action read_tx_head_desc_idx(VIRTQ_DESC_IDX_D_PARAMS)
{
    GENERATE_GLOBAL_K

    GENERATE_TO_S3_K

    GENERATE_VIRTQ_DESC_IDX_D
}


/* Stage 4, Table 0 */
@pragma scratch_metadata
metadata virtq_desc VIRTQ_DESC_D_SCRATCH;

action read_tx_head_desc(VIRTQ_DESC_D_PARAMS)
{
    GENERATE_GLOBAL_K
    GENERATE_VIRTIO_S2S_T0_K

    GENERATE_TO_S4_K

    GENERATE_VIRTQ_DESC_D
}


/* Stage 5, Table 0 */

action read_tx_payload_desc(VIRTQ_DESC_D_PARAMS)
{
    GENERATE_GLOBAL_K
    GENERATE_VIRTIO_S2S_T0_K

    GENERATE_TO_S5_K

    GENERATE_VIRTQ_DESC_D
}


#if 0
/* Stage 5, Table 1 */
@pragma scratch_metadata
metadata virtio_net_hdr_no_mrg_rxbuf VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH;

action read_tx_virtio_net_hdr(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_PARAMS)
{
    GENERATE_GLOBAL_K

    GENERATE_VIRTIO_NET_HDR_NO_MRG_RXBUF_D
}
#endif
