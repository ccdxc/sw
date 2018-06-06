#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define TO_S1_P to_s1_to_stage
#define TO_S5_P to_s5_to_stage

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

    phvwr       CAPRI_PHV_FIELD(TO_S5_P, bth_se), CAPRI_APP_DATA_BTH_SE

    // set DMA CMD ptr
    RXDMA_DMA_CMD_PTR_SET(REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START_FLIT_CMD_ID)
    // In case of RDMA feedback, skip_to_eop is not required as there is no payload. Also,
    // remaining_payload_bytes in to_stage is not relevant for the feedback msg.
    seq            c2, CAPRI_APP_DATA_RAW_FLAGS, REQ_RX_FLAG_RDMA_FEEDBACK
    seq            c1, CAPRI_APP_DATA_BTH_PAD, 0
    bcf            [c1 | c2], to_stage_arg
    // payload_len = app_data_payload_len = bth.padcnt
    sub            r1, CAPRI_APP_DATA_PAYLOAD_LEN, CAPRI_APP_DATA_BTH_PAD // Branch Delay Slot

    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_SKIP_TO_EOP)
    DMA_SKIP_CMD_SETUP(r2, 0 /* CMD_EOP */, 1 /* SKIP_TO_EOP */)

to_stage_arg:
    phvwrpair.e CAPRI_PHV_FIELD(TO_S1_P, aeth_msn), CAPRI_APP_DATA_AETH_MSN, \
                CAPRI_PHV_FIELD(TO_S1_P, bth_psn), CAPRI_APP_DATA_BTH_PSN
    phvwrpair   CAPRI_PHV_FIELD(TO_S1_P, aeth_syndrome), CAPRI_APP_DATA_AETH_SYNDROME, \
                CAPRI_PHV_FIELD(TO_S1_P, remaining_payload_bytes), r1

