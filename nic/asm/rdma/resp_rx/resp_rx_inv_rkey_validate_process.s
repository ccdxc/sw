#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"

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

#define IN_TO_S_P to_s4_lkey_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_STATS_INFO_P to_s7_stats_info


%%

.align
resp_rx_inv_rkey_validate_process:

    // this program is loaded only for send with invalidate

    bbeq        CAPRI_KEY_FIELD(IN_TO_S_P, rsvd_key_err), 1, error_completion
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS // BD Slot

    /*  check if MR is eligible for invalidation
     *  check if state is invalid (same for MR and MW)
     *  if MR - check PD
     *  if type 1 MW - not allowed
     *  if type 2A MW - 
     *      * check QP if state is valid
     *      * check PD if state is free
     *  if any check fails - send NAK with appropriate error and move QP to error disable state
     */

    // it is an error to invalidate an MR not eligible for invalidation
    seq         c1, d.mr_flags.inv_en, 0 //BD Slot
    // it is an error to invalidate an MR/MW in INVALID state
    seq         c2, d.state, KEY_STATE_INVALID
    // invalidation is not allowed for type 1 MW
    seq         c3, d.type, MR_TYPE_MW_TYPE_1 
    bcf         [c1 | c2 | c3], error_completion

    // check PD if MR
    seq         c1, d.type, MR_TYPE_MR // BD Slot
    bcf         [c1], check_pd
    // c1: MR

    // neither MW type 1, nor MR, must be MW type 2A
    seq         c2, d.state, KEY_STATE_FREE // BD Slot
    bcf         [c2], check_pd

    // must be MW type 2A in valid state, check QP
    seq         c3, d.qp, K_GLOBAL_QID // BD Slot 
    bcf         [c3], update_state
    nop // BD Slot

    b           error_completion

check_pd:
    seq         c2, d.pd, CAPRI_KEY_FIELD(IN_TO_S_P, pd) // BD Slot
    bcf         [!c2], error_completion
    nop // BD Slot

update_state:
    // move state update post write_back (inv_rkey_process)
    CAPRI_SET_FIELD2(TO_S_WB1_P, inv_rkey, 1)
    // if key type is not MR, must be MW, update stats
    CAPRI_SET_FIELD2_C(TO_S_STATS_INFO_P, incr_mem_window_inv, 1, !c1)

exit:
    CAPRI_SET_TABLE_3_VALID_CE(c0, 0)
    nop // Exit Slot

error_completion:
    // As per standard, NAK is NOT required to be generated when an
    // incoming Send with Invalidate contains an invalid R_Key, 
    // or the R_Key contained in the IETH cannot be invalidated
    // However, RQ should be moved to ERROR state.
    // But it may be better to anyway generate a NAK if we can, 
    // as moving silently to error disable state is not a good thing anyway.

    // Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

    // update cqe status and error in phv so that completion with error is generated
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_ACC_ERR, p.cqe.error, 1
    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    // set error disable flag and ACK flag
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, (RESP_RX_FLAG_ERR_DIS_QP|RESP_RX_FLAG_ACK_REQ)

    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_CQE_ERR_OFFSET))

    CAPRI_SET_TABLE_3_VALID_CE(c0, 0)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS) // Exit Slot

