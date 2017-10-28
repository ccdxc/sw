#include "../common-p4+/common_txdma_dummy.p4"

/*******************************************************
 * Table names
 ******************************************************/
#define tx_table_s0_t0 gc_tx_initial_action
#define tx_table_s1_t0 gc_tx_read_descr_addr
#define tx_table_s2_t0 gc_tx_read_descr
#define tx_table_s3_t0 gc_tx_read_descr_free_pair_pi
#define tx_table_s4_t0 gc_tx_read_page_free_pair_pi

/*******************************************************
 * Action names
 ******************************************************/
#define tx_table_s0_t0_action initial_action
#define tx_table_s1_t0_action read_descr_addr
#define tx_table_s2_t0_action read_descr
#define tx_table_s3_t0_action read_descr_free_pair_pi
#define tx_table_s4_t0_action read_page_free_pair_pi

#include "../common-p4+/common_txdma.p4"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.desc_addr, common_phv.desc_addr);
    
/******************************************************************************
 * D-vectors
 *****************************************************************************/
header_type gc_txdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        
        CAPRI_QSTATE_HEADER_RING(0)

        CAPRI_QSTATE_HEADER_RING(1)

        CAPRI_QSTATE_HEADER_RING(2)

        CAPRI_QSTATE_HEADER_RING(3)

        CAPRI_QSTATE_HEADER_RING(4)

        CAPRI_QSTATE_HEADER_RING(5) // Total 32 bytes

        ring_base               : 64;
        ring_shift              : 8;
    }    
}

// d for stage 1
header_type read_descr_addr_d_t {
    fields {
        desc_addr               : 64;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        desc_addr               : 34;
        // global k (max 128)
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

// 160 bytes
header_type common_t0_s2s_phv_t {
    fields {
        a0                      : 34;
        a1                      : 34;
        a2                      : 34;
    }
}

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata gc_txdma_initial_action_t gc_txdma_initial_d;
@pragma scratch_metadata
metadata read_descr_addr_d_t read_descr_addr_d;
@pragma scratch_metadata
metadata pkt_descr_aol_t read_descr_d;
@pragma scratch_metadata
metadata semaphore_pi_t read_descr_free_pair_pi_d;
@pragma scratch_metadata
metadata semaphore_pi_t read_page_free_pair_pi_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma pa_header_union ingress common_t0_s2s
metadata common_t0_s2s_phv_t t0_s2s;


@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;
@pragma scratch_metadata
metadata common_t0_s2s_phv_t t0_s2s_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata ring_entry_t ring_entry1;
@pragma dont_trim
metadata ring_entry_t ring_entry2;
@pragma dont_trim
metadata semaphore_ci_t ci_1;
@pragma dont_trim
metadata semaphore_ci_t ci_2;
@pragma dont_trim
metadata semaphore_ci_t ci_3;
@pragma dont_trim
metadata semaphore_ci_t ci_4;
@pragma dont_trim
metadata dma_cmd_phv2mem_t ringentry1_dma;      // dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_1_dma;            // dma cmd 2
@pragma dont_trim
metadata dma_cmd_mem2mem_t ringentry2_dma;      // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_2_dma;            // dma cmd 4
@pragma dont_trim
metadata dma_cmd_mem2mem_t ringentry3_dma;      // dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_3_dma;            // dma cmd 6
@pragma dont_trim
metadata dma_cmd_mem2mem_t ringentry4_dma;      // dma cmd 7
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_4_dma;            // dma cmd 8

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 action
 */
action initial_action(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
                             pi_0, ci_0, pi_1, ci_1, pi_2, ci_2, pi_3, ci_3,
                             pi_4, ci_4, pi_5, ci_5, ring_base, ring_shift) {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // from app header

    // d for stage 0
    
    modify_field(gc_txdma_initial_d.rsvd, rsvd);
    modify_field(gc_txdma_initial_d.cosA, cosA);
    modify_field(gc_txdma_initial_d.cosB, cosB);
    modify_field(gc_txdma_initial_d.cos_sel, cos_sel);
    modify_field(gc_txdma_initial_d.eval_last, eval_last);
    modify_field(gc_txdma_initial_d.host, host);
    modify_field(gc_txdma_initial_d.total, total);
    modify_field(gc_txdma_initial_d.pid, pid);

    modify_field(gc_txdma_initial_d.pi_0, pi_0);
    modify_field(gc_txdma_initial_d.ci_0, ci_0);
    modify_field(gc_txdma_initial_d.pi_1, pi_1);
    modify_field(gc_txdma_initial_d.ci_1, ci_1);
    modify_field(gc_txdma_initial_d.pi_2, pi_2);
    modify_field(gc_txdma_initial_d.ci_2, ci_2);
    modify_field(gc_txdma_initial_d.pi_3, pi_3);
    modify_field(gc_txdma_initial_d.ci_3, ci_3);
    modify_field(gc_txdma_initial_d.pi_4, pi_4);
    modify_field(gc_txdma_initial_d.ci_4, ci_4);
    modify_field(gc_txdma_initial_d.pi_5, pi_5);
    modify_field(gc_txdma_initial_d.ci_5, ci_5);

    modify_field(gc_txdma_initial_d.ring_base, ring_base);
    modify_field(gc_txdma_initial_d.ring_shift, ring_shift);
}

/*
 * Stage 1 table 0 action
 */
action read_descr_addr(desc_addr) {
    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 1
    modify_field(read_descr_addr_d.desc_addr, desc_addr);
}

/*
 * Stage 2 table 0 action
 */
action read_descr(A0, O0, L0, A1, O1, L1, A2, O2, L2) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t0_s2s_scratch.a0, t0_s2s.a0);
    modify_field(t0_s2s_scratch.a1, t0_s2s.a1);
    modify_field(t0_s2s_scratch.a2, t0_s2s.a2);

    // d for stage 2
    modify_field(read_descr_d.A0, A0);
    modify_field(read_descr_d.O0, O0);
    modify_field(read_descr_d.L0, L0);
    modify_field(read_descr_d.A1, A1);
    modify_field(read_descr_d.O1, O1);
    modify_field(read_descr_d.L1, L1);
    modify_field(read_descr_d.A2, A2);
    modify_field(read_descr_d.O2, O2);
    modify_field(read_descr_d.L2, L2);
}

/*
 * Stage 3 table 0 action
 */
action read_descr_free_pair_pi(index) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t0_s2s_scratch.a0, t0_s2s.a0);
    modify_field(t0_s2s_scratch.a1, t0_s2s.a1);
    modify_field(t0_s2s_scratch.a2, t0_s2s.a2);

    // d for stage 3
    modify_field(read_descr_free_pair_pi_d.index, index);
}

/*
 * Stage 4 table 0 action
 */
action read_page_free_pair_pi(index) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t0_s2s_scratch.a0, t0_s2s.a0);
    modify_field(t0_s2s_scratch.a1, t0_s2s.a1);
    modify_field(t0_s2s_scratch.a2, t0_s2s.a2);

    // d for stage 4
    modify_field(read_page_free_pair_pi_d.index, index);
}
