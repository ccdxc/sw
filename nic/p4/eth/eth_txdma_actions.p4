/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_txdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy eth_tx_fetch_desc
#define tx_table_s0_t0_action eth_tx_fetch_desc
#define tx_table_s1_t0_action eth_tx_prep
#define tx_table_s2_t0_action eth_tx_commit
#define tx_table_s3_t0_action eth_tx
#define tx_table_s3_t1_action eth_tx_sg
//#define tx_table_s3_t2_action eth_tx_tso
#define tx_table_s7_t0_action eth_tx_completion

#include "../common-p4+/common_txdma.p4"
#include "eth_txdma.p4"

/******************************************************************************
 * Action functions
 *****************************************************************************/

action eth_tx_dummy(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata0)
}

action eth_tx_fetch_desc(
        rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
        p_index0, c_index0, comp_index, ci_fetch,
        enable, color, rsvd1,
        ring_base, ring_size, cq_ring_base, intr_assert_addr,
        spurious_db_cnt, sg_ring_base)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_QSTATE_INTRINSIC(eth_tx_qstate)
    modify_field(eth_tx_qstate.comp_index, comp_index);
    modify_field(eth_tx_qstate.ci_fetch, ci_fetch);
    modify_field(eth_tx_qstate.enable, enable);
    modify_field(eth_tx_qstate.color, color);
    modify_field(eth_tx_qstate.rsvd1, rsvd1);
    modify_field(eth_tx_qstate.ring_base, ring_base);
    modify_field(eth_tx_qstate.ring_size, ring_size);
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base);
    modify_field(eth_tx_qstate.intr_assert_addr, intr_assert_addr);
    modify_field(eth_tx_qstate.spurious_db_cnt, spurious_db_cnt);
    modify_field(eth_tx_qstate.sg_ring_base, sg_ring_base);
}

action eth_tx_prep(
    PARAM_TX_DESC(0),
    PARAM_TX_DESC(1),
    PARAM_TX_DESC(2),
    PARAM_TX_DESC(3)
)
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S
    MODIFY_ETH_TX_TO_S1

    // D
    MODIFY_TX_DESC(0)
    MODIFY_TX_DESC(1)
    MODIFY_TX_DESC(2)
    MODIFY_TX_DESC(3)
}

action eth_tx_commit(
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
    p_index0, c_index0, comp_index, ci_fetch,
    enable, color, rsvd1,
    ring_base, ring_size, cq_ring_base, intr_assert_addr,
    spurious_db_cnt, sg_ring_base)
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S
    MODIFY_ETH_TX_TO_S2

    // D
    modify_field(eth_tx_qstate.pc, pc);
    MODIFY_QSTATE_INTRINSIC(eth_tx_qstate)
    modify_field(eth_tx_qstate.comp_index, comp_index);
    modify_field(eth_tx_qstate.ci_fetch, ci_fetch);
    modify_field(eth_tx_qstate.enable, enable);
    modify_field(eth_tx_qstate.color, color);
    modify_field(eth_tx_qstate.rsvd1, rsvd1);
    modify_field(eth_tx_qstate.ring_base, ring_base);
    modify_field(eth_tx_qstate.ring_size, ring_size);
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base);
    modify_field(eth_tx_qstate.intr_assert_addr, intr_assert_addr);
    modify_field(eth_tx_qstate.spurious_db_cnt, spurious_db_cnt);
    modify_field(eth_tx_qstate.sg_ring_base, sg_ring_base);
}

action eth_tx()
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S
    MODIFY_ETH_TX_TO_S3
}

action eth_tx_sg(
    PARAM_TX_SG_ELEM(0),
    PARAM_TX_SG_ELEM(1),
    PARAM_TX_SG_ELEM(2),
    PARAM_TX_SG_ELEM(3)
)
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T1_S2S
    MODIFY_ETH_TX_TO_S3

    // D
    MODIFY_TX_SG_ELEM(0)
    MODIFY_TX_SG_ELEM(1)
    MODIFY_TX_SG_ELEM(2)
    MODIFY_TX_SG_ELEM(3)
}

action eth_tx_completion()
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S
}
