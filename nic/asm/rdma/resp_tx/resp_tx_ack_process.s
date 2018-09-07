#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s1_t0_k k;

#define TO_STAGE_T struct resp_tx_to_stage_t
#define TO_S5_P to_s5_rqcb1_wb_info

%%
    .param      resp_tx_dcqcn_enforce_process

resp_tx_ack_process:

    // check if syndrome is ACK
    seq          c7, d.syndrome[7:5], AETH_CODE_ACK
    bcf          [c7], prepare_aeth
    AETH_ACK_SYNDROME_GET_C(r6, d.credits, c7)  //BD Slot

    // check if syndrome is RNR and update the syndrome
    // using the rnr_timeout in 'd'
    seq          c6, d.syndrome[7:5], AETH_CODE_RNR
    bcf         [c6], prepare_aeth
    AETH_RNR_SYNDROME_GET_C(r6, d.rnr_timeout, c6)  //BD Slot

    // all the below code assumes that syndrome is NAK
    seq         c5, d.syndrome[4:0], NAK_CODE_SEQ_ERR
    bcf         [c5], prepare_aeth
    add         r6, d.syndrome, r0 //BD Slot

    // for any nak code other than NAK_CODE_SEQ_ERR, we need to error disable qp
    phvwr       CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1

    // when rq generates the flush wqe, it is communicated to resp_tx side using a vendor
    // specific nak code. In this case, all we have to do is to move the state to Error.
    // This happens in the write back code. Note that we don't even need to inform sq, 
    // as flush itself is generated because of sq moving to error.
    seq         c4, d.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_FLUSH_RQ)
    b           prepare_aeth 
    phvwr.c4    CAPRI_PHV_FIELD(TO_S5_P, flush_rq), 1   //BD Slot
    
prepare_aeth:
    // prepare aeth
    phvwrpair   p.aeth.syndrome, r6, p.aeth.msn, d.msn
    phvwr       p.bth.psn, d.ack_nak_psn

    phvwr       CAPRI_PHV_FIELD(phv_global_common, _ack), 1

    // invoke MPU only dcqcn in table 1.
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r0)
 
    CAPRI_SET_TABLE_0_VALID(0)
exit:
    nop.e
    nop
