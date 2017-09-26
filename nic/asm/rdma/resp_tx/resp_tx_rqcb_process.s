#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define DMA_CMD_BASE r1
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define NEW_RSQ_C_INDEX r6

%%

resp_tx_rqcb_process:

    // set DMA cmd ptr
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID)

    seq         c1, RSQ_C_INDEX, RSQ_P_INDEX
    tblwr.!c1   RSQ_C_INDEX, RSQ_P_INDEX
    bcf         [c1], exit
    nop

    // TODO: need to write actual code.
    // for now, prevent the model loop by copying p_index to c_index

    add     NEW_RSQ_C_INDEX, r0, RSQ_P_INDEX

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_CINDEX, DB_SCHED_WR_EVAL_RING, CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, DB_ADDR)
    CAPRI_SETUP_DB_DATA(CAPRI_TXDMA_INTRINSIC_QID, RSQ_RING_ID, NEW_RSQ_C_INDEX, DB_DATA)

    phvwr   p.db_data, DB_DATA.wx

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, TMP, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_RSQ_DB)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, db_data, db_data, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    

exit:
    nop.e
    nop
