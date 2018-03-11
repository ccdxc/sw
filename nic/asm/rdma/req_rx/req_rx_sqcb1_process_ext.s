#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;
#define SQCB1_TO_STAGE_T struct req_rx_to_stage_t

%%

.align
req_rx_sqcb1_process_ext:

    // global fields
    add            r1, r0, offsetof(struct phv_, common_global_global_data)

    // qstate addr is available as instrinsic data
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD_RANGE(r1, PHV_GLOBAL_COMMON_T, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, flags, CAPRI_APP_DATA_RAW_FLAGS)

    // set DMA CMD ptr
    RXDMA_DMA_CMD_PTR_SET(REQ_RX_DMA_CMD_START_FLIT_ID)

    CAPRI_GET_STAGE_1_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_TO_STAGE_T, msn, CAPRI_APP_DATA_AETH_MSN)
    CAPRI_SET_FIELD(r7, SQCB1_TO_STAGE_T, bth_psn, CAPRI_APP_DATA_BTH_PSN)
    CAPRI_SET_FIELD(r7, SQCB1_TO_STAGE_T, syndrome, CAPRI_APP_DATA_AETH_SYNDROME)

    nop.e
    nop
