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
    seq           c1, K_GLOBAL_SPEC_CINDEX, SQ_C_INDEX // Branch Delay Slot
    bcf            [!c1], spec_fail
    CAPRI_SET_TABLE_2_VALID(0) // BD-slot
    /*
     * Prepare FRPMR DMA commands.
     */
    // 1 DMA MR PT-entries from host to HBM.

    // 1.1 PT-Table host-dma-src-addr
    sll          r7, K_DMA_SIZE, CAPRI_LOG_SIZEOF_U64

    beqi         r7, 1 << CAPRI_LOG_SIZEOF_U64, skip_dma_pt
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_SRC_ADDR)  //BD Slot
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r7, K_DMA_SRC_ADDR)

    // 1.2 PT-Table hbm-dst-addr
    PT_BASE_ADDR_GET2(r4)
    add          r3, r4, K_PT_BASE, CAPRI_LOG_SIZEOF_U64 
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRPMR_PT_TABLE_DST_ADDR)
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, r7, r3)

skip_dma_pt: 
    // 2. Set frpmr_in_progress flag in sqcb0_t and set write-fence
    // TODO: Update needs_credits flag in cb1_byte. All other flags can be reset to zero.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_SET_FRPMR_IN_PROGRESS)
    SQCB0_ADDR_GET(r2)
    add            r3, r2, FIELD_OFFSET(sqcb0_t, config_state_flags)
    phvwr          p.sqcb0_state_flags, d.config_state_flags
    phvwrmi        p.sqcb0_state_flags, (1 << SQCB0_FRPMR_IN_PROGRESS_BIT_OFFSET), \
                   (1 << SQCB0_FRPMR_IN_PROGRESS_BIT_OFFSET)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, sqcb0_state_flags, sqcb0_state_flags, r3)
    DMA_SET_WR_FENCE_END_OF_CMDS_E(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    // Update curr_wqe_ptr for second pass.
    tblwr          d.curr_wqe_ptr, K_WQE_ADDR //BD-slot

spec_fail:
    phvwr.e   p.common.p4_intr_global_drop, 1
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], exit

    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)

exit:
    nop.e
    nop
