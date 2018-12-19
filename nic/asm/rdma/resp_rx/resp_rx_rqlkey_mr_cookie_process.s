#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t0_k k;
struct key_entry_aligned_t d;

#define IN_TO_S_P    to_s4_lkey_info
#define TO_S_STATS_INFO_P to_s7_stats_info

#define GLOBAL_FLAGS r6
#define DMA_CMD_BASE r1

#define K_MW_COOKIE CAPRI_KEY_FIELD(IN_TO_S_P, mw_cookie)

%%

.align
resp_rx_rqlkey_mr_cookie_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_4
    bcf              [!c1], bubble_to_next_stage
    seq              c1, r1[4:2], STAGE_3 // BD Slot

    // access is allowed only in valid state
    seq         c1, d.state, KEY_STATE_VALID
    seq         c2, d.mr_cookie, K_MW_COOKIE 

    bcf         [!c1 | !c2], error_completion
    nop // BD Slot

    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    nop // Exit Slot

bubble_to_next_stage:
    // c1: stage_3
    bcf              [!c1], exit
    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r7) // BD Slot

    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS)
    nop // Exit Slot

exit:
    nop.e
    nop

error_completion:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_COMPLETION)
    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_READ_REQ|RESP_RX_FLAG_ATOMIC_FNA|RESP_RX_FLAG_ATOMIC_CSWAP)

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_ACC_ERR, p.cqe.error, 1
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_CQE_ERR_OFFSET))

    // set error disable flag 
    // turn on ACK req bit
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_ACK_REQ

    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, c2)

    //Generate DMA command to skip to payload end
    // move dma cmd base by 2 to accomodate ACK info 
    // and doorbell ringing
    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 2)
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, c1)
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0) 
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, !c1) // Exit Slot
