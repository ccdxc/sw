#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqpt_process_k_t k;

#define INFO_OUT1_T struct resp_rx_rqcb_to_wqe_info_t

#define TBL_KEY_P   r4
#define TBL_ARG_P   r5
#define TBL_ID      r6
#define RAW_TABLE_PC r1
#define GLOBAL_FLAGS r7

%%
    .param  resp_rx_rqwqe_wrid_process
    .param  resp_rx_rqwqe_process

.align
resp_rx_rqpt_process:

    add         GLOBAL_FLAGS, r0, k.global.flags

    //page_addr_p = (u64 *) (d_p + sizeof(u64) * rqcb_to_pt_info_p->page_seg_offset);

    //big-endian
    sub     r3, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset //BD Slot
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64_BITS
    //big-endian
    tblrdp.dx  r3, r3, 0, CAPRI_SIZEOF_U64_BITS

    // wqe_p = (void *)(*page_addr_p + rqcb_to_pt_info_p->page_offset);
    add     r3, r3, k.args.page_offset
    // now r3 has wqe_p to load

    add     TBL_ID, r0, k.args.tbl_id
    CAPRI_GET_TABLE_I_K_AND_ARG(resp_rx_phv_t, TBL_ID, TBL_KEY_P, TBL_ARG_P)

    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, in_progress, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, current_sge_id, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, current_sge_offset, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, num_valid_sges, 0)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, curr_wqe_ptr, r3)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, cache, k.args.cache)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, tbl_id, k.args.tbl_id)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, inv_r_key, k.args.inv_r_key)

    // if write_with_imm, load resp_rx_rqwqe_wrid_process, 
    // else load resp_rx_rqwqe_process
    ARE_ALL_FLAGS_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)
    CAPRI_SET_RAW_TABLE_PC_C(c1, RAW_TABLE_PC, resp_rx_rqwqe_wrid_process)
    CAPRI_SET_RAW_TABLE_PC_C(!c1, RAW_TABLE_PC, resp_rx_rqwqe_process)

    CAPRI_NEXT_TABLE_I_READ(TBL_KEY_P, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)

    nop.e
    nop
