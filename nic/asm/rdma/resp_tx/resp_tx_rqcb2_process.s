#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s1_t0_k k;

#define RSQWQE_INFO_P t0_s2s_rqcb2_to_rsqwqe_info
#define TO_STAGE_T struct resp_tx_to_stage_t

#define IN_P t0_s2s_rqcb_to_rqcb2_info

%%
    .param      resp_tx_rsqwqe_process

resp_tx_rqcb2_process:

    // prepare aeth
    phvwrpair   p.aeth.syndrome, d.aeth.syndrome, p.aeth.msn, d.aeth.msn

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RSQWQE_INFO_P, curr_read_rsp_psn, read_rsp_in_progress, CAPRI_KEY_RANGE(IN_P, curr_read_rsp_psn_sbit0_ebit7, read_rsp_in_progress)) 

    // load 256 bits (i.e,32 bytes) which is the size of rsqwqe
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_tx_rsqwqe_process, CAPRI_KEY_RANGE(IN_P, rsqwqe_addr_sbit0_ebit7, rsqwqe_addr_sbit32_ebit63))

exit:
    nop.e
    nop
