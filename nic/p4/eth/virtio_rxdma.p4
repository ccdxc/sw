/*
 *  Virtio RX (RXDMA) support
 */

#include "../common-p4+/common_rxdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy  read_virtq_rx_qstate
#define rx_table_s1_t0_action           read_rx_virtq_avail
#define rx_table_s2_t0_action           check_rx_reqs
#define rx_table_s3_t0_action           read_rx_head_desc_idx
#define rx_table_s4_t0_action           read_rx_head_desc
#define rx_table_s5_t0_action           read_rx_payload_desc

#include "../common-p4+/common_rxdma.p4"
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
        rx_virtq_avail_addr     : 64;
        qid                     : 24;
        rx_virtq_avail_idx      : 16;
        lif                     : 11;
    }
}

#define TO_S2_SCRATCH   to_s2_scratch
#define TO_S2           to_s2

#define GENERATE_TO_S2_K                                                    \
    modify_field(TO_S2_SCRATCH.rx_virtq_avail_addr, TO_S2.rx_virtq_avail_addr); \
    modify_field(TO_S2_SCRATCH.qid, TO_S2.qid);                                 \
    modify_field(TO_S2_SCRATCH.rx_virtq_avail_idx, TO_S2.rx_virtq_avail_idx);   \
    modify_field(TO_S2_SCRATCH.lif, TO_S2.lif);


@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;

@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;

/* to Stage 3 */
header_type to_stage_3_phv_t {
    fields {
        rx_virtq_desc_addr      : 64;
    }
}
#define TO_S3_SCRATCH   to_s3_scratch
#define TO_S3           to_s3

#define GENERATE_TO_S3_K                                                    \
    modify_field(TO_S3_SCRATCH.rx_virtq_desc_addr, TO_S3.rx_virtq_desc_addr);

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;


/* to Stage 4 */
header_type to_stage_4_phv_t {
    fields {
        rx_virtq_desc_addr      : 64;
        rx_virtq_used_addr      : 64;
    }
}
#define TO_S4_SCRATCH   to_s4_scratch
#define TO_S4           to_s4

#define GENERATE_TO_S4_K                                                    \
    modify_field(TO_S4_SCRATCH.rx_virtq_desc_addr, TO_S4.rx_virtq_desc_addr);   \
    modify_field(TO_S4_SCRATCH.rx_virtq_used_addr, TO_S4.rx_virtq_used_addr);

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

/* to Stage 5 */
header_type to_stage_5_phv_t {
    fields {
        rx_virtq_desc_addr      : 64;
        rx_virtq_used_addr      : 64;
    }
}
#define TO_S5_SCRATCH   to_s5_scratch
#define TO_S5           to_s5

#define GENERATE_TO_S5_K                                                    \
    modify_field(TO_S5_SCRATCH.rx_virtq_desc_addr, TO_S5.rx_virtq_desc_addr);   \
    modify_field(TO_S5_SCRATCH.rx_virtq_used_addr, TO_S5.rx_virtq_used_addr);

@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;

@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

/* Global K definition */
header_type virtio_rx_global_k {
    fields {
        features                : 64;
        head_desc_idx           : 16;
        /* may need to be 17 bits with support for TSO + virtio_net_hdr ? */
        rx_virtq_avail_ci       : 16;
        rx_virtq_used_pi        : 16;
    }
}
#define VIRTIO_GLOBAL_PHV_SCRATCH   virtio_rxvirtio_rx_global_phv_scratch
#define VIRTIO_GLOBAL_PHV           virtio_rx_global_phv
#define GENERATE_GLOBAL_K                                                   \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.features, VIRTIO_GLOBAL_PHV.features); \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.head_desc_idx, VIRTIO_GLOBAL_PHV.head_desc_idx);     \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.rx_virtq_avail_ci, VIRTIO_GLOBAL_PHV.rx_virtq_avail_ci); \
    modify_field(VIRTIO_GLOBAL_PHV_SCRATCH.rx_virtq_used_pi, VIRTIO_GLOBAL_PHV.rx_virtq_used_pi);


@pragma scratch_metadata
metadata virtio_rx_global_k VIRTIO_GLOBAL_PHV_SCRATCH;

@pragma pa_header_union ingress common_global
metadata virtio_rx_global_k VIRTIO_GLOBAL_PHV;

/* Stage-to-stage, Table 0 */
header_type virtio_s2s_t0_t {
    fields {
        rx_queue_size_mask      : 16;
        pkt_len                 : 16;
        rx_intr_assert_addr     : 32;
        rx_intr_assert_data     : 32;
        no_interrupt            : 1;
    }
}
#define VIRTIO_S2S_T0_PHV_SCRATCH   virtio_s2s_t0_phv_scratch
#define VIRTIO_S2S_T0_PHV           virtio_s2s_t0_phv
#define GENERATE_VIRTIO_S2S_T0_K                                                                        \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.rx_queue_size_mask, VIRTIO_S2S_T0_PHV.rx_queue_size_mask);   \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.pkt_len, VIRTIO_S2S_T0_PHV.pkt_len);                         \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.rx_intr_assert_addr, VIRTIO_S2S_T0_PHV.rx_intr_assert_addr); \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.rx_intr_assert_data, VIRTIO_S2S_T0_PHV.rx_intr_assert_data); \
    modify_field(VIRTIO_S2S_T0_PHV_SCRATCH.no_interrupt, VIRTIO_S2S_T0_PHV.no_interrupt);

@pragma scratch_metadata
metadata virtio_s2s_t0_t VIRTIO_S2S_T0_PHV_SCRATCH;

@pragma pa_header_union ingress common_t0_s2s
metadata virtio_s2s_t0_t VIRTIO_S2S_T0_PHV;

