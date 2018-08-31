#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s1_t0_k k;

#define TO_STAGE_T struct resp_tx_to_stage_t

%%
    .param      resp_tx_dcqcn_enforce_process

resp_tx_ack_process:

    // check if syndrome is 0
    seq          c7, d.syndrome, 0
    bcf          [c7], process_ack
    // c7: ACK

    // check if rnr and update the syndrome
    // using the rnr_timeout in 'd'
    seq          c6, d.syndrome[7:5], AETH_CODE_RNR // BD Slot
    // c6: RNR 
    AETH_RNR_SYNDROME_GET_C(r6, d.rnr_timeout, c6) 
    // copy syndrome into r6 if not RNR (i.e. NAK)
    add.!c6      r6, d.syndrome, r0

process_ack:
    AETH_ACK_SYNDROME_GET_C(r6, d.credits, c7)
    // prepare aeth
    phvwrpair   p.aeth.syndrome, r6, p.aeth.msn, d.msn
    phvwr       p.bth.psn, d.ack_nak_psn

    // invoke MPU only dcqcn in table 1.
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r0)
 
    CAPRI_SET_TABLE_0_VALID(0)
exit:
    nop.e
    nop
