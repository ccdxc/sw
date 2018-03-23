#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t3_k k;
struct key_entry_aligned_t d;

#define DMA_CMD_BASE r1
#define GLOBAL_FLAGS r2
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define RQCB2_ADDR r6
#define KEY_P   r7

%%

    .param  resp_rx_stats_process

.align
resp_rx_inv_rkey_process:
    // handle ack generation
    // Since this program is always called and also has 
    // less instructions, enqueuing ACK DMA instructions in this stage.

    add             GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    crestore        [c7, c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_RX_FLAG_UD | RESP_RX_FLAG_ACK_REQ | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_ERR_DIS_QP)
    //c7: ud c6: ack c5: inv_rkey c4: cswap c3: fna c2: read c1: err_dis_qp
    
    // skip ack generation for UD
    // skip ack generation if qp is in err_disabled state as
    // lkey_process in prev stage would have generated NAK
    bcf             [c7 | c1], check_invalidate
    RQCB2_ADDR_GET(RQCB2_ADDR)      //BD Slot

    setcf       c1, [c4 | c3 | c2]
    // c1 : atomic_or_read

    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, !c1)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, c1)

    // prepare for acknowledgement
    RESP_RX_POST_ACK_INFO_TO_TXDMA_NO_DB(DMA_CMD_BASE, RQCB2_ADDR, TMP)
    // for read/atomic operations, do not ring doorbell
    // also if ack req bit is not set, even then do not ring doorbell
    bcf         [c1 | !c6], check_invalidate
    RESP_RX_POST_ACK_INFO_TO_TXDMA_DB_ONLY(DMA_CMD_BASE,
                                   K_GLOBAL_LIF,
                                   K_GLOBAL_QTYPE,
                                   K_GLOBAL_QID,
                                   DB_ADDR, DB_DATA)    //BD Slot
check_invalidate:

    bcf         [!c5], invoke_stats

    // it is an error to invalidate an MR not eligible for invalidation
    // (Disabled for now till MR objects in DOL can have this
    //  configuration)
    // it is an error to invalidate an MR in INVALID state
    //CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, flags)
    //ARE_ALL_FLAGS_SET_B(c1, r1, MR_FLAG_INV_EN)
    //bcf         [!c1], error_completion

    seq         c1, d.state, KEY_STATE_INVALID //BD slot (after uncomment of above code)
    bcf         [c1], error_completion
    nop    //BD slot
    
    // update the state to FREE
    tblwr       d.state, KEY_STATE_FREE

invoke_stats:
    
    // invoke stats always !!
    // invoke it as mpu_only program and it stats_process keeps 
    // bubbling down and handles actual stats in stage 7.
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_stats_process, r0)
    
    nop.e
    nop

error_completion:
    //TODO

    nop.e
    nop
