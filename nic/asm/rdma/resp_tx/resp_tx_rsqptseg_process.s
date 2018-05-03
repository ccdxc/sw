#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s4_t0_k k;

#define TBL_ID r7
#define PAGE_ID r7
#define PAGE_OFFSET r6
#define DMA_CMD_INDEX r5
#define TRANSFER_BYTES r4
#define DMA_BYTES r3
#define DMA_ADDR r2
#define DMA_CMD_BASE r1

#define F_FIRST_PASS c7

#define IN_P t0_s2s_rkey_to_ptseg_info

#define K_PT_SEG_OFFSET CAPRI_KEY_RANGE(IN_P, pt_seg_offset_sbit0_ebit7, pt_seg_offset_sbit24_ebit31)

%%

resp_tx_rsqptseg_process:

    // k_p->pt_offset / log_page_size
    srlv        PAGE_ID, K_PT_SEG_OFFSET, CAPRI_KEY_FIELD(IN_P, log_page_size)
    //big-endian
    sub		PAGE_ID, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), PAGE_ID

    // k_p->pt_offset % log_page_size
    add         PAGE_OFFSET, 0, K_PT_SEG_OFFSET
    mincr       PAGE_OFFSET, CAPRI_KEY_FIELD(IN_P, log_page_size), r0
    
    add         DMA_CMD_INDEX, r0, CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
    add         TRANSFER_BYTES, r0, CAPRI_KEY_FIELD(IN_P, pt_seg_bytes)

    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]


transfer_loop:

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, DMA_CMD_INDEX)
    // r1 has DMA_CMD_BASE

    sll                 DMA_BYTES, 1, CAPRI_KEY_FIELD(IN_P, log_page_size)
    sub.F_FIRST_PASS    DMA_BYTES, DMA_BYTES, PAGE_OFFSET    
    slt                 c3, DMA_BYTES, TRANSFER_BYTES
    cmov                DMA_BYTES, c3, DMA_BYTES, TRANSFER_BYTES
    // r4 has amount of bytes to be xfered

    sll                 r2, PAGE_ID, CAPRI_LOG_SIZEOF_U64_BITS
    //big-endian
    tblrdp.dx           DMA_ADDR, r2, 0, CAPRI_SIZEOF_U64_BITS
    // r2 has page ptr, add page offset for DMA addr
    add                 DMA_ADDR, DMA_ADDR, PAGE_OFFSET
    
    DMA_MEM2PKT_SETUP(DMA_CMD_BASE, c1, DMA_BYTES, DMA_ADDR)
    
    add                 PAGE_OFFSET, r0, r0
    sub                 TRANSFER_BYTES, TRANSFER_BYTES, DMA_BYTES
    add                 DMA_CMD_INDEX, DMA_CMD_INDEX, 1
    //big-endian
    sub                 PAGE_ID, PAGE_ID, 1

    // loop if still some more bytes to be xfered
    seq                 c2, TRANSFER_BYTES, r0
    bcf                 [!c2], transfer_loop
    // first_pass = FALSE
    setcf       F_FIRST_PASS, [!c0]  // BD Slot

    add                 TBL_ID, r0, CAPRI_KEY_FIELD(IN_P, tbl_id)
    CAPRI_SET_TABLE_I_VALID(TBL_ID, 0)

    nop.e
    nop
