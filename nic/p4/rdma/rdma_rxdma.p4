/*****************************************************************************/
/* rdma_rxdma.p4
/*****************************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

#define common_p4plus_stage0_app_header_table rdma_stage0_table
#define common_p4plus_stage0_app_header_table_action_dummy rdma_stage0_bth_action
#define common_p4plus_stage0_app_header_table_action_dummy1 rdma_stage0_bth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy2 rdma_stage0_bth_reth_action
#define common_p4plus_stage0_app_header_table_action_dummy3 rdma_stage0_bth_reth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy4 rdma_stage0_bth_aeth_action
#define common_p4plus_stage0_app_header_table_action_dummy5 rdma_stage0_bth_aeth_atomicaeth_action
#define common_p4plus_stage0_app_header_table_action_dummy6 rdma_stage0_bth_atomiceth_action
#define common_p4plus_stage0_app_header_table_action_dummy7 rdma_stage0_bth_ieth_action
#define common_p4plus_stage0_app_header_table_action_dummy8 rdma_stage0_bth_deth_action
#define common_p4plus_stage0_app_header_table_action_dummy9 rdma_stage0_bth_deth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy10 rdma_stage0_bth_xrceth_action
#define common_p4plus_stage0_app_header_table_action_dummy11 rdma_stage0_bth_xrceth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy12 rdma_stage0_bth_xrceth_reth_action
#define common_p4plus_stage0_app_header_table_action_dummy13 rdma_stage0_bth_xrceth_reth_immeth_action
#define common_p4plus_stage0_app_header_table_action_dummy14 rdma_stage0_bth_xrceth_atomiceth_action
#define common_p4plus_stage0_app_header_table_action_dummy15 rdma_stage0_bth_xrceth_ieth_action
#define common_p4plus_stage0_app_header_table_action_dummy16 rdma_stage0_recirc_action
#define common_p4plus_stage0_app_header_table_action_dummy17 rdma_stage0_ud_feedback_action

#include "../common-p4+/common_rxdma.p4"

#include "./rdma_rxdma_headers.p4"

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
 * Header unions for PHV layout
 *****************************************************************************/

@pragma pa_header_union ingress app_header rdma_recirc rdma_bth rdma_bth_immeth rdma_bth_reth rdma_bth_reth_immeth rdma_bth_aeth rdma_bth_aeth_atomicaeth rdma_bth_atomiceth rdma_bth_ieth rdma_bth_deth rdma_bth_deth_immeth rdma_bth_xrceth rdma_bth_xrceth_immeth rdma_bth_xrceth_reth rdma_bth_xrceth_reth_immeth rdma_bth_xrceth_atomiceth rdma_bth_xrceth_ieth rdma_ud_feedback

metadata roce_recirc_header_t rdma_recirc;
metadata p4_to_p4plus_roce_bth_header_t rdma_bth;
metadata p4_to_p4plus_roce_bth_immeth_header_t rdma_bth_immeth;
metadata p4_to_p4plus_roce_bth_reth_header_t rdma_bth_reth;
metadata p4_to_p4plus_roce_bth_reth_immeth_header_t rdma_bth_reth_immeth;
metadata p4_to_p4plus_roce_bth_aeth_header_t rdma_bth_aeth;
metadata p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t rdma_bth_aeth_atomicaeth;
metadata p4_to_p4plus_roce_bth_atomiceth_header_t rdma_bth_atomiceth;
metadata p4_to_p4plus_roce_bth_ieth_header_t rdma_bth_ieth;
metadata p4_to_p4plus_roce_bth_deth_header_t rdma_bth_deth;
metadata p4_to_p4plus_roce_bth_deth_immeth_header_t rdma_bth_deth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_header_t rdma_bth_xrceth;
metadata p4_to_p4plus_roce_bth_xrceth_immeth_header_t rdma_bth_xrceth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_reth_header_t rdma_bth_xrceth_reth;
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t rdma_bth_xrceth_reth_immeth;
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t rdma_bth_xrceth_atomiceth;
metadata p4_to_p4plus_roce_bth_xrceth_ieth_header_t rdma_bth_xrceth_ieth;
metadata rdma_ud_feedback_header_t rdma_ud_feedback;



