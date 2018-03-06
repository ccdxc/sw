#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rsq_backtrack_adjust_process_k_t k;
struct rqcb0_t d;

#define NEW_RSQ_PINDEX  r1
#define DB_ADDR         r2
#define DB_DATA         r3
#define TMP             r4
#define DMA_CMD_BASE    r5


%%

.align
resp_rx_rsq_backtrack_adjust_process:
    // TBD: migrate to new logic

//    // increment nxt_to_go_token_id to release control to others
//    tbladd      d.nxt_to_go_token_id, 1
//    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_START)
//
//check_adjust_c_index:
//    seq         c1, k.args.adjust_c_index, 1
//    bcf         [!c1], check_adjust_p_index
//    seq         c2, k.args.adjust_p_index, 1        //BD Slot
//
//    // quiesce and adjust_rsq_c_index_in_progress are used by same RxDMA pipeline,
//    // hence keep them as tblwr
//    tblwr       d.adjust_rsq_c_index_in_progress, 1
//
//    // store current value of pindex as rsq_pindex_prime so that we know
//    // the backtrack end boundary. once rsq_pindex is retracted to index+1 below,
//    // then onwards we expect duplicates to come inorder and lock-step manner.
//    // only when rsq_pindex reaches to rsq_pindex_prime, we resume the normal
//    // activity
//    add         NEW_RSQ_PINDEX, r0, k.args.index
//    mincr       NEW_RSQ_PINDEX, d.log_rsq_size, 1
//    
//    // special case: if index+1 is already equal to rsq_p_index, there is no
//    // need to put into quiesce mode
//    seq         c3, NEW_RSQ_PINDEX, RSQ_P_INDEX
//    tblwr.!c3   d.rsq_quiesce, 1
//    tblwr.!c3   d.rsq_pindex_prime, RSQ_P_INDEX
//
//    // use DMA instructions to update adjust_rsq_c_index and ringing backtrack
//    // doorbell. Put a fence on backtrack doorbell so that it is rung only
//    // after adjust_rsq_c_index is committed to HBM
//
//    // dma_cmd: populate adjust_rsq_c_index
//    phvwr       p.adjust_rsq_c_index, k.args.index
//    RQCB0_ADDR_GET(r6)
//    add         r6, r6, FIELD_OFFSET(rqcb0_t, adjust_rsq_c_index)
//    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, adjust_rsq_c_index, adjust_rsq_c_index, r6)
//
//
//    // dma_cmd: ring backtrack doorbell to adjust cindex
//    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 1)
//    PREPARE_DOORBELL_INC_PINDEX(k.global.lif, k.global.qtype, k.global.qid, \
//                                RSQ_BT_RING_ID, DB_ADDR, DB_DATA)
//    phvwr       p.db_data1, DB_DATA.dx
//    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
//    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
//
//
//    // dma_cmd: ring rsq doorbell to change pindex
//    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 1)
//    PREPARE_DOORBELL_WRITE_PINDEX(k.global.lif, k.global.qtype, k.global.qid, \
//                                  RSQ_RING_ID, NEW_RSQ_PINDEX, DB_ADDR, DB_DATA)
//    phvwr       p.db_data2, DB_DATA.dx
//    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data2, db_data2, DB_ADDR)
//    DMA_SET_WR_FENCE_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)  //putting fence_fence for safe side.
//
//
//    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
//    b           exit
//    CAPRI_SET_TABLE_0_VALID(0)                      //BD Slot
//
//
//check_adjust_p_index:
//    bcf         [!c2], exit
//    CAPRI_SET_TABLE_0_VALID(0)                      //BD Slot
//    add         NEW_RSQ_PINDEX, r0, RSQ_P_INDEX
//    seq         c3, k.args.index, NEW_RSQ_PINDEX
//    seq         c4, d.rsq_quiesce, 1
//    bcf         [!c3 | !c4], exit
//    // adjust the p_index only when we are in quiesce
//    // mode and the found index is matching current
//    // pindex.
//    mincr       NEW_RSQ_PINDEX, d.log_rsq_size, 1   //BD Slot
//
//    PREPARE_DOORBELL_WRITE_PINDEX(k.global.lif, k.global.qtype, k.global.qid, \
//                                  RSQ_RING_ID, NEW_RSQ_PINDEX, DB_ADDR, DB_DATA)
//    phvwr       p.db_data1, DB_DATA.dx
//    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
//    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
//
//    //if p_index has caught up with p_index_prime, turn off quiesce
//    seq         c5, NEW_RSQ_PINDEX, d.rsq_pindex_prime
//    tblwr.c5    d.rsq_quiesce, 0

exit:
    nop.e
    nop
