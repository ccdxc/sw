/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma_dummy.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma_dummy.p4"
#elif defined(POSEIDON)
#include "nic/apollo/p4/poseidon_txdma/common_txdma_dummy.p4"
#else
#include "nic/p4/common-p4+/common_txdma_dummy.p4"
#endif

#define common_p4plus_stage0_app_header_table_action_dummy nicmgr_fetch_desc
#define tx_table_s0_t0_action nicmgr_fetch_desc
#define tx_table_s0_t0_action1 nicmgr_drop
#define tx_table_s1_t0_action nicmgr_process_desc_hdr
#define tx_table_s1_t1_action nicmgr_process_desc_data
#define tx_table_s2_t0_action nicmgr_post_adminq
#define tx_table_s3_t0_action nicmgr_commit

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma.p4"
#elif defined(POSEIDON)
#include "nic/apollo/p4/poseidon_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "nicmgr_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action nicmgr_fetch_desc(PARAMS_NICMGR_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_NICMGR_QSTATE
}

action nicmgr_drop(PARAMS_NICMGR_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_NICMGR_QSTATE
}

action nicmgr_process_desc_hdr(lif, qtype, qid, comp_index, adminq_qstate_addr)
{
    // K + I
    MODIFY_NICMGR_GLOBAL
    MODIFY_NICMGR_TO_S1
    MODIFY_NICMGR_T0_S2S

    // D
    modify_field(nicmgr_resp_desc.lif, lif);
    modify_field(nicmgr_resp_desc.qtype, qtype);
    modify_field(nicmgr_resp_desc.qid, qid);
    modify_field(nicmgr_resp_desc.comp_index, comp_index);
    modify_field(nicmgr_resp_desc.adminq_qstate_addr, adminq_qstate_addr);
}

action nicmgr_process_desc_data(adminq_comp_desc)
{
    // K + I
    MODIFY_NICMGR_GLOBAL
    MODIFY_NICMGR_TO_S1
    MODIFY_NICMGR_T1_S2S

    // D
    modify_field(nicmgr_resp_desc.adminq_comp_desc, adminq_comp_desc);
}

action nicmgr_post_adminq(PARAMS_ADMINQ_QSTATE)
{
    // K + I
    MODIFY_NICMGR_GLOBAL
    MODIFY_NICMGR_TO_S2
    MODIFY_NICMGR_T0_S2S

    // D
    modify_field(adminq_qstate.pc, pc);
    MODIFY_ADMINQ_QSTATE
}

action nicmgr_commit(PARAMS_NICMGR_QSTATE)
{
    // K + I
    MODIFY_NICMGR_GLOBAL
    MODIFY_NICMGR_TO_S3
    MODIFY_NICMGR_T0_S2S

    // D
    modify_field(nicmgr_qstate.pc, pc);
    MODIFY_NICMGR_QSTATE
}
