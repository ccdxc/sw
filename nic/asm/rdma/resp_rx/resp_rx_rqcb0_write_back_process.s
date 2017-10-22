#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqcb0_write_back_process_k_t k;
struct rqcb0_t d;

#define DMA_CMD_BASE r1
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define RQCB1_ADDR r6

%%

.align
resp_rx_rqcb0_write_back_process:
    
    crestore        [c7, c6, c5], k.global.flags, (RESP_RX_FLAG_ACK_REQ | RESP_RX_FLAG_ONLY | RESP_RX_FLAG_LAST)

    tblwr           d.in_progress, k.args.in_progress
    seq             c1, k.args.incr_nxt_to_go_token_id, 1

    seq             c2, k.args.incr_c_index, 1
    bcf             [!c2], incr_c_index_exit
    tbladd.c1       d.nxt_to_go_token_id, 1     //BD slot
    tblmincri.c2    PROXY_RQ_C_INDEX, d.log_num_wqes, 1

incr_c_index_exit:

    //assumption is that write back is called with table 2
    seq         c3, k.args.do_not_invalidate_tbl, 1
    CAPRI_SET_TABLE_2_VALID_C(!c3, 0)


    // RQCB0 writeback is called for SEND/WRITE/ATOMIC as well.
    // also this stage has very few instructions.
    // Since it is a common routine and also has less instructions, 
    // enqueuing ACK DMA instructions in this stage.

    //if ((RESP_RX_FLAGS_IS_SET(last)) || (RESP_RX_FLAGS_IS_SET(only))) {
    //  rx_post_ack_info_to_txdma(phv_p, dma_cmd_index, RESP_RX_GET_RQCB1_ADDR());
    //

    setcf       c4, [c6 | c5]

    // TODO: for now, generate ack only when it is a last/only packet AND 
    // ack req bit is also set. This will help in controlling DOL test cases
    // to generate ACK only when needed. Remove ACK_REQ bit check later ?
    bcf         [!c7 | !c4], exit
    RQCB1_ADDR_GET(RQCB1_ADDR)      //BD Slot
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)

    // prepare for acknowledgement
    RESP_RX_POST_ACK_INFO_TO_TXDMA(DMA_CMD_BASE, RQCB1_ADDR, TMP, \
                                   k.global.lif,
                                   k.global.qtype,
                                   k.global.qid,
                                   DB_ADDR, DB_DATA)
exit:
    nop.e
    nop
