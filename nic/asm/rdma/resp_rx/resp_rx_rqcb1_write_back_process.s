#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqcb1_write_back_process_k_t k;
struct rqcb1_t d;

#define F_SEND_COMPL_AND_MID_OR_LAST  c7
#define F_COMPL_AND_SEND         c6
#define F_INV_RKEY               c5
#define F_MID_OR_LAST            c4
#define F_ERR                    c3

#define GBL_FLAGS           r7
#define TBL_ID              r6
#define KEY_P               r5
#define ARG_P               r4
#define CQCB_ADDR           r3
#define RQCB4_ADDR          r3
#define KEY_ADDR            r3
#define RAW_TABLE_PC        r2

#define STATS_INFO_T struct resp_rx_stats_info_t

%%
    .param                  resp_rx_cqcb_process
    .param                  resp_rx_inv_rkey_process
    .param                  resp_rx_stats_process

.align
resp_rx_rqcb1_write_back_process:

    bbeq        k.global.flags.resp_rx._only, 1, skip_updates
    add         r7, r0, k.global.flags //BD Slot

    tblwr       d.current_sge_id, k.args.current_sge_id;
    tblwr       d.current_sge_offset, k.args.current_sge_offset;
    crestore    [c2, c1], k.{to_stage.s4.wb1.update_wqe_ptr...to_stage.s4.wb1.update_num_sges}, 0x3
    #c2 - update_wqe_ptr
    #c1 - update_num_sges
    tblwr.c2    d.curr_wqe_ptr, k.args.curr_wqe_ptr;
    tblwr.c1    d.num_sges, k.to_stage.s4.wb1.num_sges

skip_updates:

    // if completion is not set, stats after writeback
    bbne        k.global.flags.resp_rx._completion, 1, stats
    //assumption is that write back is called with table 2
    CAPRI_SET_TABLE_2_VALID(0)  //BD Slot

    // if both non-first/only & completion for a send packet is set, 
    // wrid present in CB1 should be stored into phv.cqwqe
    IS_ANY_FLAG_SET(F_MID_OR_LAST, r7, RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_LAST)
    ARE_ALL_FLAGS_SET(F_COMPL_AND_SEND, r7, RESP_RX_FLAG_SEND | RESP_RX_FLAG_COMPLETION)
    setcf        F_SEND_COMPL_AND_MID_OR_LAST, [F_MID_OR_LAST & F_COMPL_AND_SEND]
    
    ARE_ALL_FLAGS_SET(F_INV_RKEY, r7, RESP_RX_FLAG_INV_RKEY)
    ARE_ALL_FLAGS_SET(F_ERR, r7, RESP_RX_FLAG_ERR_DIS_QP)

    bcf                     [!F_INV_RKEY | F_ERR], completion
    phvwr.F_SEND_COMPL_AND_MID_OR_LAST  p.cqwqe.id.wrid, d.wrid // BD Slot

inv_rkey:

    KT_BASE_ADDR_GET(r3, r2)

    //key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add         r2, r0, k.to_stage.s4.wb1.inv_r_key    

    //andi        r2, r2, KEY_INDEX_MASK
    //sll         r2, r2, LOG_SIZEOF_KEY_ENTRY_T
    //add         r2, r2, r3

    KEY_ENTRY_ADDR_GET(KEY_ADDR, r3, r2)
    // now r3 has key_addr


    CAPRI_GET_TABLE_3_K(resp_rx_phv_t, KEY_P)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_inv_rkey_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, KEY_ADDR)

completion:
    
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, KEY_P)
    RESP_RX_CQCB_ADDR_GET(CQCB_ADDR, d.cq_id)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_cqcb_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, CQCB_ADDR)

stats:

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, KEY_P)
    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, ARG_P)
    CAPRI_SET_FIELD(ARG_P, STATS_INFO_T, bubble_count, 1)
    RQCB4_ADDR_GET(RQCB4_ADDR)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_stats_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB4_ADDR)

exit:

    nop.e
    nop 
    
