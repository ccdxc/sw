#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb1_t d;

#define TO_STAGE_T struct resp_tx_to_stage_t

%%
    .param      resp_tx_dcqcn_enforce_process

resp_tx_ack_process:

    // Pass dcqcn_cb_addr to stage 3.
    add         r2, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    CAPRI_GET_STAGE_3_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s3.dcqcn.dcqcn_cb_addr, r2)

    // invoke MPU only dcqcn in table 1.
    CAPRI_GET_TABLE_1_K(resp_tx_phv_t, r4)
    CAPRI_NEXT_TABLE_I_READ_PC(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r0)
 
    CAPRI_SET_TABLE_0_VALID(0)
exit:
    nop.e
    nop
