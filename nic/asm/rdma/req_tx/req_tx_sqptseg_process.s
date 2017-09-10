#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct req_tx_sqptseg_process_k_t k;

%%

req_tx_sqptseg_process:

    // page_id = pt_info_p->pt_offset / pt_info_p->page_size
    add        r1, k.args.log_page_size, r0
    srlv       r1, k.args.pt_offset, r1
    // big-endian
    sub        r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), r1

    // page_offset = pt_info_p->pt_offset % pt_info_p->page_size
    add        r2, k.args.pt_offset, r0
    mincr      r2, k.args.log_page_size, r0

    // dma_cmd_index = pt_info_p->dma_cmd_start_index
    add        r3, k.args.dma_cmd_start_index, r0

    // transfer_bytes = pt_info_p->pt_bytes
    add        r4, k.args.pt_bytes, r0

ptseg_loop:
    // page_bytes = min(transfer_bytes, (pt_info_p->page_size - page_offset))
    add        r5, r0, k.args.log_page_size
    sllv       r5, 1, r5
    sub        r5, r5, r2
    slt        c1, k.args.pt_bytes, r5
    cmov       r5, c1, k.args.pt_bytes, r5

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_I_BASE_GET(r7, r6, REQ_TX_DMA_CMD_START_FLIT_ID, r3)

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

    // set cmdeop in the last mem2pkt dma cmd 
    seq        c1, k.args.dma_cmd_eop, 1
    DMA_SET_END_OF_CMDS_C(DMA_CMD_MEM2PKT_T, r7, c1)
    DMA_SET_END_OF_PKT_C(DMA_CMD_MEM2PKT_T, r7, c1)

    add          r1, k.args.sge_index, r0
    CAPRI_SET_TABLE_I_VALID(r1, 0)

    nop.e
    nop
