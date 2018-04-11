#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s5_t1_k k;
struct rqcb0_t d;

#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4
#define DMA_CMD_BASE        r1

#define IN_P t1_s2s_rqcb0_write_back_info
#define IN_TO_S_P to_s5_rqcb1_wb_info

%%

resp_tx_rqcb0_write_back_process:

    bbeq       CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, dcqcn_rl_failure
    CAPRI_SET_TABLE_1_VALID(0) // BD slot


add_headers_common:

    // intrinsic
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type)

#ifndef GFT
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_TXDMA_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)
    phvwr          p.common.p4_txdma_intr_qid, K_GLOBAL_QID
    RQCB0_ADDR_GET(r1)
    phvwrpair       p.common.p4_txdma_intr_qstate_addr, r1, p.common.p4_txdma_intr_qtype, K_GLOBAL_QTYPE

    // common-p4+
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_COMMON_P4PLUS)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4);
    phvwrpair       P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags

    bbeq           CAPRI_KEY_FIELD(IN_TO_S_P, ack_nak_process), 1, add_ack_header
    phvwr          P4PLUS_TO_P4_VLAN_TAG, 0 //BD-slot

rsq_write_back:
    tblwr       d.read_rsp_in_progress, CAPRI_KEY_FIELD(IN_P, read_rsp_in_progress)
    seq         c1, CAPRI_KEY_FIELD(IN_P, read_rsp_in_progress), 1
    cmov        CURR_READ_RSP_PSN, c1, CAPRI_KEY_FIELD(IN_P, curr_read_rsp_psn), 0
    mincr.c1    CURR_READ_RSP_PSN, 24, 1
    tblwr       d.curr_read_rsp_psn, CURR_READ_RSP_PSN

    // Update RSQ_C_INDEX to NEW_RSQ_C_INDEX only when read rsp NOT in progress (!c1)
    //TBD: do we need hx somewhere ?
    tblwr.!c1   RSQ_C_INDEX, CAPRI_KEY_FIELD(IN_TO_S_P, new_c_index)
    tblwr       d.read_rsp_lock, 0

    nop.e
    nop

    
add_ack_header:
    add         r2, RDMA_PKT_OPC_ACK, d.serv_type, BTH_OPC_SVC_SHIFT
    phvwrpair   p.bth.opcode, r2, p.bth.dst_qp, d.dst_qp

    // phv_p->bth.pkey = 0xffff
    phvwr       p.bth.pkey, 0xffff  
    
    // header_template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    sll         r5, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, d.header_template_size, r5)

    // BTH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    // AETH
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

    // No PAD for ack packet
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    nop.e
    nop
    
dcqcn_rl_failure:
    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, ack_nak_process), 1, exit
    nop
    // release read_rsp_lock only in rsq path.
    tblwr           d.read_rsp_lock, 0   //TODO: For now avoid this, as moved this to RQCB0

exit:
    nop.e
    nop