/*---------------------------------------------------------------------------*/
/*                                  PHV Scratch                              */
/*---------------------------------------------------------------------------*/

@pragma dont_trim
metadata virtio_net_hdr_no_mrg_rxbuf    vnet_hdr_no_mrg_rxbuf;

@pragma dont_trim
metadata virtq_used_elem vq_used_elem; 

@pragma dont_trim
metadata virtq_used_idx vq_used_idx;

header_type pad_to_dma_cmds_t {
    fields {
        pad                 : 96;
    }
}

@pragma dont_trim
metadata pad_to_dma_cmds_t dma_cmd_pad;

/*---------------------------------------------------------------------------*/
/*                                  DMA Commands                             */
/*---------------------------------------------------------------------------*/

@pragma dont_trim
metadata dma_cmd_phv2mem_t vnet_hdr_no_mrg_rxbuf_cmd;
@pragma dont_trim
metadata dma_cmd_pkt2mem_t pkt_cmd;
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
metadata virtio_rx_qstate VIRTIO_RX_QSTATE_D_SCRATCH;

action read_virtq_rx_qstate(VIRTIO_RX_QSTATE_D_PARAMS)
{
    /* 
     *  The unionization and the scratch already defined in nic/p4/common-p4+/common_rxdma.p4
     */
    modify_field(p4_to_p4plus_scratch.p4plus_app_id, p4_to_p4plus.p4plus_app_id);
    modify_field(p4_to_p4plus_scratch.table0_valid, p4_to_p4plus.table0_valid);
    modify_field(p4_to_p4plus_scratch.table1_valid, p4_to_p4plus.table1_valid);
    modify_field(p4_to_p4plus_scratch.table2_valid, p4_to_p4plus.table2_valid);
    modify_field(p4_to_p4plus_scratch.table3_valid, p4_to_p4plus.table3_valid);

    modify_field(p4_to_p4plus_scratch.vlan_pcp, p4_to_p4plus.vlan_pcp);
    modify_field(p4_to_p4plus_scratch.vlan_dei, p4_to_p4plus.vlan_dei);
    modify_field(p4_to_p4plus_scratch.vlan_vid, p4_to_p4plus.vlan_vid);
    modify_field(p4_to_p4plus_scratch.vlan_valid, p4_to_p4plus.vlan_valid);
    modify_field(p4_to_p4plus_scratch.packet_len, p4_to_p4plus.packet_len);
    modify_field(p4_to_p4plus_scratch.csum, p4_to_p4plus.csum);
    modify_field(p4_to_p4plus_scratch.csum_ip_bad, p4_to_p4plus.csum_ip_bad);
    modify_field(p4_to_p4plus_scratch.csum_ip_ok, p4_to_p4plus.csum_ip_ok);
    modify_field(p4_to_p4plus_scratch.csum_udp_bad, p4_to_p4plus.csum_udp_bad);
    modify_field(p4_to_p4plus_scratch.csum_udp_ok, p4_to_p4plus.csum_udp_ok);
    modify_field(p4_to_p4plus_scratch.csum_tcp_bad, p4_to_p4plus.csum_tcp_bad);
    modify_field(p4_to_p4plus_scratch.csum_tcp_ok, p4_to_p4plus.csum_tcp_ok);

    modify_field(p4_to_p4plus_scratch.rss_flags, p4_to_p4plus.rss_flags);
    modify_field(p4_to_p4plus_scratch.l4_sport, p4_to_p4plus.l4_sport);
    modify_field(p4_to_p4plus_scratch.l4_dport, p4_to_p4plus.l4_dport);
    modify_field(p4_to_p4plus_scratch.ip_sa, p4_to_p4plus.ip_sa);
    modify_field(p4_to_p4plus_scratch.ip_da, p4_to_p4plus.ip_da);

    GENERATE_VIRTIO_RX_QSTATE_D
}

/* Stage 1, Table 0 */
@pragma scratch_metadata
metadata virtq_avail VIRTQ_AVAIL_D_SCRATCH;

action read_rx_virtq_avail(VIRTQ_AVAIL_D_PARAMS)
{
    GENERATE_GLOBAL_K
    GENERATE_VIRTIO_S2S_T0_K

    GENERATE_TO_S1_K

    GENERATE_VIRTQ_AVAIL_D
}

/* Stage 2, Table 0 */
action check_rx_reqs(VIRTIO_RX_QSTATE_NON_STG0_D_PARAMS)
{
    GENERATE_GLOBAL_K

    GENERATE_TO_S2_K

    GENERATE_VIRTIO_RX_QSTATE_NON_STG0_D
}

/* Stage 3, Table 0 */
@pragma scratch_metadata
metadata virtq_desc_idx VIRTQ_DESC_IDX_D_SCRATCH;

action read_rx_head_desc_idx(VIRTQ_DESC_IDX_D_PARAMS)
{
  GENERATE_GLOBAL_K

    GENERATE_TO_S3_K

    GENERATE_VIRTQ_DESC_IDX_D
}


/* Stage 4, Table 0 */
@pragma scratch_metadata
metadata virtq_desc VIRTQ_DESC_D_SCRATCH;

action read_rx_head_desc(VIRTQ_DESC_D_PARAMS)
{
    GENERATE_GLOBAL_K
    GENERATE_VIRTIO_S2S_T0_K

    GENERATE_TO_S4_K

    GENERATE_VIRTQ_DESC_D
}


/* Stage 5, Table 0 */

action read_rx_payload_desc(VIRTQ_DESC_D_PARAMS)
{
    GENERATE_GLOBAL_K
    GENERATE_VIRTIO_S2S_T0_K

    GENERATE_TO_S5_K

    GENERATE_VIRTQ_DESC_D
}
