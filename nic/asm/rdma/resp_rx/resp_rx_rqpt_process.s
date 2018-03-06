#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqpt_process_k_t k;

#define INFO_OUT1_T struct resp_rx_rqcb_to_wqe_info_t

#define TBL_KEY_P   r4
#define TBL_ARG_P   r5
#define RAW_TABLE_PC r1
#define GLOBAL_FLAGS r7

%%
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

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, TBL_KEY_P)
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, TBL_ARG_P)

    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, in_progress, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, current_sge_id, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, current_sge_offset, 0)
    //CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, num_valid_sges, 0)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, curr_wqe_ptr, r3)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(TBL_ARG_P, INFO_OUT1_T, dma_cmd_index, RESP_RX_DMA_CMD_PYLD_BASE)

    // we don't need to handle write_with_imm as we don't need to get
    // wrid for completion purpose (driver would take care of it).
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, r3)

    nop.e
    nop
