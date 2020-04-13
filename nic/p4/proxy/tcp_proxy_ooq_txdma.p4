/*****************************************************************************/
/* tcp_proxy_ooq_common.p4
/*****************************************************************************/


/******************************************************************************
 * Table names
 *****************************************************************************/
#include "../common-p4+/common_txdma_dummy.p4"
#include "asic/cmn/asic_common.hpp"

#define tx_table_s0_t0 s0_t0_ooq_tcp_tx
#define tx_table_s0_t1 s0_t1_ooq_tc_tx
#define tx_table_s0_t2 s0_t2_ooq_tcp_tx
#define tx_table_s0_t3 s0_t3_ooq_tcp_tx

#define tx_table_s1_t0 s1_t0_ooq_tcp_tx
#define tx_table_s1_t1 s1_t1_ooq_tcp_tx
#define tx_table_s1_t2 s1_t2_ooq_tcp_tx
#define tx_table_s1_t3 s1_t3_ooq_tcp_tx

#define tx_table_s2_t0 s2_t0_ooq_tcp_tx
#define tx_table_s2_t1 s2_t1_ooq_tcp_tx
#define tx_table_s2_t2 s2_t2_ooq_tcp_tx
#define tx_table_s2_t3 s2_t3_ooq_tcp_tx

#define tx_table_s3_t0 s3_t0_ooq_tcp_tx
#define tx_table_s3_t1 s3_t1_ooq_tcp_tx
#define tx_table_s3_t2 s3_t2_ooq_tcp_tx
#define tx_table_s3_t3 s3_t3_ooq_tcp_tx

#define tx_table_s4_t0 s4_t0_ooq_tcp_tx
#define tx_table_s4_t1 s4_t1_ooq_tcp_tx
#define tx_table_s4_t2 s4_t2_ooq_tcp_tx
#define tx_table_s4_t3 s4_t3_ooq_tcp_tx

#define tx_table_s5_t0 s5_t0_ooq_tcp_tx
#define tx_table_s5_t1 s5_t1_ooq_tcp_tx
#define tx_table_s5_t2 s5_t2_ooq_tcp_tx
#define tx_table_s5_t3 s5_t3_ooq_tcp_tx

#define tx_table_s6_t0 s6_t0_ooq_tcp_tx
#define tx_table_s6_t1 s6_t1_ooq_tcp_tx
#define tx_table_s6_t2 s6_t2_ooq_tcp_tx
#define tx_table_s6_t3 s6_t3_ooq_tcp_tx

#define tx_table_s7_t0 s7_t0_ooq_tcp_tx
#define tx_table_s7_t1 s7_t1_ooq_tcp_tx
#define tx_table_s7_t2 s7_t2_ooq_tcp_tx
#define tx_table_s7_t3 s7_t3_ooq_tcp_tx

#define tx_stage0_lif_params_table lif_params_ooq_tcp_tx
#define tx_table_s5_t4_lif_rate_limiter_table lif_rate_limiter_ooq_tcp_tx

/******************************************************************************
 * Action names
 *****************************************************************************/
#define tx_table_s0_t0_action load_stage0

#define tx_table_s1_t0_action1 load_rx2tx_slot
#define tx_table_s1_t0_action2 process_next_descr_addr
#define tx_table_s1_t1_action free_ooq

#define tx_table_s2_t0_action set_current_ooq

#include "../common-p4+/common_txdma.p4"
#include "tcp_proxy_common.p4"


/******************************************************************************
 * D-vectors
 *****************************************************************************/

// d for stage 0
header_type ooq_tcp_txdma_qstate_d_t {
    fields {
        action_id                   : 8;
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)
        CAPRI_QSTATE_HEADER_RING(1)
        ooq_work_in_progress        : 8;
        ooo_rx2tx_qbase             : 64;
        ooo_rx2tx_free_pi_addr      : 64;
        curr_ooo_qbase              : 64;
        ooo_rx2tx_producer_ci_addr  : 64;
        curr_ooq_num_entries        : 16;
        curr_ooq_trim               : 16;
        curr_index                  : 16;
    }
}

// d for stage 1
header_type ooq_tcp_txdma_load_rnmdr_addr_t {
    fields {
        descr_addr                  : 64;
    }
}

header_type ooq_free_pi_t {
    fields {
        ooq_free_pi                 : 32;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : 34;
        all_ooq_done            : 1;
    }
}

header_type to_stage_1_phv_t {
    fields {
        qbase_addr              : HBM_FULL_ADDRESS_WIDTH;
    }
}

header_type to_stage_2_phv_t {
    fields {
        qbase_addr              : HBM_FULL_ADDRESS_WIDTH;
        num_entries             : 16;
        trim                    : 14;
    }
}

header_type to_stage_3_phv_t {
    fields {
        curr_rnmdr_addr : 64;
        pad   : 64;
    }
}

header_type to_stage_4_phv_t {
    fields {
        curr_rnmdr_addr : 64;
        pad   : 64;
    }
}

header_type to_stage_5_phv_t {
    fields {
        new_qbase_addr : 64;
        num_entries    : 16;
        curr_index     : 16;
        new_processing : 1;
        pad            : 31;
    }
}

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.all_ooq_done, common_phv.all_ooq_done);

