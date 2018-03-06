#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_rqcb2_process_k_t k;

#define RSQWQE_INFO_T struct resp_tx_rqcb2_to_rsqwqe_info_t
#define TO_STAGE_T struct resp_tx_to_stage_t

%%
    .param      resp_tx_rsqwqe_process

resp_tx_rqcb2_process:

    // prepare aeth
    phvwrpair   p.aeth.syndrome, d.aeth.syndrome, p.aeth.msn, d.aeth.msn

    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r4)
    CAPRI_SET_FIELD_RANGE(r4, RSQWQE_INFO_T, curr_read_rsp_psn, read_rsp_in_progress, k.{args.curr_read_rsp_psn...args.read_rsp_in_progress}) 

    // load 256 bits (i.e,32 bytes) which is the size of rsqwqe
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_tx_rsqwqe_process, k.args.rsqwqe_addr)

exit:
    nop.e
    nop