@pragma scratch_metadata
metadata roce_recirc_header_t rdma_recirc_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_header_t rdma_bth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_immeth_header_t rdma_bth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_reth_header_t rdma_bth_reth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_reth_immeth_header_t rdma_bth_reth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_aeth_header_t rdma_bth_aeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_aeth_atomicaeth_header_t rdma_bth_aeth_atomicaeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_atomiceth_header_t rdma_bth_atomiceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_ieth_header_t rdma_bth_ieth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_header_t rdma_bth_deth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_deth_immeth_header_t rdma_bth_deth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_header_t rdma_bth_xrceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_immeth_header_t rdma_bth_xrceth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_header_t rdma_bth_xrceth_reth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_reth_immeth_header_t rdma_bth_xrceth_reth_immeth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_atomiceth_header_t rdma_bth_xrceth_atomiceth_scr;
@pragma scratch_metadata
metadata p4_to_p4plus_roce_bth_xrceth_ieth_header_t rdma_bth_xrceth_ieth_scr;
@pragma scratch_metadata
metadata rdma_ud_feedback_header_t rdma_ud_feedback_scr;


/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 recirc action
 */
action rdma_stage0_recirc_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_recirc_scr.app_data0, rdma_recirc.app_data0);
    modify_field(rdma_recirc_scr.app_data1, rdma_recirc.app_data1);
    modify_field(rdma_recirc_scr.app_data2, rdma_recirc.app_data2);

    // recirc header bits
    modify_field(rdma_recirc_scr.token_id, rdma_recirc.token_id);
    modify_field(rdma_recirc_scr.recirc_reason, rdma_recirc.recirc_reason);
    modify_field(rdma_recirc_scr.rsvd, rdma_recirc.rsvd);
}

/*
 * Stage 0 table 0 bth action
 */
action rdma_stage0_bth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from p4-to-p4plus-rdma-hdr
    modify_field(rdma_bth_scr.p4plus_app_id, rdma_bth.p4plus_app_id);
    modify_field(rdma_bth_scr.table0_valid, rdma_bth.table0_valid);
    modify_field(rdma_bth_scr.table1_valid, rdma_bth.table1_valid);
    modify_field(rdma_bth_scr.table2_valid, rdma_bth.table2_valid);
    modify_field(rdma_bth_scr.table3_valid, rdma_bth.table3_valid);
    modify_field(rdma_bth_scr.rdma_hdr_len, rdma_bth.rdma_hdr_len);
    modify_field(rdma_bth_scr.raw_flags, rdma_bth.raw_flags);
    modify_field(rdma_bth_scr.ecn, rdma_bth.ecn);
    modify_field(rdma_bth_scr.payload_len, rdma_bth.payload_len);

    // bth params
    modify_field(rdma_bth_scr.bth_opcode, rdma_bth.bth_opcode);
    modify_field(rdma_bth_scr.bth_se, rdma_bth.bth_se);
    modify_field(rdma_bth_scr.bth_m, rdma_bth.bth_m);
    modify_field(rdma_bth_scr.bth_pad, rdma_bth.bth_pad);
    modify_field(rdma_bth_scr.bth_tver, rdma_bth.bth_tver);
    modify_field(rdma_bth_scr.bth_pkey, rdma_bth.bth_pkey);
    modify_field(rdma_bth_scr.bth_f, rdma_bth.bth_f);
    modify_field(rdma_bth_scr.bth_b, rdma_bth.bth_b);
    modify_field(rdma_bth_scr.bth_rsvd1, rdma_bth.bth_rsvd1);
    modify_field(rdma_bth_scr.bth_dst_qp, rdma_bth.bth_dst_qp);
    modify_field(rdma_bth_scr.bth_a, rdma_bth.bth_a);
    modify_field(rdma_bth_scr.bth_rsvd2, rdma_bth.bth_rsvd2);
    modify_field(rdma_bth_scr.bth_psn, rdma_bth.bth_psn);

}

/*
 * Stage 0 table 0 bth immeth action
 */
action rdma_stage0_bth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_immeth_scr.common_header_bits, rdma_bth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_immeth_scr.bth_header_bits, rdma_bth_immeth.bth_header_bits);

    // immeth params
    modify_field(rdma_bth_immeth_scr.immeth_data, rdma_bth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth reth action
 */
action rdma_stage0_bth_reth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_reth_scr.common_header_bits, rdma_bth_reth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_reth_scr.bth_header_bits, rdma_bth_reth.bth_header_bits);

    // reth params
    modify_field(rdma_bth_reth_scr.reth_va, rdma_bth_reth.reth_va);
    modify_field(rdma_bth_reth_scr.reth_r_key, rdma_bth_reth.reth_r_key);
    modify_field(rdma_bth_reth_scr.reth_dma_len1, rdma_bth_reth.reth_dma_len1);
    modify_field(rdma_bth_reth_scr.reth_dma_len2, rdma_bth_reth.reth_dma_len2);
}


/*
 * Stage 0 table 0 bth reth immeth action
 */
