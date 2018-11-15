#include "../common-p4+/common_txdma_dummy.p4"

/*******************************************************
 * Table names
 ******************************************************/
#define tx_table_s0_t0 gc_tx_initial_action
#define tx_table_s1_t0 gc_tx_read_descr_addr
#define tx_table_s2_t0 gc_tx_read_descr
#define tx_table_s3_t0 gc_tx_read_descr_free_pair_pi

/*******************************************************
 * Action names
 ******************************************************/
#define tx_table_s0_t0_action initial_action
#define tx_table_s1_t0_action read_descr_addr
#define tx_table_s2_t0_action dummy
#define tx_table_s3_t0_action read_descr_free_pair_pi

#include "../common-p4+/common_txdma.p4"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.num_entries_freed, common_phv.num_entries_freed);
    
/******************************************************************************
 * D-vectors
 *****************************************************************************/
header_type gc_txdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        
        CAPRI_QSTATE_HEADER_RING(0) // Total 12 bytes

        ring_base               : 64; // Total 20 bytes

        pad                     : 96; // Total 32 bytes

        // offset 32 (needs to match TCP_GC_CB_SW_PI_OFFSET)
        sw_pi                   : 16;
        sw_ci                   : 16;
    }    
}

// d for stage 1
header_type read_descr_addr_d_t {
    fields {
        desc_addr1              : 64;
        desc_addr2              : 64;
        desc_addr3              : 64;
        desc_addr4              : 64;
        desc_addr5              : 64;
        desc_addr6              : 64;
        desc_addr7              : 64;
        desc_addr8              : 64;
    }
}

// d for stage 3
header_type gc_global_t {
    fields {
        rnmdpr_fp_pi            : 32;
        tnmdpr_fp_pi            : 32;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        num_entries_freed       : 8;
        // global k (max 128)
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

// 160 bytes
header_type common_t0_s2s_phv_t {
    fields {
        dummy                   : 16;
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
metadata gc_global_t read_descr_free_pair_pi_d;
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
metadata ring_entry_t ring_entry3;
@pragma dont_trim
metadata ring_entry_t ring_entry4;
@pragma dont_trim
metadata ring_entry_t ring_entry5;
@pragma dont_trim
metadata ring_entry_t ring_entry6;
@pragma dont_trim
metadata ring_entry_t ring_entry7;
@pragma dont_trim
metadata ring_entry_t ring_entry8;
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
metadata dma_cmd_phv2mem_t ringentry2_dma;      // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_1_dma;            // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_2_dma;            // dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2mem_t ringentry3_dma;      // dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t ci_3_dma;            // dma cmd 6
@pragma dont_trim
metadata dma_cmd_phv2mem_t ringentry4_dma;      // dma cmd 7
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
                             pi_0, ci_0, ring_base, pad, sw_pi, sw_ci) {
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

    modify_field(gc_txdma_initial_d.ring_base, ring_base);
    modify_field(gc_txdma_initial_d.pad, pad);
    modify_field(gc_txdma_initial_d.sw_pi, sw_pi);
    modify_field(gc_txdma_initial_d.sw_ci, sw_ci);
}

/*
 * Stage 1 table 0 action
 */
action read_descr_addr(desc_addr1, desc_addr2, desc_addr3, desc_addr4,
        desc_addr5, desc_addr6, desc_addr7, desc_addr8) {
    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 1
    modify_field(read_descr_addr_d.desc_addr1, desc_addr1);
    modify_field(read_descr_addr_d.desc_addr2, desc_addr2);
    modify_field(read_descr_addr_d.desc_addr3, desc_addr3);
    modify_field(read_descr_addr_d.desc_addr4, desc_addr4);
    modify_field(read_descr_addr_d.desc_addr5, desc_addr5);
    modify_field(read_descr_addr_d.desc_addr6, desc_addr6);
    modify_field(read_descr_addr_d.desc_addr7, desc_addr7);
    modify_field(read_descr_addr_d.desc_addr8, desc_addr8);
}

/*
 * Stage 2 table 0 action
 */
action dummy() {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 2
}

/*
 * Stage 3 table 0 action
 */
action read_descr_free_pair_pi(rnmdpr_fp_pi, tnmdpr_fp_pi) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t0_s2s_scratch.dummy, t0_s2s.dummy);

    // d for stage 3
    modify_field(read_descr_free_pair_pi_d.rnmdpr_fp_pi, rnmdpr_fp_pi);
    modify_field(read_descr_free_pair_pi_d.tnmdpr_fp_pi, tnmdpr_fp_pi);
}

