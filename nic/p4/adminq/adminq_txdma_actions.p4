/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma_dummy.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma_dummy.p4"
#elif defined(ATHENA)
#include "nic/apollo/p4/athena_txdma/common_txdma_dummy.p4"
#else
#include "nic/p4/common-p4+/common_txdma_dummy.p4"
#endif

#define common_p4plus_stage0_app_header_table_action_dummy adminq_fetch_desc
#define tx_table_s0_t0_action adminq_fetch_desc
#define tx_table_s1_t0_action adminq_process_desc
#define tx_table_s2_t0_action adminq_post_nicmgr
#define tx_table_s3_t0_action adminq_commit

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma.p4"
#elif defined(ATHENA)
#include "nic/apollo/p4/athena_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "adminq_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action adminq_fetch_desc(PARAMS_ADMINQ_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_ADMINQ_QSTATE
}

action adminq_process_desc(opcode, data)
{
    // K + I
    MODIFY_ADMINQ_GLOBAL
    MODIFY_ADMINQ_T0_S2S
    MODIFY_ADMINQ_TO_S1

    // D
    modify_field(adminq_cmd_desc.opcode, opcode);
    modify_field(adminq_cmd_desc.data, data);
}

action adminq_post_nicmgr(PARAMS_NICMGR_QSTATE)
{
    // K + I
    MODIFY_ADMINQ_GLOBAL
    MODIFY_ADMINQ_T0_S2S
    MODIFY_ADMINQ_TO_S2

    // D
    modify_field(nicmgr_qstate.pc, pc);
    MODIFY_NICMGR_QSTATE
}

action adminq_commit(PARAMS_ADMINQ_QSTATE)
{
    // K + I
    MODIFY_ADMINQ_GLOBAL
    MODIFY_ADMINQ_T0_S2S
    MODIFY_ADMINQ_TO_S3

    // D
    modify_field(adminq_qstate.pc, pc);
    MODIFY_ADMINQ_QSTATE
}
