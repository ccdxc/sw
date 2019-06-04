#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s7_t0_nvme_sesspostxts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s7_t0_nvme_sesspostxts_tx_k_ k;
struct s7_t0_nvme_sesspostxts_tx_pdu_ctxt_process_d d;

#define NEXT_PTR_ADDR         r1
#define AOL_P                 r2
#define TMP_R                 r2
#define NEXT_AOL_P            r3
#define DMA_CMD_BASE          r4
#define PHV_START_LEN         r5
#define PHV_AOL_NEXT_PTR      r6
#define NUM_REMAINING_PAGES   r7

%%

.align
nvme_sesspostxts_tx_pdu_ctxt_process:
    // compose DMA cmds to chain AOLs based on base AOL_P and num_pages
    mfspr          AOL_P, spr_tbladdr
    add            AOL_P, AOL_P, NVME_PDU_CTXT_AOL_DESC_LIST_OFFSET

    DMA_CMD_BASE_GET(DMA_CMD_BASE, aol0_next_ptr_dma)
    add            PHV_AOL_NEXT_PTR, r0, offsetof(struct phv_, aol0_next_ptr_ptr)
    add            PHV_START_LEN, r0, r0
    add            NUM_REMAINING_PAGES, d.num_pages, r0
chain_aol_descs:
    blti           NUM_REMAINING_PAGES, 3, init_last_ddgst_desc
    add            NEXT_AOL_P, AOL_P, 1, LOG_DGST_DESC_SIZE // BD Slot
    add            NEXT_PTR_ADDR, AOL_P, HW_DGST_NEXT_PTR_OFFSET
    phvwrp         PHV_AOL_NEXT_PTR, 0,  CAPRI_SIZEOF_U64_BITS, NEXT_AOL_P.dx
    DMA_HBM_PHV2MEM_START_SLEN_ELEN_SETUP(DMA_CMD_BASE, TMP_R, aol0_next_ptr_ptr, PHV_START_LEN, CAPRI_SIZEOF_U64_BYTES, NEXT_PTR_ADDR)
    sub            PHV_AOL_NEXT_PTR, PHV_AOL_NEXT_PTR, CAPRI_SIZEOF_U64_BITS
    add            PHV_START_LEN, PHV_START_LEN, CAPRI_SIZEOF_U64_BYTES
    sub            NUM_REMAINING_PAGES, NUM_REMAINING_PAGES, 3
    add            AOL_P, NEXT_AOL_P, r0
    b              chain_aol_descs
    add            DMA_CMD_BASE, DMA_CMD_BASE, DMA_CMD_SIZE_BITS // BD Slot

init_last_ddgst_desc:
    seq            c1, NUM_REMAINING_PAGES, 2
    bcf            [c1], init_from_A2
    DMA_CMD_BASE_GET(DMA_CMD_BASE, last_ddgst_aol_desc_dma) // BD Slot

init_from_A1:
    add            AOL_P, AOL_P, 1, LOG_ONE_AOL_SIZE
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, last_ddgst_aol_desc_A1, last_ddgst_aol_desc_rsvd, AOL_P)
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    nop             // Exit Slot

init_from_A2:
    add            AOL_P, AOL_P, 2, LOG_ONE_AOL_SIZE
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, last_ddgst_aol_desc_A2, last_ddgst_aol_desc_rsvd, AOL_P)
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    nop            // Exit Slot

