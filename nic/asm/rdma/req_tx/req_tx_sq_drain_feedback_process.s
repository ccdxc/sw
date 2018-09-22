#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t3_k k;
struct sqcb2_t d;

#define TO_S7_STATS_P         to_s7_stats_info

%%
    .param req_tx_stats_process

.align
req_tx_sq_drain_feedback_process:
    // Pin sq_drain_feedback to stage 5, same as add_headers_process so that
    // sqcb2 can be locked and latest ssn/tx_psn can be fetched
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_5
    bcf            [!c1], bubble_to_next_stage

    // send sq_drain feedback msg to RxDMA so that it can mark drain completion
    // of non-packet generating wqes. Upon receiving acks for all outstanding
    // responses it can complete drain for all packet generating wqes as well
    // and raise affiliated async event
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_FEEDBACK) // Branch Delay Slot

    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)

    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    SQCB0_ADDR_GET(r1)
    phvwrpair      p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, K_GLOBAL_QTYPE, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri         p.p4_to_p4plus.table0_valid, 1

    // Fill sq_drain feedback msg with txdma version of ssn and tx_psn. Compare
    // ssn with msn in RXDMA to identify completion of all oustanding responses
    phvwr          p.rdma_feedback.feedback_type, RDMA_SQ_DRAIN_FEEDBACK
    phvwrpair      p.rdma_feedback.sq_drain.tx_psn, d.tx_psn, \
                   p.rdma_feedback.sq_drain.ssn, d.ssn

    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

    phvwr          CAPRI_PHV_FIELD(TO_S7_STATS_P, sq_drain), 1

    //invoke stats_process to account for sq_drain event
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_stats_process, r0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], exit
    SQCB2_ADDR_GET(r1) // Branch Delay Slot

    // Invoke sq_drain feedback in the same stage & table as add_headers_process so
    // that latest ssn/tx_psn can be fetched from SQCB2
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    nop.e
    nop
