/*********************************************************************************
 * MPU Timestamp
 *********************************************************************************/

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma_dummy.p4"
#elif defined(ATEHNA)
#include "nic/apollo/p4/athena_txdma/common_txdma_dummy.p4"
#else
#include "nic/p4/common-p4+/common_txdma_dummy.p4"
#endif

/*
 * stage 0
 */
#define tx_table_s0_t0_action   timestamp_start


/*
 * Table names
 */
#define tx_table_s0_t0          s0_tbl

#if defined(ARTEMIS)
#include "nic/apollo/p4/artemis_txdma/common_txdma.p4"
#elif defined(ATHENA)
#include "nic/apollo/p4/athena_txdma/common_txdma.p4"
#else
#include "nic/p4/common-p4+/common_txdma.p4"
#endif

#include "include/ftl_dev_shared_p4.h"

/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata mpu_timestamp_cb_t         timestamp_cb;

/*
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 */

/*****************************************************************************
 *  Initial stage
 *****************************************************************************/
MPU_TIMESTAMP_CB_PRAGMA
action timestamp_start(MPU_TIMESTAMP_CB_DATA)
{
    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    MPU_TIMESTAMP_CB_USE(timestamp_cb)
}

