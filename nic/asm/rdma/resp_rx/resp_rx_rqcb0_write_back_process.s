#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqcb0_write_back_process_k_t k;
struct rqcb0_t d;

#define INFO_WBCB1_T struct resp_rx_rqcb1_write_back_info_t

#define DMA_CMD_BASE r1
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define RQCB1_ADDR r6
#define T2_ARG r7
#define T2_K r7
#define RAW_TABLE_PC r2

%%
    .param  resp_rx_rqcb1_write_back_process

.align
resp_rx_rqcb0_write_back_process:
    //mfspr           r1, spr_mpuid
    //seq             c1, r1[6:2], STAGE_3
    //bcf             [!c1], exit
    
    seq             c1, k.to_stage.s3.wb0.my_token_id, d.nxt_to_go_token_id
    bcf             [!c1], recirc
    
    tblwr           d.in_progress, k.args.in_progress
    seq             c1, k.args.incr_nxt_to_go_token_id, 1

    seq             c2, k.args.incr_c_index, 1
    bcf             [!c2], incr_c_index_exit
    tbladd.c1       d.nxt_to_go_token_id, 1     //BD slot
    tblmincri.c2    PROXY_RQ_C_INDEX, d.log_num_wqes, 1

incr_c_index_exit:

    // Skip ACK generation for UD
    bbeq        k.global.flags.resp_rx._ud, 1, invoke_wb1
    RQCB1_ADDR_GET(RQCB1_ADDR)      //BD Slot

    crestore        [c7, c6, c5, c4, c3, c2], k.global.flags, (RESP_RX_FLAG_ACK_REQ | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_ONLY | RESP_RX_FLAG_LAST)
    //c7: ack c6: cswap c5: fna c4: read c3: only c2: last

    // RQCB0 writeback is called for SEND/WRITE/ATOMIC as well.
    // also this stage has very few instructions.
    // Since it is a common routine and also has less instructions, 
    // enqueuing ACK DMA instructions in this stage.

    //if ((RESP_RX_FLAGS_IS_SET(last)) || (RESP_RX_FLAGS_IS_SET(only))) {
    //  rx_post_ack_info_to_txdma(phv_p, dma_cmd_index, RESP_RX_GET_RQCB1_ADDR());
    //

    setcf       c1, [c6 | c5 | c4 | c3 | c2]

    // TODO: for now, generate ack only when it is a last/only packet AND 
    // ack req bit is also set. This will help in controlling DOL test cases
    // to generate ACK only when needed. Remove ACK_REQ bit check later ?
    bcf         [!c7 & !c1], invoke_wb1
    CAPRI_GET_TABLE_2_ARG(resp_rx_phv_t, T2_ARG)    //BD Slot

    setcf       c1, [c6 | c5 | c4]
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, !c1)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, c1)

    // prepare for acknowledgement
    RESP_RX_POST_ACK_INFO_TO_TXDMA_NO_DB(DMA_CMD_BASE, RQCB1_ADDR, TMP)
    // for read/atomic operations, do not ring doorbell
    bcf         [c1], invoke_wb1
    nop         //BD Slot
    RESP_RX_POST_ACK_INFO_TO_TXDMA_DB_ONLY(DMA_CMD_BASE,
                                   k.global.lif,
                                   k.global.qtype,
                                   k.global.qid,
                                   DB_ADDR, DB_DATA)

invoke_wb1:
    // invoke rqcb1 write back from rqcb0 write back
    CAPRI_SET_FIELD_RANGE(T2_ARG, INFO_WBCB1_T, current_sge_offset, curr_wqe_ptr, k.{args.current_sge_offset...args.curr_wqe_ptr})
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, T2_K)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqcb1_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(T2_K, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_ADDR)

    nop.e
    nop

recirc:
    phvwr   p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_DONE
    // invalidate table ?
    CAPRI_SET_TABLE_2_VALID(0)

exit:
    nop.e
    nop
