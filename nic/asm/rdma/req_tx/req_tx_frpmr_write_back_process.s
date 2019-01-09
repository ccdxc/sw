#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s6_t2_k k;
struct sqcb0_t d;

#define IN_P                t2_s2s_frpmr_write_back_info
#define IN_TO_S_P           to_s6_frpmr_sqcb_wb_info

#define K_DMA_SRC_ADDR      CAPRI_KEY_RANGE(IN_TO_S_P, frpmr_dma_src_addr_sbit0_ebit7, frpmr_dma_src_addr_sbit48_ebit63)
#define K_WQE_ADDR          CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)
#define K_PT_BASE           CAPRI_KEY_RANGE(IN_P, pt_base_sbit0_ebit7, pt_base_sbit24_ebit31)
#define K_DMA_SIZE          CAPRI_KEY_RANGE(IN_P, dma_size_sbit0_ebit7, dma_size_sbit24_ebit31)
#define K_SPEC_CINDEX       CAPRI_KEY_RANGE(IN_P, spec_cindex_sbit0_ebit7,spec_cindex_sbit8_ebit15)

#define DMA_CMD_BASE        r6

%%

.align
req_tx_frpmr_write_back_process:
    // Pin frpmr_write_back to stage 6
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], bubble_to_next_stage

    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. 
    seq           c1, K_SPEC_CINDEX, SQ_C_INDEX // Branch Delay Slot
    bcf            [!c1], spec_fail
    CAPRI_SET_TABLE_2_VALID(0) // BD-slot
    
    // Generate error-completion on FRPMR failure.
    bbeq           K_GLOBAL_FLAG(_error_disable_qp), 1, error_exit

    /*
     * Prepare FRPMR DMA commands.
     */
    // 1 DMA MR PT-entries from host to HBM.

    // 1.1 PT-Table host-dma-src-addr

    sll          r7, K_DMA_SIZE, CAPRI_LOG_SIZEOF_U64 
    sub          r7, r7, d.frpmr_map_count_completed, CAPRI_LOG_SIZEOF_U64    
    beqi         r7, 0, dma_complete_drop
    sle          c2, r7, FRPMR_DMA_MAX_DATA_SIZE  // BD-Slot
    add.!c2      r7, r0, FRPMR_DMA_MAX_DATA_SIZE


    seq          c1, K_DMA_SIZE, 1 
    add          r5, K_WQE_ADDR, FRPMR_DMA_ADDR_OFFSET
    seq          c3, d.sq_in_hbm, 1 
    bcf          [c1 & c3], dma_from_hbm
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_SRC_ADDR) // BD-Slot
    add          r4, K_DMA_SRC_ADDR, d.frpmr_map_count_completed, CAPRI_LOG_SIZEOF_U64
    DMA_HOST_MEM2MEM_SRC_SETUP_C(DMA_CMD_BASE, r7, r5, r4, c1)
    b            dma_dst_setup
    PT_BASE_ADDR_GET2(r4)


dma_from_hbm:
    DMA_HBM_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r7, r5)
    PT_BASE_ADDR_GET2(r4) //BD-slot

dma_dst_setup:
    // 1.2 PT-Table hbm-dst-addr
    add          r3, r4, K_PT_BASE, CAPRI_LOG_SIZEOF_U64
    add          r3, r3, d.frpmr_map_count_completed, CAPRI_LOG_SIZEOF_U64
    
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_DST_ADDR)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r7, r3)
    // Set EOC if there are more pt-entries to DMA
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE, !c2)
    bcf          [!c2], skip_frpmr_done

    // 2. Set frpmr_dma_done flag in sqcb0_t and set write-fence
    // TODO: Update needs_credits flag in cb1_byte. All other flags can be reset to zero.
    SQCB0_ADDR_GET(r2) // BD-slot
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_SET_FRPMR_DMA_DONE)
    add            r3, r2, FIELD_OFFSET(sqcb0_t, state_flags)
    phvwr          p.sqcb0_state_flags, d.state_flags
    phvwrmi        p.sqcb0_state_flags, (1 << SQCB0_FRPMR_DMA_DONE_BIT_OFFSET), \
                   (1 << SQCB0_FRPMR_DMA_DONE_BIT_OFFSET)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, sqcb0_state_flags, sqcb0_state_flags, r3)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

skip_frpmr_done:
    srl            r3, r7, CAPRI_LOG_SIZEOF_U64
    tbladd         d.frpmr_map_count_completed, r3
    tblwr.e        d.frpmr_in_progress, 1
    tblwr          d.curr_wqe_ptr, K_WQE_ADDR

spec_fail:
dma_complete_drop:
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

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], exit

    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)

exit:
    nop.e
    nop
