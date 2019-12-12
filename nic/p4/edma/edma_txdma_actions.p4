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

#define common_p4plus_stage0_app_header_table_action_dummy edma_fetch_desc
#define tx_table_s0_t0_action edma_fetch_desc
#define tx_table_s1_t0_action edma_process_desc
#define tx_table_s2_t0_action edma_completion

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(APULU)
#include "nic/apollo/p4/apulu_txdma/common_txdma.p4"
#elif defined(POSEIDON)
#include "nic/apollo/p4/poseidon_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "edma_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action edma_fetch_desc(PARAMS_EDMA_QSTATE)
{
    // K+I
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // D
    MODIFY_EDMA_QSTATE
}

action edma_process_desc(opcode, len, src_lif, src_addr, dst_lif, dst_addr, rsvd0)
{
    // K + I
    MODIFY_EDMA_GLOBAL
    MODIFY_EDMA_T0_S2S

    // D
    modify_field(edma_cmd_desc.opcode, opcode);
    modify_field(edma_cmd_desc.len, len);
    modify_field(edma_cmd_desc.src_lif, src_lif);
    modify_field(edma_cmd_desc.src_addr, src_addr);
    modify_field(edma_cmd_desc.dst_lif, dst_lif);
    modify_field(edma_cmd_desc.dst_addr, dst_addr);
    modify_field(edma_cmd_desc.rsvd0, rsvd0);
}

action edma_completion()
{
    // K + I
    MODIFY_EDMA_GLOBAL
    MODIFY_EDMA_T0_S2S

    // D
}
