#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct req_tx_s5_t0_k k;

#define IN_P t0_s2s_lkey_to_ptseg_info

#define K_PT_OFFSET CAPRI_KEY_RANGE(IN_P, pt_offset_sbit0_ebit7, pt_offset_sbit24_ebit31)
#define K_LOG_PAGE_SIZE CAPRI_KEY_FIELD(IN_P, log_page_size)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_RANGE(IN_P, dma_cmd_start_index_sbit0_ebit2, dma_cmd_start_index_sbit3_ebit7)
#define K_PT_BYTES CAPRI_KEY_RANGE(IN_P, pt_bytes_sbit0_ebit2, pt_bytes_sbit11_ebit15)
#define K_SGE_INDEX CAPRI_KEY_RANGE(IN_P, sge_index_sbit0_ebit2, sge_index_sbit3_ebit7)
#define K_HOST_ADDR CAPRI_KEY_FIELD(IN_P, host_addr)

%%
.align
nop
.align
nop
.align
nop
.align
nop
.align
nop
.align
nop
.align
nop
.align
nop
.align
nop
.align
req_tx_sqptseg_process:

    // page_id = pt_info_p->pt_offset / pt_info_p->page_size
    srl        r1, K_PT_OFFSET, K_LOG_PAGE_SIZE
    // big-endian
    sub        r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), r1

    // page_offset = pt_info_p->pt_offset % pt_info_p->page_size
    add        r2, K_PT_OFFSET, r0
    mincr      r2, K_LOG_PAGE_SIZE, r0

    // dma_cmd_index = pt_info_p->dma_cmd_start_index
    add        r3, K_DMA_CMD_START_INDEX, r0

    // transfer_bytes = pt_info_p->pt_bytes
    add        r4, K_PT_BYTES, r0

ptseg_loop:
    // page_bytes = min(transfer_bytes, (pt_info_p->page_size - page_offset))
    sllv       r5, 1, K_LOG_PAGE_SIZE
    sub        r5, r5, r2
    slt        c1, r4, r5
    cmov       r5, c1, r4, r5

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_I_BASE_GET(r7, r6, REQ_TX_DMA_CMD_START_FLIT_ID, r3)

    crestore      [c2], K_HOST_ADDR, 0x1
    // phy_addr_p = page[page_id] + page_offset
    sll        r6, r1, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx     r6, r6, 0, CAPRI_SIZEOF_U64_BITS
    add           r6, r2, r6

    // setup mem2pkt cmd to transfer data from host memory to pkt payload
    DMA_MEM2PKT_SETUP(r7, c2, r5, r6)

    // transfer_bytes -= page_bytes
    sub        r4, r4, r5

    // dma_cmd_index++
    add        r3, r3, 1

    // page_id++
    // big-endian
    sub        r1, r1, 1
    blt        r0, r4, ptseg_loop

    // page_offset = 0
    add        r2, r0, r0 // Branch Delay Slot

    add          r1, K_SGE_INDEX, r0
    CAPRI_SET_TABLE_I_VALID(r1, 0)

    nop.e
    nop
