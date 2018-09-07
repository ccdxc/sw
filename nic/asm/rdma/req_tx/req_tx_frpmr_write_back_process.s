#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb0_t d;

#define IN_P                t2_s2s_frpmr_write_back_info
#define IN_TO_S_P           to_s5_frpmr_sqcb_wb_info

#define K_DMA_SRC_ADDR      CAPRI_KEY_RANGE(IN_TO_S_P, frpmr_dma_src_addr_sbit0_ebit7, frpmr_dma_src_addr_sbit48_ebit63)
#define K_L_KEY             CAPRI_KEY_RANGE(IN_TO_S_P, l_key_sbit0_ebit7, l_key_sbit24_ebit31)
#define K_WQE_ADDR          CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)
#define K_PT_BASE           CAPRI_KEY_RANGE(IN_P, pt_base_sbit0_ebit7, pt_base_sbit24_ebit31)
#define K_DMA_SIZE          CAPRI_KEY_RANGE(IN_P, dma_size_sbit0_ebit7, dma_size_sbit24_ebit31)
#define K_SPEC_CINDEX       CAPRI_KEY_RANGE(IN_P, spec_cindex_sbit0_ebit7,spec_cindex_sbit8_ebit15)

#define DMA_CMD_BASE        r6

%%

.align
req_tx_frpmr_write_back_process:
    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. 
    seq           c1, K_SPEC_CINDEX, SQ_C_INDEX
    bcf            [!c1], spec_fail
    CAPRI_SET_TABLE_2_VALID(0) // BD-slot
    
    // Generate error-completion on FRPMR failure.
    bbeq           K_GLOBAL_FLAG(_error_disable_qp), 1, error_exit

    /*
     * Prepare FRPMR DMA commands.
     */

    // 1 DMA MR PT-entries from host to HBM.

    // 1.1 PT-Table host-dma-src-addr
    sll          r7, K_DMA_SIZE, CAPRI_LOG_SIZEOF_U64 //BD-Slot
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_SRC_ADDR)
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r7, K_DMA_SRC_ADDR)

    // 1.2 PT-Table hbm-dst-addr
    PT_BASE_ADDR_GET2(r4) 
    add          r3, r4, K_PT_BASE, CAPRI_LOG_SIZEOF_U64
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_DST_ADDR)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r7, r3)

    // 2. Set frpmr_in_progress flag in sqcb0_t and set write-fence
    // TODO: Update needs_credits flag in cb1_byte. All other flags can be reset to zero.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_SET_FRPMR_IN_PROGRESS)
    SQCB0_ADDR_GET(r2)
    add            r3, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    phvwr          p.frpmr_cb1_byte, (1 << SQCB0_FRPMR_IN_PROGRESS)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, frpmr_cb1_byte, frpmr_cb1_byte, r3)
    DMA_SET_WR_FENCE_END_OF_CMDS_E(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    // Update curr_wqe_ptr for second pass.
    tblwr          d.curr_wqe_ptr, K_WQE_ADDR //BD-slot

spec_fail:
    phvwr.e   p.common.p4_intr_global_drop, 1
    nop

error_exit:
    /*
     *  TODO: Incrementing cindex to satisfy model. Ideally, on error disabling we should just exit and be
     *  in the same state which caused the error.
     */
    tblmincri      SQ_C_INDEX, d.log_num_wqes, 1

    // Send Error-Completion feedback PHV to req-rx.
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_ERR_FEEDBACK) 
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
    DMA_SET_END_OF_PKT_END_OF_CMDS_E(DMA_CMD_PHV2PKT_T, r6)
    phvwri         p.p4_to_p4plus.table0_valid, 1 // BD-slot
