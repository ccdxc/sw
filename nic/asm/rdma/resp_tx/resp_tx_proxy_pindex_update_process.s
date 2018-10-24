#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s1_t0_k k;

#define IN_P            t0_s2s_rqcb_to_pi_update_info
#define K_RQ_P_INDEX    CAPRI_KEY_RANGE(IN_P, rq_p_index_sbit0_ebit7, rq_p_index_sbit8_ebit15)
#define DMA_CMD_BASE    r1

%%

resp_tx_proxy_pindex_update_process:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, \
                            RESP_TX_DMA_CMD_RDMA_PROXY_PI_UPDATE_FEEDBACK)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4_intr_global, rdma_feedback)
    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    phvwr          p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, Q_TYPE_RDMA_RQ, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, RESP_RX_FLAG_ERR_DIS_QP
    phvwri         p.p4_to_p4plus.table0_valid, 1   //Exit Slot

    phvwrpair      p.rdma_feedback.feedback_type, RDMA_RQ_PROXY_PI_FEEDBACK, \
                   p.rdma_feedback.rq_proxy_pi.proxy_pindex, K_RQ_P_INDEX

    DMA_SET_END_OF_PKT_END_OF_CMDS_E(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot
