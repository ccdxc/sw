#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s5_t0_k k;

#define PAGE_ID r7
#define PAGE_OFFSET r6
#define DMA_CMD_INDEX r5
#define TRANSFER_BYTES r4
#define DMA_BYTES r3
#define DMA_ADDR r2
#define DMA_CMD_BASE r1
#define GLOBAL_FLAGS r6

#define F_FIRST_PASS c7
#define DISABLE_QP c1
#define IS_ATOMIC c4

#define IN_P t0_s2s_lkey_to_pt_info

%%

.align
resp_rx_ptseg_process:

    // MPU GLOBAL
    // take a copy of raw_flags in GLOBAL_FLAGS and keep it for further checks
    add     GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS

    // do not perform any payload xfers if qp was err disabled
    IS_ANY_FLAG_SET(DISABLE_QP, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP)
    bcf     [DISABLE_QP], exit

    IS_ANY_FLAG_SET(IS_ATOMIC, GLOBAL_FLAGS, RESP_RX_FLAG_ATOMIC_CSWAP|RESP_RX_FLAG_ATOMIC_FNA)

    // k_p->pt_offset / log_page_size
    srl         PAGE_ID, CAPRI_KEY_RANGE(IN_P, pt_offset_sbit0_ebit7, pt_offset_sbit24_ebit31), CAPRI_KEY_FIELD(IN_P, log_page_size)
    //big-endian
    sub		PAGE_ID, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), PAGE_ID

    // k_p->pt_offset % log_page_size
    add         PAGE_OFFSET, 0, CAPRI_KEY_RANGE(IN_P, pt_offset_sbit0_ebit7, pt_offset_sbit24_ebit31)
    mincr       PAGE_OFFSET, CAPRI_KEY_FIELD(IN_P, log_page_size), r0
    
    add         DMA_CMD_INDEX, r0, CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
    add         TRANSFER_BYTES, r0, CAPRI_KEY_RANGE(IN_P, pt_bytes_sbit0_ebit7, pt_bytes_sbit24_ebit31)

    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]


transfer_loop:

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_RX_DMA_CMD_START_FLIT_ID, DMA_CMD_INDEX)
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
    bcf                 [IS_ATOMIC], atomic
    
    //STORAGE_USE_CASE
    crestore            [c2, c1], CAPRI_KEY_RANGE(IN_P, host_addr, override_lif_vld), 0x3 //BD Slot
    DMA_PKT2MEM_SETUP_OVERRIDE_LIF(DMA_CMD_BASE, c2, DMA_BYTES, DMA_ADDR, c1, CAPRI_KEY_RANGE(IN_P, override_lif_sbit0_ebit3, override_lif_sbit4_ebit11))
    
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

    seq                 c1, CAPRI_KEY_FIELD(IN_P, dma_cmdeop), 1
    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE, c1)
    
exit:
    
    seq                 c1, CAPRI_KEY_FIELD(IN_P, sge_index), 0
    CAPRI_SET_TABLE_0_VALID_C(c1, 0)
    CAPRI_SET_TABLE_1_VALID_C(!c1, 0)

    nop.e
    nop

atomic:
    phvwr.e             p.pcie_atomic.host_addr, DMA_ADDR.dx
    CAPRI_SET_TABLE_1_VALID(0) //Exit slot
