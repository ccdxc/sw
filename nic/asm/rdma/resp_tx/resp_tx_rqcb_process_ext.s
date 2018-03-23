#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

%%

resp_tx_rqcb_process_ext:

    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) //BD Slot

    add            r2, r0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    srl            r2, r2, RQCB_ADDR_SHIFT

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, r2)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    // set DMA cmd ptr   (dma cmd idx with in flit is zero)
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID, 0)

    nop.e
    nop
