#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define INFO_OUT1_P t0_s2s_rqcb_to_wqe_info

#define TBL_ARG_P   r5
#define RAW_TABLE_PC r1
#define GLOBAL_FLAGS r7

#define IN_P t0_s2s_rqcb_to_pt_info
#define RQWQE_P CAPRI_KEY_RANGE(IN_P, rqwqe_p_sbit0_ebit7, rqwqe_p_sbit48_ebit63)
#define IN_PAGE_OFFSET CAPRI_KEY_FIELD(IN_P, page_offset)

%%
    .param  resp_rx_rqwqe_mpu_only_process

.align
resp_rx_rqpt_process:

    //page_addr_p = (u64 *) (d_p + sizeof(u64) * rqcb_to_pt_info_p->page_seg_offset);

    //big-endian
    sub     r3, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), CAPRI_KEY_FIELD(IN_P, page_seg_offset) //BD Slot
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64_BITS
    //big-endian
    tblrdp.dx  r3, r3, 0, CAPRI_SIZEOF_U64_BITS
    or         r3, r3, 1, 63
    or         r3, r3, K_GLOBAL_LIF, 52

    // wqe_p = (void *)(*page_addr_p + rqcb_to_pt_info_p->page_offset);
    add     r3, r3, IN_PAGE_OFFSET
    // now r3 has wqe_p to load

    CAPRI_RESET_TABLE_0_ARG()

    CAPRI_SET_FIELD2(INFO_OUT1_P, curr_wqe_ptr, r3)
    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, remaining_payload_bytes), \
                CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15), \
                CAPRI_PHV_FIELD(INFO_OUT1_P, dma_cmd_index), \
                RESP_RX_DMA_CMD_PYLD_BASE

    // invoke rqwqe mpu only
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqwqe_mpu_only_process, r3)