action rdma_stage0_bth_reth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_reth_immeth_scr.common_header_bits, rdma_bth_reth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_reth_immeth_scr.bth_header_bits, rdma_bth_reth_immeth.bth_header_bits);

    // reth bits
    modify_field(rdma_bth_reth_immeth_scr.reth_header_bits_1, rdma_bth_reth_immeth.reth_header_bits_1);
    modify_field(rdma_bth_reth_immeth_scr.reth_header_bits_2, rdma_bth_reth_immeth.reth_header_bits_2);

    // reth_immeth params
    modify_field(rdma_bth_reth_immeth_scr.immeth_data, rdma_bth_reth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth aeth action
 */
action rdma_stage0_bth_aeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_aeth_scr.common_header_bits, rdma_bth_aeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_aeth_scr.bth_header_bits, rdma_bth_aeth.bth_header_bits);

    // aeth params
    modify_field(rdma_bth_aeth_scr.aeth_syndrome, rdma_bth_aeth.aeth_syndrome);
    modify_field(rdma_bth_aeth_scr.aeth_msn, rdma_bth_aeth.aeth_msn);
}


/*
 * Stage 0 table 0 bth aeth_atomicaeth action
 */
action rdma_stage0_bth_aeth_atomicaeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_aeth_atomicaeth_scr.common_header_bits, rdma_bth_aeth_atomicaeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_aeth_atomicaeth_scr.bth_header_bits, rdma_bth_aeth_atomicaeth.bth_header_bits);

    // aeth bits
    modify_field(rdma_bth_aeth_atomicaeth_scr.aeth_header_bits, rdma_bth_aeth_atomicaeth.aeth_header_bits);

    // atomicaeth params
    modify_field(rdma_bth_aeth_atomicaeth_scr.atomicaeth_orig_data, rdma_bth_aeth_atomicaeth.atomicaeth_orig_data);
}

/*
 * Stage 0 table 0 bth atomiceth action
 */
action rdma_stage0_bth_atomiceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_atomiceth_scr.common_header_bits, rdma_bth_atomiceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_atomiceth_scr.bth_header_bits, rdma_bth_atomiceth.bth_header_bits);

    // atomiceth params
    modify_field(rdma_bth_atomiceth_scr.atomiceth_va, rdma_bth_atomiceth.atomiceth_va);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_r_key, rdma_bth_atomiceth.atomiceth_r_key);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_swap_or_add_data, rdma_bth_atomiceth.atomiceth_swap_or_add_data);
    modify_field(rdma_bth_atomiceth_scr.atomiceth_cmp_data, rdma_bth_atomiceth.atomiceth_cmp_data);
}

/*
 * Stage 0 table 0 bth ieth action
 */
action rdma_stage0_bth_ieth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_ieth_scr.common_header_bits, rdma_bth_ieth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_ieth_scr.bth_header_bits, rdma_bth_ieth.bth_header_bits);

    // ieth params
    modify_field(rdma_bth_ieth_scr.ieth_r_key, rdma_bth_ieth.ieth_r_key);
}


/*
 * Stage 0 table 0 bth deth action
 */
action rdma_stage0_bth_deth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_deth_scr.common_header_bits, rdma_bth_deth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_deth_scr.bth_header_bits, rdma_bth_deth.bth_header_bits);

    // deth params
    modify_field(rdma_bth_deth_scr.deth_q_key, rdma_bth_deth.deth_q_key);
    modify_field(rdma_bth_deth_scr.deth_rsvd, rdma_bth_deth.deth_rsvd);
    modify_field(rdma_bth_deth_scr.deth_src_qp, rdma_bth_deth.deth_src_qp);
    modify_field(rdma_bth_deth_scr.smac, rdma_bth_deth.smac);
    modify_field(rdma_bth_deth_scr.dmac, rdma_bth_deth.dmac);
}


/*
 * Stage 0 table 0 bth deth immeth action
 */
action rdma_stage0_bth_deth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_deth_immeth_scr.common_header_bits, rdma_bth_deth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_deth_immeth_scr.bth_header_bits, rdma_bth_deth_immeth.bth_header_bits);

    // deth bits
    modify_field(rdma_bth_deth_immeth_scr.deth_header_bits, rdma_bth_deth_immeth.deth_header_bits);

    // deth_immeth params
    modify_field(rdma_bth_deth_immeth_scr.immeth_data, rdma_bth_deth_immeth.immeth_data);
    modify_field(rdma_bth_deth_immeth_scr.smac, rdma_bth_deth_immeth.smac);
}


/*
 * Stage 0 table 0 bth xrceth action
 */
