/*****************************************************************************/
/* rdma_txdma.p4
/*****************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0 rdma_stage0_table
#define tx_table_s0_t0_action  rdma_stage0_table_action
#define tx_table_s0_t0_action1 rdma_stage0_recirc_action

#include "../common-p4+/common_txdma.p4"
#include "./rdma_txdma_headers.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * D-vectors
 *****************************************************************************/

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress app_header rdma_recirc

/* metadata common_global_phv_t common_phv; */
metadata roce_recirc_header_t rdma_recirc;

@pragma scratch_metadata
/* metadata common_global_phv_t common_global_scratch; */
metadata roce_recirc_header_t rdma_recirc_scr;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/
/*
 * Stage 0 table 0 action
 */
action rdma_stage0_table_action() {

    // k + i for stage 0
}

/*
 * Stage 0 table 0 recirc action
 */
action rdma_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // recirc header bits
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
}
