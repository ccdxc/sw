#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_compl_or_inv_rkey_process_k_t k;
struct rqcb1_t d;

#define F_COMPL_AND_LAST    c7
#define F_COMPL             c6
#define F_INV_RKEY          c5

#define GBL_FLAGS           r7
#define TBL_ID              r6
#define KEY_P               r5
#define ARG_P               r4
#define CQCB_ADDR           r3
#define RAW_TABLE_PC        r2

#define CQ_INFO_T   struct resp_rx_rqcb_to_cq_info_t


%%
    .param                  resp_rx_cqcb_process

.align
resp_rx_compl_or_inv_rkey_process:

    add                     r7, r0, k.global.flags
    add                     r6, r0, k.args.tbl_id

    // if both last & completion is set, wrid present in CB1 should be stored into phv.cqwqe
    ARE_ALL_FLAGS_SET(F_COMPL_AND_LAST, r7, RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_LAST)
    ARE_ALL_FLAGS_SET(F_COMPL, r7, RESP_RX_FLAG_COMPLETION)
    ARE_ALL_FLAGS_SET(F_INV_RKEY, r7, RESP_RX_FLAG_INV_RKEY)

    bcf                     [!F_INV_RKEY], completion
    phvwr.F_COMPL_AND_LAST  p.cqwqe.id.wrid, d.wrid // BD Slot

inv_rkey:


    //TODO
    add                     TBL_ID, TBL_ID, 1



completion:
    

    CAPRI_GET_TABLE_I_K_AND_ARG(resp_rx_phv_t, TBL_ID, KEY_P, ARG_P)
    CAPRI_SET_FIELD(ARG_P, CQ_INFO_T, tbl_id, TBL_ID)
    CAPRI_SET_FIELD(ARG_P, CQ_INFO_T, dma_cmd_index, k.args.dma_cmd_index)
    CQCB_ADDR_GET(CQCB_ADDR, d.cq_id)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_cqcb_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, CQCB_ADDR)

    nop.e
    nop 
    
