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

#define common_p4plus_stage0_app_header_table_action_dummy notify_fetch_desc
#define tx_table_s0_t0_action notify_fetch_desc
#define tx_table_s1_t0_action notify_process_desc
#define tx_table_s2_t0_action notify_completion

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma.p4"
#elif defined(POSEIDON)
#include "nic/apollo/p4/poseidon_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "notify_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action notify_fetch_desc(PARAMS_NOTIFY_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_NOTIFY_QSTATE
}

action notify_process_desc(data)
{
    // K + I
    MODIFY_NOTIFY_GLOBAL
    MODIFY_NOTIFY_T0_S2S

    // D
    modify_field(notify_event_desc.data, data);
}

action notify_completion()
{
    // K + I
    MODIFY_NOTIFY_GLOBAL
    MODIFY_NOTIFY_T0_S2S

    // D
}
