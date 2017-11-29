#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_cqpt_process_k_t k;

//TODO: verify if it is in right order
//#define PHV_CQWQE_START cqwqe.id.wrid
//#define PHV_CQWQE_END   cqwqe.r_key

#define EQ_INFO_T struct req_rx_cqcb_to_eq_info_t
#define TMP r3

%%
    .param  req_rx_eqcb_process

.align
req_rx_cqpt_process:

    add             r6, r0, k.args.tbl_id
    add             r4, r0, k.args.dma_cmd_index
    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset
    sll             r1, r1, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp.dx       r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, r1, k.args.page_offset

    DMA_CMD_I_BASE_GET(r2, r3, REQ_RX_DMA_CMD_START_FLIT_ID, r4)
    DMA_PHV2MEM_SETUP(r2, c1, cqwqe, cqwqe, r1)
    add             r4, r4, 1

    CAPRI_SET_TABLE_I_VALID(r6, 0)
    seq             c2, k.args.arm, 1 
    bcf             [!c2], cqpt_exit
    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_phv2mem_t, r2, !c2) //BD slot

    EQCB_ADDR_GET(r5, TMP, k.args.eq_id) // BD Slot
    CAPRI_GET_TABLE_I_K_AND_ARG(req_rx_phv_t, r6, r3, r7)

    CAPRI_SET_FIELD(r7, EQ_INFO_T, tbl_id, r6)
    CAPRI_SET_FIELD(r7, EQ_INFO_T, dma_cmd_index, r4)
    CAPRI_SET_FIELD(r7, EQ_INFO_T, cq_id, k.args.cq_id)
    CAPRI_SET_RAW_TABLE_PC(r2, req_rx_eqcb_process)
    CAPRI_NEXT_TABLE_I_READ(r3, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2, r5)

cqpt_exit:

    nop.e
    nop

