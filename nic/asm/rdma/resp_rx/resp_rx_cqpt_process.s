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
#define DMA_CMD_INDEX   r4
#define EQCB_ADDR       r5
#define TBL_ID          r6

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

    add             TBL_ID, r0, k.args.tbl_id
    add             DMA_CMD_INDEX, r0, k.args.dma_cmd_index
    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             PAGE_ADDR_P, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset
    sll             PAGE_ADDR_P, PAGE_ADDR_P, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp.dx       PAGE_ADDR_P, PAGE_ADDR_P, 0, CAPRI_SIZEOF_U64_BITS

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQWQE_P, PAGE_ADDR_P, k.args.page_offset

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, TMP, RESP_RX_DMA_CMD_START_FLIT_ID, DMA_CMD_INDEX)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqwqe, cqwqe, CQWQE_P)
    add             DMA_CMD_INDEX, DMA_CMD_INDEX, 1

    CAPRI_SET_TABLE_I_VALID(TBL_ID, 0)
    seq             c2, k.args.arm, 1 
    bcf             [!c2], cqpt_exit
    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE, !c2) //BD slot

    RESP_RX_EQCB_ADDR_GET(EQCB_ADDR, TMP, k.args.eq_id) // BD Slot
    CAPRI_GET_TABLE_I_K_AND_ARG(resp_rx_phv_t, TBL_ID, KEY_P, ARG_P)

    CAPRI_SET_FIELD(ARG_P, EQ_INFO_T, tbl_id, TBL_ID)
    CAPRI_SET_FIELD(ARG_P, EQ_INFO_T, dma_cmd_index, DMA_CMD_INDEX)
    CAPRI_SET_FIELD(ARG_P, EQ_INFO_T, cq_id, k.args.cq_id)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_eqcb_process)
    CAPRI_NEXT_TABLE_I_READ(KEY_P, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, EQCB_ADDR)

cqpt_exit:

    nop.e
    nop

