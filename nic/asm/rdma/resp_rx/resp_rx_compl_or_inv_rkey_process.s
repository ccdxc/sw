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
#define F_ERR               c4

#define GBL_FLAGS           r7
#define TBL_ID              r6
#define KEY_P               r5
#define ARG_P               r4
#define CQCB_ADDR           r3
#define KEY_ADDR            r3
#define KEY_ID              r2
#define RAW_TABLE_PC        r2

#define CQ_INFO_T   struct resp_rx_rqcb_to_cq_info_t
#define INV_RKEY_T  struct resp_rx_inv_rkey_info_t


%%
    .param                  resp_rx_cqcb_process
    .param                  resp_rx_inv_rkey_process

.align
resp_rx_compl_or_inv_rkey_process:

    add                     r7, r0, k.global.flags
    add                     TBL_ID, r0, k.args.tbl_id

    // if both last & completion is set, wrid present in CB1 should be stored into phv.cqwqe
    ARE_ALL_FLAGS_SET(F_COMPL_AND_LAST, r7, RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_LAST)
    ARE_ALL_FLAGS_SET(F_COMPL, r7, RESP_RX_FLAG_COMPLETION)
    ARE_ALL_FLAGS_SET(F_INV_RKEY, r7, RESP_RX_FLAG_INV_RKEY)
    ARE_ALL_FLAGS_SET(F_ERR, r7, RESP_RX_FLAG_ERR_DIS_QP)

    bcf                     [!F_INV_RKEY | F_ERR], completion
    phvwr.F_COMPL_AND_LAST  p.cqwqe.id.wrid, d.wrid // BD Slot

inv_rkey:

    KT_BASE_ADDR_GET(r3, r2)

    //key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add         r2, r0, k.args.r_key    
    andi        r2, r2, KEY_INDEX_MASK
    sll         r2, r2, LOG_SIZEOF_KEY_ENTRY_T

    add         r2, r2, r3
    // now r2 has key_addr

    //aligned_key_addr = key_addr & ~HBM_CACHE_LINE_MASK;
    and         r3, r2, HBM_CACHE_LINE_SIZE_MASK
    sub         KEY_ADDR, r2, r3
    // r3 now has aligned_key_addr

    //key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
    // compute (key_addr - aligned_key_addr) >> log_key_entry_t
    sub         r2, r2, r3
    srl         KEY_ID, r2, LOG_SIZEOF_KEY_ENTRY_T
    // r2 now has key_id

    CAPRI_GET_TABLE_I_K_AND_ARG(resp_rx_phv_t, TBL_ID, KEY_P, ARG_P)
    CAPRI_SET_FIELD(ARG_P, INV_RKEY_T, tbl_id, TBL_ID)
    CAPRI_SET_FIELD(ARG_P, INV_RKEY_T, key_id, KEY_ID)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_inv_rkey_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, KEY_ADDR)

    //TODO
    add                     TBL_ID, TBL_ID, 1

completion:
    
    CAPRI_GET_TABLE_I2_K_AND_ARG(resp_rx_phv_t, TBL_ID, KEY_P, ARG_P)
    CAPRI_SET_FIELD(ARG_P, CQ_INFO_T, tbl_id, TBL_ID)
    CAPRI_SET_FIELD(ARG_P, CQ_INFO_T, dma_cmd_index, k.args.dma_cmd_index)
    CQCB_ADDR_GET(CQCB_ADDR, d.cq_id)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_cqcb_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, CQCB_ADDR)

    nop.e
    nop 
    
