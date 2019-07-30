#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s6_t2_k k;

#define IN_P to_s6_rqpt_info
#define TO_S7_P to_s7_stats_info

#define K_PAGE_OFFSET CAPRI_KEY_FIELD(IN_P, page_offset)
#define K_PAGE_SEG_OFFSET CAPRI_KEY_FIELD(IN_P, page_seg_offset)
#define K_PREF_BASE_ADDR CAPRI_KEY_FIELD(IN_P, prefetch_base_addr)
#define K_LOG_WQE_SIZE CAPRI_KEY_FIELD(IN_P, log_wqe_size)
#define K_PREF_PINDEX_PRE CAPRI_KEY_FIELD(IN_P, pref_pindex_pre)
#define K_PREF_PINDEX_POST CAPRI_KEY_FIELD(IN_P, pref_pindex_post)
#define K_CMD_EOP CAPRI_KEY_FIELD(IN_P, cmd_eop)
#define K_TRANSFER_NUM_WQES CAPRI_KEY_FIELD(IN_P, transfer_num_wqes)
#define K_INVOKE_STATS CAPRI_KEY_FIELD(IN_P, invoke_stats)

#define GLOBAL_FLAGS r7
#define DMA_CMD_BASE r5
#define TRANSFER_SIZE r6
#define RQCB4_ADDR   r1
%%

    .param      resp_tx_stats_process

.align
resp_tx_rqpt_process:

    bcf         [c2 | c3 | c7], table_error
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS // BD Slot

    //page_addr_p = (u64 *) (d_p + sizeof(u64) * rqcb_to_pt_info_p->page_seg_offset);

    //big-endian
    sub     r3, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), K_PAGE_SEG_OFFSET //BD Slot
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64_BITS
    //big-endian
    tblrdp.dx  r3, r3, 0, CAPRI_SIZEOF_U64_BITS
    or         r3, r3, 1, 63
    or         r3, r3, K_GLOBAL_LIF, 52

    // wqe_p = (void *)(*page_addr_p + rqcb_to_pt_info_p->page_offset);
    add     r3, r3, K_PAGE_OFFSET
    // now r3 has wqe_p to load

    sll     TRANSFER_SIZE, K_TRANSFER_NUM_WQES, K_LOG_WQE_SIZE
    // DMA commands to read WQE from host memory
    // write to HBM
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_RQ_PREFETCH_WQE_RD)
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, TRANSFER_SIZE, r3)

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_RQ_PREFETCH_WQE_TO_HBM)
    // pref_base_addr = pref_base_addr + (pref_pindex << log_wqe_size)
    sll     r3, K_PREF_PINDEX_PRE, K_LOG_WQE_SIZE
    add     r3, r3, K_PREF_BASE_ADDR, PT_BASE_ADDR_SHIFT
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, TRANSFER_SIZE, r3)

    // Update proxy_pindex in rqcb1_t and set write-fence
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_SET_PROXY_PINDEX)
    RQCB1_ADDR_GET(r2)
    add     r3, r2, FIELD_OFFSET(rqcb1_t, proxy_pindex)
    phvwr   p.rq_prefetch.proxy_pindex, K_PREF_PINDEX_POST
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, rq_prefetch.proxy_pindex, rq_prefetch.proxy_pindex, r3)
    DMA_SET_WR_FENCE(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)

    CAPRI_SET_FIELD2(TO_S7_P, incr_prefetch_cnt, 1)
    CAPRI_SET_TABLE_2_VALID(0)

    bbeq    K_INVOKE_STATS, 0, skip_stats
    RQCB4_ADDR_GET(RQCB4_ADDR) // BD Slot

    CAPRI_SET_FIELD2(TO_S7_P, prefetch_only, 1)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_stats_process, RQCB4_ADDR)

skip_stats:
    seq.e   c1, K_CMD_EOP, 1
    DMA_SET_END_OF_CMDS_C(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE, c1) // Exit Slot

table_error:
    // TODO handle table error