/******************************************************************************
 * scratch for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata ooq_tcp_txdma_qstate_d_t ooq_tcp_txdma_qstate_d;
@pragma scratch_metadata
metadata ooq_rx2tx_queue_entry_t ooq_tcp_txdma_load_rx2tx_slot_d;
@pragma scratch_metadata
metadata ooq_free_pi_t ooq_free_pi_d;
@pragma scratch_metadata
metadata pkt_descr_aol_t read_descr_d;
@pragma scratch_metadata
metadata ooq_tcp_txdma_load_rnmdr_addr_t ooq_tcp_txdma_load_rnmdr_addr;

/******************************************************************************
 * Scratch for k-vector generation
 *****************************************************************************/
@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;


/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;
@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;
@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;
@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;
@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;


/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata cap_phv_intr_rxdma_t intr_rxdma;
@pragma dont_trim
metadata cap_phv_intr_rxdma_t intr_rxdma2;
@pragma dont_trim
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_app_hdr;
@pragma dont_trim
metadata ring_entry_t ooq_slot;
@pragma dont_trim
metadata semaphore_ci_t ooq_free_pi;

header_type tx2rx_feedback_type_t {
    fields {
        entry : 8;
    }
}
@pragma dont_trim
metadata tx2rx_feedback_type_t feedback_type;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic;       // dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t ooq_ring_entry;  // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t ooq_alloc_ci;    // dma cmd 3
@pragma dont_trim
metadata dma_cmd_mem2pkt_t tcp_app_header;  // dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic2;      // dma cmd 5
@pragma dont_trim
metadata dma_cmd_mem2pkt_t tcp_app_header2; // dma cmd 6
@pragma dont_trim
metadata dma_cmd_phv2pkt_t tcp_app_hdr1;     // dma cmd 8
@pragma dont_trim
metadata dma_cmd_phv2pkt_t feedback;        // dma cmd 7


/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

#define STAGE0_PARAMS \
    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0, pi_1, ci_1, \
    ooq_work_in_progress, ooo_rx2tx_qbase, ooo_rx2tx_free_pi_addr, \
    curr_ooo_qbase, ooo_rx2tx_producer_ci_addr, curr_ooq_num_entries, \
    curr_ooq_trim, curr_index

#define GENERATE_STAGE0_D \
    modify_field(ooq_tcp_txdma_qstate_d.rsvd, rsvd); \
    modify_field(ooq_tcp_txdma_qstate_d.cosA, cosA); \
    modify_field(ooq_tcp_txdma_qstate_d.cosB, cosB); \
    modify_field(ooq_tcp_txdma_qstate_d.cos_sel, cos_sel); \
    modify_field(ooq_tcp_txdma_qstate_d.eval_last, eval_last); \
    modify_field(ooq_tcp_txdma_qstate_d.host, host); \
    modify_field(ooq_tcp_txdma_qstate_d.total, total); \
    modify_field(ooq_tcp_txdma_qstate_d.pid, pid); \
 \
    modify_field(ooq_tcp_txdma_qstate_d.pi_0, pi_0); \
    modify_field(ooq_tcp_txdma_qstate_d.ci_0, ci_0); \
    modify_field(ooq_tcp_txdma_qstate_d.pi_1, pi_1); \
    modify_field(ooq_tcp_txdma_qstate_d.ci_1, ci_1); \
 \
    modify_field(ooq_tcp_txdma_qstate_d.ooq_work_in_progress, ooq_work_in_progress); \
    modify_field(ooq_tcp_txdma_qstate_d.ooo_rx2tx_qbase, ooo_rx2tx_qbase); \
    modify_field(ooq_tcp_txdma_qstate_d.ooo_rx2tx_free_pi_addr, ooo_rx2tx_free_pi_addr); \
    modify_field(ooq_tcp_txdma_qstate_d.curr_ooo_qbase, curr_ooo_qbase); \
    modify_field(ooq_tcp_txdma_qstate_d.ooo_rx2tx_producer_ci_addr, ooo_rx2tx_producer_ci_addr); \
    modify_field(ooq_tcp_txdma_qstate_d.curr_ooq_num_entries, curr_ooq_num_entries); \
    modify_field(ooq_tcp_txdma_qstate_d.curr_ooq_trim, curr_ooq_trim); \
    modify_field(ooq_tcp_txdma_qstate_d.curr_index, curr_index); \

// Stage-0 Table-0
action load_stage0(STAGE0_PARAMS)
{
    GENERATE_STAGE0_D
}

// Stage-1 Table-0
action load_rx2tx_slot(qbase_addr, num_entries, trim)
{
    GENERATE_GLOBAL_K
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.qbase_addr, qbase_addr);
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.num_entries, num_entries);
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.trim, trim);
}

// Stage-1 Table-0
action process_next_descr_addr(descr_addr)
{
    GENERATE_GLOBAL_K
    modify_field(ooq_tcp_txdma_load_rnmdr_addr.descr_addr, descr_addr);
}

// Stage-1 Table-1
action free_ooq(ooq_free_pi)
{
    GENERATE_GLOBAL_K
    modify_field(to_s1_scratch.qbase_addr, to_s1.qbase_addr);
    modify_field(ooq_free_pi_d.ooq_free_pi, ooq_free_pi);
}

action set_current_ooq(action_id, STAGE0_PARAMS)
{
    GENERATE_GLOBAL_K
    modify_field(to_s2_scratch.qbase_addr, to_s2.qbase_addr);
    modify_field(to_s2_scratch.num_entries, to_s2.num_entries);
    modify_field(to_s2_scratch.trim, to_s2.trim);

    modify_field(ooq_tcp_txdma_qstate_d.action_id, action_id);
    GENERATE_STAGE0_D
}
