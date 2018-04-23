#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s3_t3_k k;
struct sqcb1_t d;

#define IN_P t3_s2s_sqcb1_write_back_info

#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit15, cur_sge_offset_sbit16_ebit31)
#define K_E_RSP_PSN CAPRI_KEY_FIELD(IN_P, e_rsp_psn)
#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_P, rexmit_psn_sbit0_ebit2, rexmit_psn_sbit19_ebit23)
#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit2, msn_sbit19_ebit23)

%%

.align
req_rx_sqcb1_write_back_process:
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_3
    bcf            [!c1], bubble_to_next_stage
    nop            // Branch Delay Slot

    tblwr          d.rrq_in_progress, CAPRI_KEY_FIELD(IN_P, rrq_in_progress)
    tblwr          d.rrqwqe_cur_sge_id, K_CUR_SGE_ID
    tblwr          d.rrqwqe_cur_sge_offset, K_CUR_SGE_OFFSET
    tblwr          d.e_rsp_psn, K_E_RSP_PSN
    tblwr          d.rexmit_psn, K_REXMIT_PSN
    tblwr          d.msn, K_MSN
    seq            c1, CAPRI_KEY_FIELD(IN_P, incr_nxt_to_go_token_id), 1
    tblmincri.c1   d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1
    seq            c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1
    tblmincri.c1   RRQ_C_INDEX, d.log_rrq_size, 1 

    bbne           CAPRI_KEY_FIELD(IN_P, post_bktrack), 1, end
    nop            // Branch Delay Slot

post_bktrack_ring:
     // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_BKTRACK_DB)

    // dma_cmd - bktrack_ring db data
    PREPARE_DOORBELL_INC_PINDEX(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, SQ_BKTRACK_RING_ID, r1, r2)
    phvwr          p.db_data2, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data2, db_data2, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)
    seq            c1, CAPRI_KEY_FIELD(IN_P, dma_cmd_eop), 1
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, r6, c1)


end:
     CAPRI_SET_TABLE_3_VALID(0)

     nop.e
     nop

bubble_to_next_stage:
     seq           c1, r1[4:2], STAGE_2
     bcf           [!c1], exit
     SQCB1_ADDR_GET(r1)
     CAPRI_GET_TABLE_3_K(req_rx_phv_t, r7)
     CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
     nop.e
     nop

