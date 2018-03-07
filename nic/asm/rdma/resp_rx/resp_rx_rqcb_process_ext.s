#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define TO_S_FWD_INFO_T struct resp_rx_to_stage_wb1_info_t
#define TO_S_STATS_INFO_T struct resp_rx_to_stage_stats_info_t

%%

.align
resp_rx_rqcb_process_ext:

    // is this a fresh packet ?
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf     [!c1], recirc_pkt

    //fresh packet
    // populate global fields
    add r3, r0, offsetof(struct phv_, common_global_global_data) //BD Slot
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)

    //Temporary code to test UDP options
    //For now, checking on ts flag for both options ts and mss to avoid performance cost
    bbeq     CAPRI_APP_DATA_ROCE_OPT_TS_VALID, 0, skip_roce_opt_parsing
    CAPRI_SET_FIELD_RANGE(r3, PHV_GLOBAL_COMMON_T, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE) //BD Slot

    //get rqcb3 address
    add      r6, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_ts_value)
    memwr.d  r5, CAPRI_APP_DATA_ROCE_OPT_TS_VALUE_AND_ECHO
    add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_mss)
    memwr.h  r5, CAPRI_APP_DATA_ROCE_OPT_MSS

skip_roce_opt_parsing:

#   // get a tokenid for the fresh packet
#   phvwr  p.common.rdma_recirc_token_id, d.token_id

#   CAPRI_GET_STAGE_3_ARG(resp_rx_phv_t, r4)
#   CAPRI_SET_FIELD(r4, TO_S_FWD_INFO_T, my_token_id, d.token_id)

    CAPRI_GET_STAGE_7_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, TO_S_STATS_INFO_T, bytes, CAPRI_APP_DATA_PAYLOAD_LEN)

recirc_pkt:
exit:
    nop.e
    nop
