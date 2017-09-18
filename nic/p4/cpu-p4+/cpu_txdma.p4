#include "../common-p4+/common_txdma_dummy.p4"

/*******************************************************
 * Table and actions
 ******************************************************/
#define tx_table_s0_t0 cpu_tx_initial_action
#define tx_table_s0_t0_action cpu_tx_initial_action

#define tx_table_s1_t0 cpu_tx_read_asq_ci
#define tx_table_s1_t0_action read_asq_ci
#
#define tx_table_s2_t0 cpu_tx_read_asq_descr
#define tx_table_s2_t0_action read_asq_descr
#define tx_table_s2_t1 cpu_tx_asq_consume
#define tx_table_s2_t1_action asq_consume

#define tx_table_s3_t0 cpu_tx_write_pkt
#define tx_table_s3_t0_action write_pkt

#include "../common-p4+/common_txdma.p4"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \

/******************************************************************************
 * D-vectors
 *****************************************************************************/
header_type cpu_txdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        
        CAPRI_QSTATE_HEADER_RING(0)

        CAPRI_QSTATE_HEADER_RING(1)

        CAPRI_QSTATE_HEADER_RING(2)

        CAPRI_QSTATE_HEADER_RING(3)

        CAPRI_QSTATE_HEADER_RING(4)

        CAPRI_QSTATE_HEADER_RING(5) // Total 32 bytes
        
        asq_base    : HBM_ADDRESS_WIDTH; 
    }    
}

// d for stage1
header_type read_asq_ci_d_t {
    fields {
        desc_addr      : ADDRESS_WIDTH;     
    }    
}

// d fpr stage2
// use pkt_descr_t


/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        qstate_addr             : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_1_phv_t {
    fields {
        asq_ci_addr             : HBM_ADDRESS_WIDTH; 
    }    
}

header_type to_stage_2_phv_t {
    fields {
        asq_cidx                : 16;     
    }    
}

header_type to_stage_3_phv_t {
    fields {
        asq_desc_addr           : HBM_ADDRESS_WIDTH;    
        page_addr               : HBM_ADDRESS_WIDTH;
        offset                  : 16;
        len                     : 16;
    }    
}

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata cpu_txdma_initial_action_t cpu_txdma_initial_d;

@pragma scratch_metadata
metadata read_asq_ci_d_t read_asq_ci_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t read_asq_descr_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t cpu_app_header;

@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;

@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;


@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;

@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd0;

@pragma dont_trim
metadata dma_cmd_mem2pkt_t dma_cmd1;


/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 action
 */
action cpu_tx_initial_action(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
                             pi_0, ci_0, asq_base) {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // from app header
    //modify_field(cpu_scratch_app.p4plus_app_id, cpu_app_header.p4plus_app_id);
    //modify_field(cpu_scratch_app.table0_valid, cpu_app_header.table0_valid);
    //modify_field(cpu_scratch_app.table1_valid, cpu_app_header.table1_valid);
    //modify_field(cpu_scratch_app.table2_valid, cpu_app_header.table2_valid);
    //modify_field(cpu_scratch_app.table3_valid, cpu_app_header.table3_valid);

    // d for stage 0
    
    modify_field(cpu_txdma_initial_d.rsvd, rsvd);
    modify_field(cpu_txdma_initial_d.cosA, cosA);
    modify_field(cpu_txdma_initial_d.cosB, cosB);
    modify_field(cpu_txdma_initial_d.cos_sel, cos_sel);
    modify_field(cpu_txdma_initial_d.eval_last, eval_last);
    modify_field(cpu_txdma_initial_d.host, host);
    modify_field(cpu_txdma_initial_d.total, total);
    modify_field(cpu_txdma_initial_d.pid, pid);

    modify_field(cpu_txdma_initial_d.pi_0, pi_0);
    modify_field(cpu_txdma_initial_d.ci_0, ci_0);
    
    modify_field(cpu_txdma_initial_d.asq_base, asq_base);
}

// Stage 1 table 0
action read_asq_ci(desc_addr) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1
    modify_field(to_s1_scratch.asq_ci_addr, to_s1.asq_ci_addr);

    // d for stage 1
    modify_field(read_asq_ci_d.desc_addr, desc_addr);
}

// Stage 2 table 0
action read_asq_descr(A0, O0, L0, A1, O1, L1, A2, O2, L2, next_addr, next_pkt) {
     // from ki global
    GENERATE_GLOBAL_K

   
    // from to_stage 2
    modify_field(to_s2_scratch.asq_cidx, to_s2.asq_cidx);

    // d for stage 2
    modify_field(read_asq_descr_d.A0, A0);
    modify_field(read_asq_descr_d.O0, O0);
    modify_field(read_asq_descr_d.L0, L0);
    modify_field(read_asq_descr_d.A1, A1);
    modify_field(read_asq_descr_d.O1, O1);
    modify_field(read_asq_descr_d.L1, L1);
    modify_field(read_asq_descr_d.A2, A2);
    modify_field(read_asq_descr_d.O2, O2);
    modify_field(read_asq_descr_d.L2, L2);
    modify_field(read_asq_descr_d.next_addr, next_addr);
    modify_field(read_asq_descr_d.next_pkt, next_pkt);
}

// Stage 2 table 1
action asq_consume() {
    // from ki global
    GENERATE_GLOBAL_K

    modify_field(to_s2_scratch.asq_cidx, to_s2.asq_cidx);    
}


// Stage 3 table 0
action write_pkt() {
    // from ki global
    GENERATE_GLOBAL_K

    modify_field(to_s3_scratch.asq_desc_addr, to_s3.asq_desc_addr);
    modify_field(to_s3_scratch.page_addr, to_s3.page_addr);
    modify_field(to_s3_scratch.offset, to_s3.offset);
    modify_field(to_s3_scratch.len, to_s3.len);
}
