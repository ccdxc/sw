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

    // check if rnr and update the syndrome
    seq          c7, d.aeth.syndrome[7:5], AETH_CODE_RNR
    // c7 is set if syndrome code is RNR
    AETH_RNR_SYNDROME_GET_C(r6, d.rnr_timeout, c7) 
    //copy syndrome into r6 if not RNR
    add.!c7      r6, d.aeth.syndrome, r0

    // prepare aeth
    phvwrpair   p.aeth.syndrome, r6, p.aeth.msn, d.aeth.msn
    phvwr       p.bth.psn, d.ack_nak_psn

    // invoke MPU only dcqcn in table 1.
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r0)
 
    CAPRI_SET_TABLE_0_VALID(0)
exit:
    nop.e
    nop
