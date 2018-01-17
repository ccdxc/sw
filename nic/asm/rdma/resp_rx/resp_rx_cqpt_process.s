#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_cqpt_process_k_t k;

#define PAGE_ADDR_P     r1
#define CQWQE_P         r1
#define DMA_CMD_BASE    r2
#define TMP             r3
#define EQCB_ADDR       r5

#define KEY_P           r3
#define ARG_P           r7

#define RAW_TABLE_PC    r2

//TODO: verify if it is in right order
//#define PHV_CQWQE_START cqwqe.id.wrid
//#define PHV_CQWQE_END   cqwqe.r_key

#define EQ_INFO_T struct resp_rx_cqcb_to_eq_info_t

%%
    .param  resp_rx_eqcb_process

.align
resp_rx_cqpt_process:

    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             PAGE_ADDR_P, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset
    sll             PAGE_ADDR_P, PAGE_ADDR_P, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp.dx       PAGE_ADDR_P, PAGE_ADDR_P, 0, CAPRI_SIZEOF_U64_BITS

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQWQE_P, PAGE_ADDR_P, k.args.page_offset

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqwqe, cqwqe, CQWQE_P)

    CAPRI_SET_TABLE_2_VALID(0)
    seq             c2, k.args.arm, 1 
    bcf             [!c2], cqpt_exit
    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE, !c2) //BD slot

    RESP_RX_EQCB_ADDR_GET(EQCB_ADDR, TMP, k.args.eq_id) // BD Slot

    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, KEY_P)
    CAPRI_GET_TABLE_2_ARG(resp_rx_phv_t, ARG_P)
    CAPRI_SET_FIELD(ARG_P, EQ_INFO_T, cq_id, k.args.cq_id)
    CAPRI_NEXT_TABLE_I_READ_PC(KEY_P, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_eqcb_process, EQCB_ADDR)

cqpt_exit:

    nop.e
    nop

