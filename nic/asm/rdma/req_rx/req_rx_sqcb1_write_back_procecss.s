#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_sqcb1_write_back_process_k_t k;
struct sqcb1_t d;

%%

.align
req_rx_sqcb1_write_back_process:
    tblwr          d.in_progress, k.args.in_progress
    tblwr          d.rrqwqe_cur_sge_id, k.args.cur_sge_id
    tblwr          d.rrqwqe_cur_sge_offset, k.args.cur_sge_offset
    tblwr          d.e_rsp_psn, k.args.e_rsp_psn
    seq            c1, k.args.incr_nxt_to_go_token_id, 1
    tblmincri.c1   d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1

    seq            c1, k.args.post_bktrack, 1
    bcf            [!c1], end
    nop            // Branch Delay Slot

post_bktrack_ring:
     // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_I_BASE_GET(r6, r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_BKTRACK_PSN_DMA_CMD)

    // dma_cmd - bktrack_ring db data
    PREPARE_DOORBELL_INC_PINDEX(k.global.lif, k.global.qtype, k.global.qid, SQ_BACKTRACK_RING_ID, r1, r2)
    phvwr          p.db_data2, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data2, db_data2, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)


end:
     add           r1, k.args.tbl_id, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop

