#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s4_t2_k k;

#define IN_P t2_s2s_prefetch_info
#define INFO_OUT1_P to_s6_rqpt_info
#define TO_S5_P to_s5_rqcb1_wb_info

#define K_PT_BASE_ADDR CAPRI_KEY_FIELD(IN_P, pt_base_addr)
#define K_RQ_PINDEX CAPRI_KEY_FIELD(IN_P, rq_pindex)
#define K_LOG_RQ_PAGE_SIZE CAPRI_KEY_FIELD(IN_P, log_rq_page_size)
#define K_LOG_WQE_SIZE CAPRI_KEY_RANGE(IN_P, log_wqe_size_sbit0_ebit2, log_wqe_size_sbit3_ebit4)
#define K_CMD_EOP CAPRI_KEY_FIELD(IN_P, cmd_eop)
#define K_LOG_NUM_WQES CAPRI_KEY_FIELD(IN_P, log_num_wqes)
#define K_PREFETCH_BASE_ADDR CAPRI_KEY_FIELD(IN_P, prefetch_cb_or_base_addr)
#define K_CHECK_IN CAPRI_KEY_FIELD(IN_P, check_in)
#define K_PREFETCH_BUF_INDEX CAPRI_KEY_FIELD(IN_P, prefetch_buf_index)

#define RQ_NUM_WQES       r6
#define PREFETCH_NUM_WQES r7
#define PAGE_NUM_WQES     r4
#define TRANSFER_NUM_WQES r6

%%

    .param      resp_tx_rqpt_mpu_only_process

.align
resp_tx_rqprefetch_process:

    // compare RQ PI with RQ proxy CI
    // if equal, all WQE's have already been prefetched
    seq         c1, K_RQ_PINDEX, PROXY_RQ_C_INDEX
    bcf         [c1], prefetch_done
    nop // BD Slot

    bbeq        d.prefetch_init_done, 1, skip_init
    add         r1, r0, K_PREFETCH_BASE_ADDR // BD Slot

    tblwr       d.prefetch_base_addr, r1
    tblwr       d.prefetch_buf_index, K_PREFETCH_BUF_INDEX

    RQCB1_ADDR_GET(r2)
    add         r2, r2, FIELD_OFFSET(rqcb1_t, prefetch_base_addr)
    // byte based memwr are safer as it wont cause alignment issues
    // when field changes its offset in future
    memwr.b     r2, r1[31:24]
    add         r2, r2, 1
    memwr.b     r2, r1[23:16]
    add         r2, r2, 1
    memwr.b     r2, r1[15:8]
    add         r2, r2, 1
    memwr.b     r2, r1[7:0]

    tblwr       d.prefetch_init_done, 1

skip_init:
    // if prefetch proxy CI = (prefetch PI + 1) % prefetch_buffer_size,
    // there is no space in the prefetch buffer
    add         r1, r0, PREFETCH_P_INDEX
    mincr       r1, d.log_num_prefetch_wqes, 1
    seq         c2, PREFETCH_PROXY_C_INDEX, r1
    bcf         [c2], prefetch_done

    // calculate the number of WQE's in RQ buffer yet to be
    // prefetched, before RQ ring wraps around
    // if rq_pindex < proxy_cindex, it has wrapped around
    slt         c1, K_RQ_PINDEX, PROXY_RQ_C_INDEX // BD Slot
    sll         RQ_NUM_WQES, 1, d.log_num_prefetch_wqes
    sub.c1      RQ_NUM_WQES, RQ_NUM_WQES, PROXY_RQ_C_INDEX
    // rq_num_wqes = rq PI - rq proxy CI

    sub.!c1     RQ_NUM_WQES, K_RQ_PINDEX, PROXY_RQ_C_INDEX

    // calculate the number of WQE's that can be prefetched
    // before prefetch ring wraps around
    // if prefetch_proxy_cindex <= prefetch_pindex, handle wrap around
    sle         c1, PREFETCH_PROXY_C_INDEX, PREFETCH_P_INDEX
    bcf         [!c1], no_wrap_around
    sll         PREFETCH_NUM_WQES, 1, d.log_num_prefetch_wqes // BD Slot
    sub         PREFETCH_NUM_WQES, PREFETCH_NUM_WQES, PREFETCH_P_INDEX
    seq         c2, PREFETCH_PROXY_C_INDEX, 0
    sub.c2      PREFETCH_NUM_WQES, PREFETCH_NUM_WQES, 1

no_wrap_around:
    // prefetch_num_wqes = prefetch proxy CI - prefetch PI - 1
    sub.!c1     PREFETCH_NUM_WQES, PREFETCH_PROXY_C_INDEX, PREFETCH_P_INDEX
    sub.!c1     PREFETCH_NUM_WQES, PREFETCH_NUM_WQES, 1

    slt         c1, RQ_NUM_WQES, PREFETCH_NUM_WQES
    cmov        TRANSFER_NUM_WQES, c1, RQ_NUM_WQES, PREFETCH_NUM_WQES

    sub         r2, K_LOG_RQ_PAGE_SIZE, K_LOG_WQE_SIZE
    sll         PAGE_NUM_WQES, 1, r2
    // page_index = c_index >> (log_rq_page_size - log_wqe_size)
    add         r1, r0, PROXY_RQ_C_INDEX
    srlv        r3, r1, r2

    // page_offset = c_index & ((1 << (log_rq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr       r1, r2, r0
    // r1: c_index within page
    // PAGE_NUM_WQES = num wqes in a page - c_index within page
    sub         PAGE_NUM_WQES, PAGE_NUM_WQES, r1
    // r4 = num wqes remaining in page
    slt         c1, PAGE_NUM_WQES, TRANSFER_NUM_WQES
    cmov        TRANSFER_NUM_WQES, c1, PAGE_NUM_WQES, TRANSFER_NUM_WQES
    sll         r1, r1, K_LOG_WQE_SIZE

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and         r5, r3, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub         r3, r3, r5
    // page_index = page_index * sizeof(u64)
    // page_index += rqcb_p->pt_base_addr
    sll         r4, K_PT_BASE_ADDR, PT_BASE_ADDR_SHIFT
    add         r3, r4, r3, CAPRI_LOG_SIZEOF_U64
    // now r3 has page_p to load

    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, prefetch_base_addr), d.prefetch_base_addr, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, page_offset), r1

    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, page_seg_offset), r5, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, log_wqe_size), K_LOG_WQE_SIZE

    add         r1, r0, PREFETCH_P_INDEX
    CAPRI_SET_FIELD2(INFO_OUT1_P, prefetch_pindex_pre, r1)

    tblmincr    PROXY_RQ_C_INDEX, K_LOG_NUM_WQES, TRANSFER_NUM_WQES
    tblmincr    PREFETCH_P_INDEX, d.log_num_prefetch_wqes, TRANSFER_NUM_WQES

    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, prefetch_pindex_post), d.prefetch_pindex, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, cmd_eop), K_CMD_EOP
    CAPRI_SET_FIELD2(INFO_OUT1_P, transfer_num_wqes, TRANSFER_NUM_WQES)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_rqpt_mpu_only_process, r3) // Exit slot

prefetch_done:
    //seq         c2, K_CHECK_IN, 1
    //tblwr.c2    d.prefetch_init_done, 0
    //tblwr.c2    d.prefetch_base_addr, 0

    seq         c1, K_CMD_EOP, 1
    CAPRI_SET_TABLE_2_VALID_CE(c0, 0)
    // drop the PHV only if there are no DMA commands in another path
    phvwr.c1    p.common.p4_intr_global_drop, 1 // Exit Slot
