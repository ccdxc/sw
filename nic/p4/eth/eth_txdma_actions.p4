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
#define tx_table_s3_t2_action eth_tx_tso
#define tx_table_s7_t0_action eth_tx_completion
#define tx_table_s7_t3_action eth_tx_stats

#include "../common-p4+/common_txdma.p4"
#include "eth_txdma.p4"

/******************************************************************************
 * Action functions
 *****************************************************************************/

action eth_tx_fetch_desc(PARAMS_ETH_TX_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_ETH_TX_QSTATE
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

action eth_tx_commit(PARAMS_ETH_TX_QSTATE)
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T0_S2S
    MODIFY_ETH_TX_TO_S2

    // D
    modify_field(eth_tx_qstate.pc, pc);
    MODIFY_ETH_TX_QSTATE
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

action eth_tx_tso(
    PARAM_TX_SG_ELEM(0),
    PARAM_TX_SG_ELEM(1),
    PARAM_TX_SG_ELEM(2),
    PARAM_TX_SG_ELEM(3)
)
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    MODIFY_ETH_TX_T2_S2S
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

action eth_tx_stats()
{
    // K+I
    MODIFY_ETH_TX_GLOBAL
    // MODIFY_ETH_TX_T3_S2S
}