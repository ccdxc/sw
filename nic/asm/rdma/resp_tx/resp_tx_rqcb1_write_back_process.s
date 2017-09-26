#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rqcb1_write_back_process_k_t k;
struct rqcb1_t d;

#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4

%%

resp_tx_rqcb1_write_back_process:

    tblwr       d.read_rsp_in_progress, k.args.read_rsp_in_progress
    seq         c1, k.args.read_rsp_in_progress, 1
    cmov        CURR_READ_RSP_PSN, c1, k.args.curr_read_rsp_psn, 0

    // TODO: ordering rules
    tblwr       d.curr_read_rsp_psn, CURR_READ_RSP_PSN
    tblwr       d.read_rsp_lock, 0
    bcf         [c1], exit
    CAPRI_SET_TABLE_1_VALID(0) //BD Slot

    // TODO: ordering rules
    // ring doorbell to update RSQ_C_INDEX to NEW_RSQ_C_INDEX
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, RSQ_RING_ID, k.args.new_rsq_c_index, DB_ADDR, DB_DATA) 

exit:
    nop.e
    nop