action rdma_stage0_bth_xrceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_scr.common_header_bits, rdma_bth_xrceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_scr.bth_header_bits, rdma_bth_xrceth.bth_header_bits);

    // xrceth params
    modify_field(rdma_bth_xrceth_scr.xrceth_rsvd, rdma_bth_xrceth.xrceth_rsvd);
    modify_field(rdma_bth_xrceth_scr.xrceth_xrcsrq, rdma_bth_xrceth.xrceth_xrcsrq);
}


/*
 * Stage 0 table 0 bth xrceth immeth action
 */
action rdma_stage0_bth_xrceth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_immeth_scr.common_header_bits, rdma_bth_xrceth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_immeth_scr.bth_header_bits, rdma_bth_xrceth_immeth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_immeth_scr.xrceth_header_bits, rdma_bth_xrceth_immeth.xrceth_header_bits);

    // xrceth_immeth params
    modify_field(rdma_bth_xrceth_immeth_scr.immeth_data, rdma_bth_xrceth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth xrceth reth action
 */
action rdma_stage0_bth_xrceth_reth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_reth_scr.common_header_bits, rdma_bth_xrceth_reth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_reth_scr.bth_header_bits, rdma_bth_xrceth_reth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_reth_scr.xrceth_header_bits, rdma_bth_xrceth_reth.xrceth_header_bits);

    // reth params
    modify_field(rdma_bth_xrceth_reth_scr.reth_va, rdma_bth_xrceth_reth.reth_va);
    modify_field(rdma_bth_xrceth_reth_scr.reth_r_key, rdma_bth_xrceth_reth.reth_r_key);
    modify_field(rdma_bth_xrceth_reth_scr.reth_dma_len, rdma_bth_xrceth_reth.reth_dma_len);
}


/*
 * Stage 0 table 0 bth reth immeth action
 */
action rdma_stage0_bth_xrceth_reth_immeth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_reth_immeth_scr.common_header_bits, rdma_bth_xrceth_reth_immeth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.bth_header_bits, rdma_bth_xrceth_reth_immeth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.xrceth_header_bits, rdma_bth_xrceth_reth_immeth.xrceth_header_bits);

    // reth bits
    modify_field(rdma_bth_xrceth_reth_immeth_scr.reth_header_bits, rdma_bth_xrceth_reth_immeth.reth_header_bits);

    // reth_immeth params
    modify_field(rdma_bth_xrceth_reth_immeth_scr.immeth_data, rdma_bth_xrceth_reth_immeth.immeth_data);
}


/*
 * Stage 0 table 0 bth atomiceth action
 */
action rdma_stage0_bth_xrceth_atomiceth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_atomiceth_scr.common_header_bits, rdma_bth_xrceth_atomiceth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_atomiceth_scr.bth_header_bits, rdma_bth_xrceth_atomiceth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_atomiceth_scr.xrceth_header_bits, rdma_bth_xrceth_atomiceth.xrceth_header_bits);

    // atomiceth params
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_va, rdma_bth_xrceth_atomiceth.atomiceth_va);
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_r_key, rdma_bth_xrceth_atomiceth.atomiceth_r_key);
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_swap_or_add_data, rdma_bth_xrceth_atomiceth.atomiceth_swap_or_add_data);
    modify_field(rdma_bth_xrceth_atomiceth_scr.atomiceth_cmp_data, rdma_bth_xrceth_atomiceth.atomiceth_cmp_data);
}

/*
 * Stage 0 table 0 bth ieth action
 */
action rdma_stage0_bth_xrceth_ieth_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_bth_xrceth_ieth_scr.common_header_bits, rdma_bth_xrceth_ieth.common_header_bits);

    // bth bits
    modify_field(rdma_bth_xrceth_ieth_scr.bth_header_bits, rdma_bth_xrceth_ieth.bth_header_bits);

    // xrceth bits
    modify_field(rdma_bth_xrceth_ieth_scr.xrceth_header_bits, rdma_bth_xrceth_ieth.xrceth_header_bits);

    // ieth params
    modify_field(rdma_bth_xrceth_ieth_scr.ieth_r_key, rdma_bth_xrceth_ieth.ieth_r_key);
}

/*
 * Stage 0 table 0 ud_feedback action
 */
action rdma_stage0_ud_feedback_action () {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(rdma_ud_feedback_scr.common_header_bits, rdma_ud_feedback.common_header_bits);

    // ud_feedback_header bits
    modify_field(rdma_ud_feedback_scr.feedback_type, rdma_ud_feedback.feedback_type);
    modify_field(rdma_ud_feedback_scr.wrid, rdma_ud_feedback.wrid);
    modify_field(rdma_ud_feedback_scr.optype, rdma_ud_feedback.optype);
    modify_field(rdma_ud_feedback_scr.status, rdma_ud_feedback.status);
}

