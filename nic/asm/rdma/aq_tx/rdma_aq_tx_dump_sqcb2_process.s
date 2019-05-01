#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb2_t d;
struct aq_tx_s5_t2_k k;

#define IN_TO_S_P to_s5_info

%%
    .param      rdma_aq_tx_feedback_process

.align
rdma_aq_tx_dump_sqcb2_process:
    // r1: mpu stage number
    mfspr       r1, spr_mpuid
    or          r1, r0, r1[4:2]
    bnei        r1, STAGE_5, bubble_to_next_stage

    // r2: qp must be RC and have a valid header template
    add         r2, r0, d.service // BD Slot
    bnei        r2, RDMA_SERV_TYPE_RC, report_bad_type
    add         r2, r0, d.header_template_size // BD Slot
    beq         r2, r0, report_bad_state

    // r2: dcqcn cb addr, just to be careful guard for NULL
    add         r2, r0, d.header_template_addr, HDR_TEMP_ADDR_SHIFT // BD Slot
    beq         r2, r0, report_bad_state
    addi        r2, r2, AH_ENTRY_T_SIZE_BYTES // BD Slot

    // setup dma src dcqcn cb (dest already setup in wqe process)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_STATS_DUMP_1)
    DMA_HBM_MEM2MEM_SRC_SETUP(r6, DCQCN_CB_T_SIZE_BYTES, r2)

exit:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

report_bad_type:
    b           exit
    phvwrpair   p.rdma_feedback.aq_completion.status, AQ_CQ_STATUS_BAD_TYPE, p.rdma_feedback.aq_completion.error, 1 //BD Slot

report_bad_state:
    b           exit
    phvwrpair   p.rdma_feedback.aq_completion.status, AQ_CQ_STATUS_BAD_STATE, p.rdma_feedback.aq_completion.error, 1 //BD Slot

bubble_to_next_stage:
    bnei        r1, STAGE_4, bubble_exit

    CAPRI_GET_TABLE_2_K(aq_tx_phv_t, r7) // BD Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS)

bubble_exit:
    nop.e
    nop
