#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rsq_backtrack_adjust_process_k_t k;
struct rqcb1_t d;

#define DMA_CMD_BASE        r1
#define TMP                 r2
#define DB_ADDR             r3
#define DB_DATA             r4

%%

//RAJA: This code needs to be rewritten based on new backtrack approach
resp_tx_rsq_backtrack_adjust_process:

add_headers:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_START)

    CAPRI_SET_TABLE_0_VALID(0)

    // adjust the RSQ_C_INDEX to the value provided in adjust_rsq_c_index
    PREPARE_DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, RSQ_RING_ID, k.args.adjust_rsq_c_index, DB_ADDR, DB_DATA) 
    phvwr       p.db_data1, DB_DATA.dx
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)

    // adjust the RSQ_BT_C_INDEX to same value as RSQ_BT_P_INDEX so that 
    // backtrack ring is not scheduled anymore.
    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 1);
    PREPARE_DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, RSQ_BT_RING_ID, k.args.rsq_bt_p_index, DB_ADDR, DB_DATA) 
    phvwr       p.db_data2, DB_DATA.dx
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data2, db_data2, DB_ADDR)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

exit:
    nop.e
    nop
