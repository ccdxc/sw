#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

%%

resp_tx_rqcb_process_ext:

    add            r2, r0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    srl            r2, r2, RQCB_ADDR_SHIFT

    phvwrpair       CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, lif), \
                    CAPRI_TXDMA_INTRINSIC_LIF, \
                    CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, qid), \
                    CAPRI_TXDMA_INTRINSIC_QID

    phvwrpair       CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, qtype), \
                    CAPRI_TXDMA_INTRINSIC_QTYPE, \
                    CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, cb_addr), \
                    r2

    // set DMA cmd ptr   (dma cmd idx with in flit is zero)
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID, 0)

    nop.e
    nop
