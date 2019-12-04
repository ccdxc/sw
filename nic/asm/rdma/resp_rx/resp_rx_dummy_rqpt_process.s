#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define IN_P t0_s2s_rqcb_to_wqe_info
#define GLOBAL_FLAGS    r7
#define DMA_CMD_BASE    r1

#define IN_TO_S_P to_s1_rqpt_info
#define TO_S_STATS_INFO_P to_s7_stats_info

#define K_SEND_SGE_OPT CAPRI_KEY_FIELD(IN_TO_S_P, send_sge_opt)
#define K_HBM_WQE_PTR CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)
#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)

%%
    .param  resp_rx_rqwqe_process
    .param  resp_rx_rqwqe_wrid_process
    .param  resp_rx_rqwqe_opt_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_dummy_rqpt_process:

    bcf         [c2 | c3 | c7], table_error
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS // BD Slot

    bbeq        K_SEND_SGE_OPT, 1, rqwqe_opt
    ARE_ALL_FLAGS_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT) // BD Slot

    // if send_only and non_sge_opt case, populate cqe payload len here.
    // for send_fml case, it would happen in rqcb3_in_progress_process.
    ARE_ALL_FLAGS_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_SEND|RESP_RX_FLAG_ONLY)
    phvwr.c2    p.cqe.length, K_REM_PYLD_BYTES

    CAPRI_NEXT_TABLE0_READ_PC_CE(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_wrid_process, resp_rx_rqwqe_process, K_HBM_WQE_PTR, c1);

rqwqe_opt:
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_opt_process, K_HBM_WQE_PTR)

table_error:
    // set err_dis_qp and completion flags
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_COMPLETION
    and         GLOBAL_FLAGS, GLOBAL_FLAGS, ~(RESP_RX_FLAG_ACK_REQ)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1
    // TODO update lif_error_id if needed

    // update stats
    phvwrpair.c2   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_table_error), 1

    phvwrpair.c3   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_phv_intrinsic_error), 1

    phvwrpair.c7   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_table_resp_error), 1

    CAPRI_SET_TABLE_0_VALID(0)

    //Generate DMA command to skip to payload end if non-zero payload
    seq         c1, K_REM_PYLD_BYTES, 0
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, !c1)

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
