#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct rdma_stage0_table_k k;

#define SQCB_TO_PT_T struct req_tx_sqcb_to_pt_info_t
#define SQCB_TO_WQE_T struct req_tx_sqcb_to_wqe_info_t
#define WQE_TO_SGE_T struct req_tx_wqe_to_sge_info_t
#define TO_STAGE_T struct req_tx_to_stage_t
#define SQCB0_TO_SQCB1_T struct req_tx_sqcb0_to_sqcb1_info_t

%%

.align
req_tx_sqcb_process_ext:

    seq            c1, CAPRI_TXDMA_INTRINSIC_RECIRC_COUNT, 0 // Branch Delay Slot
    bcf            [!c1], exit

    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) 

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(SQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)
    // Is it UD service?
    seq            c1, d.service, RDMA_SERV_TYPE_UD
    CAPRI_SET_FIELD_C(r1, PHV_GLOBAL_COMMON_T, flags, REQ_TX_FLAG_UD_SERVICE, c1)

    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_START_FLIT_CMD_ID) // Branch Delay Slot

exit:
    nop.e
    nop
